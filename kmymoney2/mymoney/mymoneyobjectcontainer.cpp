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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyobjectcontainer.h>
//Added by qt3to4:
#include <Q3ValueList>

MyMoneyObjectContainer::MyMoneyObjectContainer()
{
}

MyMoneyObjectContainer::~MyMoneyObjectContainer()
{
  clear();
}

void MyMoneyObjectContainer::clear(IMyMoneyStorage* storage)
{
  // delete all objects
  QMap<QString, MyMoneyObject const *>::const_iterator it;
  for(it = m_map.begin(); it != m_map.end(); ++it) {
    delete (*it);
  }

  // then delete the pointers to them
  m_map.clear();

  if(storage)
    m_storage = storage;
}

void MyMoneyObjectContainer::clear(const QString& id)
{
  QMap<QString, MyMoneyObject const *>::iterator it;
  it = m_map.find(id);
  if(it != m_map.end()) {
    delete (*it);
    m_map.erase(it);
  }
}

#define listMethod(a, T) \
void MyMoneyObjectContainer::a(Q3ValueList<T>& list) \
{ \
  QMap<QString, const MyMoneyObject*>::const_iterator it; \
  for(it = m_map.begin(); it != m_map.end(); ++it) { \
    const T* node = dynamic_cast<const T*>(*it); \
    if(node) { \
      list.append(*node); \
    } \
  } \
}

#define preloadListMethod(a, T) \
void MyMoneyObjectContainer::preload##a(const Q3ValueList<T>& list) \
{ \
  Q3ValueList<T>::const_iterator it; \
  for(it = list.begin(); it != list.end(); ++it) { \
    delete m_map[(*it).id()]; \
    m_map[(*it).id()] = new T(*it); \
  } \
}

#define preloadMethod(a, T) \
void MyMoneyObjectContainer::preload##a(const T& obj) \
{ \
  delete m_map[obj.id()]; \
  m_map[obj.id()] = new T(obj); \
}

#define objectAccessMethod(a, T) \
const T& MyMoneyObjectContainer::a(const QString& id) \
{ \
  static T nullElement; \
  if(id.isEmpty()) \
    return nullElement; \
  QMap<QString, MyMoneyObject const *>::const_iterator it; \
  it = m_map.find(id); \
  if(it == m_map.end()) { \
    /* not found, need to load from engine */ \
    T x = m_storage->a(id); \
    m_map[id] = new T(x); \
    return dynamic_cast<const T&>(*m_map[id]); \
  } \
  return dynamic_cast<const T&>(*(*it)); \
}

void MyMoneyObjectContainer::account(QLinkedList<MyMoneyAccount>& list)
{
  QMap<QString, const MyMoneyObject*>::const_iterator it;
  for(it = m_map.begin(); it != m_map.end(); ++it) {
    const MyMoneyAccount* node = dynamic_cast<const MyMoneyAccount*>(*it);
    if(node) {
      assignFraction(const_cast<MyMoneyAccount*>(node));
      list.append(*node);
    }
  }
}

const MyMoneyAccount& MyMoneyObjectContainer::account(const QString& id)
{
  static MyMoneyAccount nullElement;
  if(id.isEmpty())
    return nullElement;
  QMap<QString, MyMoneyObject const *>::iterator it;
  it = m_map.find(id);
  if(it == m_map.end()) {
    /* not found, need to load from engine */
    MyMoneyAccount x = m_storage->account(id);
    MyMoneyAccount* item = new MyMoneyAccount(x);
    assignFraction(dynamic_cast<MyMoneyAccount*>(item));
    m_map[id] = item;
    return dynamic_cast<const MyMoneyAccount&>(*m_map[id]);
  }
  assignFraction(dynamic_cast<MyMoneyAccount*> (const_cast<MyMoneyObject*>(*it)));
  return dynamic_cast<const MyMoneyAccount&>(*(*it));
}

void MyMoneyObjectContainer::assignFraction(MyMoneyAccount* acc)
{
  if(acc != 0 && acc->m_fraction == -1) {
    const MyMoneySecurity& sec = security(acc->currencyId());
    acc->fraction(sec);
  }
}

const MyMoneyAccount& MyMoneyObjectContainer::accountByName(const QString& name) const
{
  static MyMoneyAccount nullElement;
  QMap<QString, MyMoneyObject const *>::const_iterator it;
  for(it = m_map.begin(); it != m_map.end(); ++it) {
    const MyMoneyAccount* node = dynamic_cast<const MyMoneyAccount *>(*it);
    if(node) {
      if(node->name() == name)
        return dynamic_cast<const MyMoneyAccount &>(*(*it));
    }
  }
  return nullElement;
}

void MyMoneyObjectContainer::refresh(const QString& id)
{
  if(id.isEmpty())
    return;

  QMap<QString, MyMoneyObject const *>::const_iterator it;
  it = m_map.find(id);
  if(it != m_map.end()) {
    const MyMoneyAccount* account = dynamic_cast<const MyMoneyAccount *>(*it);
    const MyMoneyPayee* payee = dynamic_cast<const MyMoneyPayee *>(*it);
    const MyMoneySecurity* security = dynamic_cast<const MyMoneySecurity *>(*it);
    const MyMoneyInstitution* institution = dynamic_cast<const MyMoneyInstitution *>(*it);
    const MyMoneySchedule* schedule = dynamic_cast<const MyMoneySchedule *>(*it);
    delete *it;
    if(account) {
      const MyMoneyAccount& a = m_storage->account(id);
      m_map[id] = new MyMoneyAccount(a);
    } else if(security) {
      const MyMoneySecurity& s = m_storage->security(id);
      if(s.id().isEmpty()) {
        const MyMoneySecurity& c = m_storage->currency(id);
        m_map[id] = new MyMoneySecurity(c);
      } else {
        m_map[id] = new MyMoneySecurity(s);
      }
    } else if(payee) {
      const MyMoneyPayee& p = m_storage->payee(id);
      m_map[id] = new MyMoneyPayee(p);
    } else if(institution) {
      const MyMoneyInstitution& i = m_storage->institution(id);
      m_map[id] = new MyMoneyInstitution(i);
    } else if(schedule) {
      const MyMoneySchedule& s = m_storage->schedule(id);
      m_map[id] = new MyMoneySchedule(s);
    } else {
      qWarning("Ooops, should preload an unknown object with id '%s'", id.data());
    }
    return;
  }
}

objectAccessMethod(schedule, MyMoneySchedule)
objectAccessMethod(payee, MyMoneyPayee)
objectAccessMethod(security, MyMoneySecurity)
objectAccessMethod(institution, MyMoneyInstitution)

preloadListMethod(Account, MyMoneyAccount)
preloadListMethod(Payee, MyMoneyPayee)
preloadListMethod(Institution, MyMoneyInstitution)
preloadListMethod(Security, MyMoneySecurity)
preloadListMethod(Schedule, MyMoneySchedule)

preloadMethod(Account, MyMoneyAccount)
preloadMethod(Security, MyMoneySecurity)
preloadMethod(Payee, MyMoneyPayee)
preloadMethod(Institution, MyMoneyInstitution)

listMethod(payee, MyMoneyPayee)
listMethod(institution, MyMoneyInstitution)

#include "mymoneyobjectcontainer.moc"
