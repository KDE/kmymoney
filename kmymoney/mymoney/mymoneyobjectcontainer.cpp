/***************************************************************************
                          mymoneyobjectcontainer.cpp
                          -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyobjectcontainer.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/onlinejob.h"

#include "imymoneystorage.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyschedule.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneycostcenter.h"

struct MyMoneyObjectContainer::Private {
  Private(MyMoneyObjectContainer *p) :
      storage(0),
      pub(p) {
  }

  void assignFraction(MyMoneyAccount* acc) {
    if (acc != 0 && acc->fraction() == -1) {
      const MyMoneySecurity& sec = pub->security(acc->currencyId());
      acc->fraction(sec);
    }
  }

  template <typename CacheType>
  const MyMoneyObject * getObject(const CacheType& map, const QString& id) {
    typename CacheType::const_iterator it = map.find(id);
    if (it != map.end()) {
      return (*it);
    }
    return 0;
  }

  template <typename ObjType, typename ObjFactory>
  bool refreshObject(const QString& id, QHash<QString, ObjType const *> &hash, ObjFactory f) {
    typename QHash<QString, ObjType const *>::iterator it = hash.find(id);
    if (it != hash.end()) {
      delete *it;
      const ObjType& t = (storage->*f)(id);
      *it = new ObjType(t);
      return true;
    }
    return false;
  }

  template <typename CacheType>
  bool clearObject(CacheType& map, const QString& id) {
    typename CacheType::iterator it = map.find(id);
    if (it != map.end()) {
      delete(*it);
      map.erase(it);
      return true;
    }
    return false;
  }

  template <typename CacheType>
  void clearCache(CacheType& map) {
    // delete all objects
    qDeleteAll(map);
    // then delete the pointers to them
    map.clear();
  }

  template <typename ObjType>
  void listMethodImpl(QList<ObjType>& list, const QHash<QString, ObjType const *> &hash) {
    for (typename QHash<QString, ObjType const *>::const_iterator it = hash.begin(); it != hash.end(); ++it) {
      list.append(*it.value());
    }
  }

  template <typename ObjType>
  void preloadMethodImpl(const ObjType& obj, QHash<QString, ObjType const *> &hash) {
    const QString &objId = obj.id();
    typename QHash<QString, ObjType const *>::iterator it = hash.find(objId);
    if (it != hash.end()) {
      delete hash.value(objId);
    }
    hash[objId] = new ObjType(obj);
  }

  template <typename ObjType>
  void preloadListMethodImpl(const QList<ObjType>& list, QHash<QString, ObjType const *> &hash) {
    for (typename QList<ObjType>::const_iterator it = list.begin(); it != list.end(); ++it)
      preloadMethodImpl(*it, hash);
  }

  template <typename ObjType, typename ObjFactory>
  const ObjType& objectAccessMethodImpl(const QString& id, QHash<QString, ObjType const *> &hash, ObjFactory f) {
    static ObjType nullElement;
    if (id.isEmpty())
      return nullElement;
    typename QHash<QString, ObjType const *>::const_iterator it = hash.constFind(id);
    if (it == hash.constEnd()) {
      /* not found, need to load from engine */
      const ObjType &x = (storage->*f)(id);
      hash[id] = new ObjType(x);
      return *hash.value(id);
    }
    return *(*it);
  }

  QHash<QString, MyMoneyAccount const *> accountCache;
  QHash<QString, MyMoneyPayee const *> payeeCache;
  QHash<QString, MyMoneyTag const *> tagCache;
  QHash<QString, MyMoneySecurity const *> securityCache;
  QHash<QString, MyMoneyInstitution const *> institutionCache;
  QHash<QString, MyMoneySchedule const *> scheduleCache;
  QHash<QString, onlineJob const *> onlineJobCache;
  QHash<QString, MyMoneyCostCenter const*> costCenterCache;

  IMyMoneyStorage* storage;
  MyMoneyObjectContainer *pub;
};

MyMoneyObjectContainer::MyMoneyObjectContainer() : d(new Private(this))
{
}

MyMoneyObjectContainer::~MyMoneyObjectContainer()
{
  clear();
  delete d;
}

void MyMoneyObjectContainer::clear(IMyMoneyStorage* storage)
{
  d->clearCache(d->accountCache);
  d->clearCache(d->payeeCache);
  d->clearCache(d->tagCache);
  d->clearCache(d->securityCache);
  d->clearCache(d->institutionCache);
  d->clearCache(d->scheduleCache);
  d->clearCache(d->onlineJobCache);
  d->clearCache(d->costCenterCache);

  if (storage)
    d->storage = storage;
}

void MyMoneyObjectContainer::clear(const QString& id)
{
  if (d->clearObject(d->accountCache, id))
    return;
  if (d->clearObject(d->payeeCache, id))
    return;
  if (d->clearObject(d->tagCache, id))
    return;
  if (d->clearObject(d->securityCache, id))
    return;
  if (d->clearObject(d->institutionCache, id))
    return;
  if (d->clearObject(d->scheduleCache, id))
    return;
  if (d->clearObject(d->onlineJobCache, id))
    return;
  if (d->clearObject(d->costCenterCache, id))
    return;
  qWarning("Ooops, should clear an unknown object with id '%s'", qPrintable(id));
}

const MyMoneyObject * MyMoneyObjectContainer::object(const QString& id) const
{
  const MyMoneyObject * obj = 0;
  if ((obj = d->getObject(d->accountCache, id)))
    return obj;
  if ((obj = d->getObject(d->payeeCache, id)))
    return obj;
  if ((obj = d->getObject(d->tagCache, id)))
    return obj;
  if ((obj = d->getObject(d->securityCache, id)))
    return obj;
  if ((obj = d->getObject(d->institutionCache, id)))
    return obj;
  if ((obj = d->getObject(d->scheduleCache, id)))
    return obj;
  if ((obj = d->getObject(d->onlineJobCache, id)))
    return obj;
  if ((obj = d->getObject(d->costCenterCache, id)))
    return obj;
  qWarning("Ooops, should get an unknown object with id '%s'", qPrintable(id));
  return 0;
}

void MyMoneyObjectContainer::account(QList<MyMoneyAccount>& list)
{
  QHash<QString, const MyMoneyAccount*>::const_iterator it;
  for (it = d->accountCache.constBegin(); it != d->accountCache.constEnd(); ++it) {
    const MyMoneyAccount* node = *it;
    if (node) {
      d->assignFraction(const_cast<MyMoneyAccount*>(node));
      list.append(*node);
    }
  }
}

const MyMoneyAccount& MyMoneyObjectContainer::account(const QString& id)
{
  static MyMoneyAccount nullElement;
  if (id.isEmpty())
    return nullElement;
  QHash<QString, MyMoneyAccount const *>::iterator it = d->accountCache.find(id);
  if (it == d->accountCache.end()) {
    /* not found, need to load from engine */
    const MyMoneyAccount &x = d->storage->account(id);
    MyMoneyAccount* item = new MyMoneyAccount(x);
    d->assignFraction(item);
    d->accountCache[id] = item;
    return *item;
  } else {
    d->assignFraction(const_cast<MyMoneyAccount*>(*it));
    return **it;
  }
  return nullElement;
}

const MyMoneyAccount& MyMoneyObjectContainer::accountByName(const QString& name) const
{
  static MyMoneyAccount nullElement;
  QHash<QString, MyMoneyAccount const *>::const_iterator it;
  for (it = d->accountCache.constBegin(); it != d->accountCache.constEnd(); ++it) {
    const MyMoneyAccount* node = *it;
    if (node && node->name() == name) {
      return **it;
    }
  }
  return nullElement;
}

void MyMoneyObjectContainer::refresh(const QString& id)
{
  if (id.isEmpty())
    return;

  if (d->refreshObject(id, d->accountCache, &IMyMoneyStorage::account))
    return;
  if (d->refreshObject(id, d->payeeCache, &IMyMoneyStorage::payee))
    return;
  if (d->refreshObject(id, d->tagCache, &IMyMoneyStorage::tag))
    return;
  if (d->refreshObject(id, d->institutionCache, &IMyMoneyStorage::institution))
    return;
  if (d->refreshObject(id, d->scheduleCache, &IMyMoneyStorage::schedule))
    return;
  if (d->refreshObject(id, d->onlineJobCache, &IMyMoneyStorage::getOnlineJob))
    return;
  if (d->refreshObject(id, d->costCenterCache, &IMyMoneyStorage::costCenter))
    return;

  // special handling of securities
  QHash<QString, MyMoneySecurity const *>::iterator it = d->securityCache.find(id);
  if (it != d->securityCache.end()) {
    delete *it;
    const MyMoneySecurity& s = d->storage->security(id);
    if (s.id().isEmpty()) {
      const MyMoneySecurity& c = d->storage->currency(id);
      *it = new MyMoneySecurity(c);
    } else {
      *it = new MyMoneySecurity(s);
    }
    return;
  }

  qWarning("Ooops, should refresh an unknown object with id '%s'", qPrintable(id));
}

const MyMoneyPayee& MyMoneyObjectContainer::payee(const QString& id)
{
  return d->objectAccessMethodImpl(id, d->payeeCache, &IMyMoneyStorage::payee);
}

const MyMoneyTag& MyMoneyObjectContainer::tag(const QString& id)
{
  return d->objectAccessMethodImpl(id, d->tagCache, &IMyMoneyStorage::tag);
}

const MyMoneySecurity& MyMoneyObjectContainer::security(const QString& id)
{
  return d->objectAccessMethodImpl(id, d->securityCache, &IMyMoneyStorage::security);
}

const MyMoneyInstitution& MyMoneyObjectContainer::institution(const QString& id)
{
  return d->objectAccessMethodImpl(id, d->institutionCache, &IMyMoneyStorage::institution);
}

const MyMoneySchedule& MyMoneyObjectContainer::schedule(const QString& id)
{
  return d->objectAccessMethodImpl(id, d->scheduleCache, &IMyMoneyStorage::schedule);
}

void MyMoneyObjectContainer::preloadAccount(const QList<MyMoneyAccount>& list)
{
  d->preloadListMethodImpl(list, d->accountCache);
}

void MyMoneyObjectContainer::preloadPayee(const QList<MyMoneyPayee>& list)
{
  d->preloadListMethodImpl(list, d->payeeCache);
}

void MyMoneyObjectContainer::preloadTag(const QList<MyMoneyTag>& list)
{
  d->preloadListMethodImpl(list, d->tagCache);
}

void MyMoneyObjectContainer::preloadInstitution(const QList<MyMoneyInstitution>& list)
{
  d->preloadListMethodImpl(list, d->institutionCache);
}

void MyMoneyObjectContainer::preloadSecurity(const QList<MyMoneySecurity>& list)
{
  d->preloadListMethodImpl(list, d->securityCache);
}

void MyMoneyObjectContainer::preloadSchedule(const QList<MyMoneySchedule>& list)
{
  d->preloadListMethodImpl(list, d->scheduleCache);
}

void MyMoneyObjectContainer::preloadOnlineJob(const QList< onlineJob >& list)
{
  d->preloadListMethodImpl(list, d->onlineJobCache);
}

void MyMoneyObjectContainer::preloadAccount(const MyMoneyAccount& account)
{
  d->preloadMethodImpl(account, d->accountCache);
}

void MyMoneyObjectContainer::preloadSecurity(const MyMoneySecurity& security)
{
  d->preloadMethodImpl(security, d->securityCache);
}

void MyMoneyObjectContainer::preloadPayee(const MyMoneyPayee& payee)
{
  d->preloadMethodImpl(payee, d->payeeCache);
}

void MyMoneyObjectContainer::preloadTag(const MyMoneyTag& tag)
{
  d->preloadMethodImpl(tag, d->tagCache);
}

void MyMoneyObjectContainer::preloadInstitution(const MyMoneyInstitution& institution)
{
  d->preloadMethodImpl(institution, d->institutionCache);
}

void MyMoneyObjectContainer::preloadSchedule(const MyMoneySchedule& schedule)
{
  d->preloadMethodImpl(schedule, d->scheduleCache);
}

void MyMoneyObjectContainer::preloadOnlineJob(const onlineJob& job)
{
  d->preloadMethodImpl(job, d->onlineJobCache);
}

void MyMoneyObjectContainer::payee(QList<MyMoneyPayee>& list)
{
  d->listMethodImpl(list, d->payeeCache);
}

void MyMoneyObjectContainer::tag(QList<MyMoneyTag>& list)
{
  d->listMethodImpl(list, d->tagCache);
}

void MyMoneyObjectContainer::institution(QList<MyMoneyInstitution>& list)
{
  d->listMethodImpl(list, d->institutionCache);
}
