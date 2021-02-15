/*
    SPDX-FileCopyrightText: 2000-2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneypayee.h"
#include "mymoneypayee_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QStringList>
#include <QSet>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneyenums.h"

/// @todo remove and replace that one occurrence with MyMoneyPayee()
MyMoneyPayee MyMoneyPayee::null;

MyMoneyPayee::MyMoneyPayee() :
  MyMoneyObject(*new MyMoneyPayeePrivate)
{
}

MyMoneyPayee::MyMoneyPayee(const QString &id):
  MyMoneyObject(*new MyMoneyPayeePrivate, id)
{
}

MyMoneyPayee::MyMoneyPayee(const MyMoneyPayee& other) :
  MyMoneyObject(*new MyMoneyPayeePrivate(*other.d_func()), other.id()),
  MyMoneyPayeeIdentifierContainer(other)
{
}

MyMoneyPayee::MyMoneyPayee(const QString& id, const MyMoneyPayee& other) :
  MyMoneyObject(*new MyMoneyPayeePrivate(*other.d_func()), id),
  MyMoneyPayeeIdentifierContainer(other)
{
}

MyMoneyPayee::~MyMoneyPayee()
{
}

bool MyMoneyPayee::operator == (const MyMoneyPayee& right) const
{
  Q_D(const MyMoneyPayee);
  auto d2 = static_cast<const MyMoneyPayeePrivate *>(right.d_func());
  return (MyMoneyObject::operator==(right) &&
          ((d->m_name.length() == 0 && d2->m_name.length() == 0) || (d->m_name == d2->m_name)) &&
          ((d->m_address.length() == 0 && d2->m_address.length() == 0) || (d->m_address == d2->m_address)) &&
          ((d->m_city.length() == 0 && d2->m_city.length() == 0) || (d->m_city == d2->m_city)) &&
          ((d->m_state.length() == 0 && d2->m_state.length() == 0) || (d->m_state == d2->m_state)) &&
          ((d->m_postcode.length() == 0 && d2->m_postcode.length() == 0) || (d->m_postcode == d2->m_postcode)) &&
          ((d->m_telephone.length() == 0 && d2->m_telephone.length() == 0) || (d->m_telephone == d2->m_telephone)) &&
          ((d->m_email.length() == 0 && d2->m_email.length() == 0) || (d->m_email == d2->m_email)) &&
          (d->m_matchingEnabled == d2->m_matchingEnabled) &&
          (d->m_usingMatchKey == d2->m_usingMatchKey) &&
          (d->m_matchKeyIgnoreCase == d2->m_matchKeyIgnoreCase) &&
          ((d->m_matchKey.length() == 0 && d2->m_matchKey.length() == 0) || d->m_matchKey == d2->m_matchKey) &&
          ((d->m_reference.length() == 0 && d2->m_reference.length() == 0) || (d->m_reference == d2->m_reference)) &&
          ((d->m_defaultAccountId.length() == 0 && d2->m_defaultAccountId.length() == 0) || d->m_defaultAccountId == d2->m_defaultAccountId));
}

bool MyMoneyPayee::operator < (const MyMoneyPayee& right) const
{
  Q_D(const MyMoneyPayee);
  auto d2 = static_cast<const MyMoneyPayeePrivate *>(right.d_func());
  return d->m_name < d2->m_name;
}

bool MyMoneyPayee::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneyPayee);
  return id == d->m_defaultAccountId;
}

QSet<QString> MyMoneyPayee::referencedObjects() const
{
  Q_D(const MyMoneyPayee);
  return { d->m_defaultAccountId };
}

QString MyMoneyPayee::name() const
{
  Q_D(const MyMoneyPayee);
  return d->m_name;
}

void MyMoneyPayee::setName(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_name = val;
}

QString MyMoneyPayee::address() const
{
  Q_D(const MyMoneyPayee);
  return d->m_address;
}

void MyMoneyPayee::setAddress(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_address = val;
}

QString MyMoneyPayee::city() const
{
  Q_D(const MyMoneyPayee);
  return d->m_city;
}

void MyMoneyPayee::setCity(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_city = val;
}

QString MyMoneyPayee::state() const
{
  Q_D(const MyMoneyPayee);
  return d->m_state;
}

void MyMoneyPayee::setState(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_state = val;
}

QString MyMoneyPayee::postcode() const
{
  Q_D(const MyMoneyPayee);
  return d->m_postcode;
}

void MyMoneyPayee::setPostcode(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_postcode = val;
}

QString MyMoneyPayee::telephone() const
{
  Q_D(const MyMoneyPayee);
  return d->m_telephone;
}

void MyMoneyPayee::setTelephone(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_telephone = val;
}

QString MyMoneyPayee::email() const
{
  Q_D(const MyMoneyPayee);
  return d->m_email;
}

void MyMoneyPayee::setEmail(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_email = val;
}

QString MyMoneyPayee::notes() const
{
  Q_D(const MyMoneyPayee);
  return d->m_notes;
}

void MyMoneyPayee::setNotes(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_notes = val;
}

QString MyMoneyPayee::reference() const
{
  Q_D(const MyMoneyPayee);
  return d->m_reference;
}

void MyMoneyPayee::setReference(const QString& ref)
{
  Q_D(MyMoneyPayee);
  d->m_reference = ref;
}

bool MyMoneyPayee::isMatchingEnabled() const
{
  Q_D(const MyMoneyPayee);
  return d->m_matchingEnabled;
}

bool MyMoneyPayee::isUsingMatchKey() const
{
  Q_D(const MyMoneyPayee);
  return d->m_usingMatchKey;
}

bool MyMoneyPayee::isMatchKeyIgnoreCase() const
{
  Q_D(const MyMoneyPayee);
  return d->m_matchKeyIgnoreCase;
}

QString MyMoneyPayee::matchKey() const
{
  Q_D(const MyMoneyPayee);
  return d->m_matchKey;
}

eMyMoney::Payee::MatchType MyMoneyPayee::matchData(bool& ignorecase, QStringList& keys) const
{
  auto type = eMyMoney::Payee::MatchType::Disabled;
  keys.clear();

  Q_D(const MyMoneyPayee);
  ignorecase = d->m_matchKeyIgnoreCase;

  if (d->m_matchingEnabled) {
    type = d->m_usingMatchKey ? eMyMoney::Payee::MatchType::Key : eMyMoney::Payee::MatchType::Name;
    if (type == eMyMoney::Payee::MatchType::Key) {
      if (d->m_matchKey.contains(QLatin1Char('\n')))
        keys = d->m_matchKey.split(QLatin1Char('\n'));
      else
        keys = d->m_matchKey.split(QLatin1Char(';'));  // for compatibility with 4.8.0
    } else if (d->m_matchKey.compare(QLatin1String("^$")) == 0) {
      type = eMyMoney::Payee::MatchType::NameExact;
    }
  }

  return type;
}

eMyMoney::Payee::MatchType MyMoneyPayee::matchData(bool& ignorecase, QString& keyString) const
{
  QStringList keys;
  auto type = matchData(ignorecase, keys);
  keyString = keys.join(QLatin1Char('\n'));
  return type;
}

void MyMoneyPayee::setMatchData(eMyMoney::Payee::MatchType type, bool ignorecase, const QStringList& keys)
{
  Q_D(MyMoneyPayee);
  d->m_matchingEnabled = (type != eMyMoney::Payee::MatchType::Disabled);
  d->m_matchKeyIgnoreCase = ignorecase;
  d->m_matchKey.clear();

  if (d->m_matchingEnabled) {
    d->m_usingMatchKey = (type == eMyMoney::Payee::MatchType::Key);
    if (d->m_usingMatchKey) {
      QRegExp validKeyRegExp("[^ ]");
      QStringList filteredKeys = keys.filter(validKeyRegExp);
      d->m_matchKey = filteredKeys.join(QLatin1Char('\n'));
    } else if(type == eMyMoney::Payee::MatchType::NameExact) {
      d->m_matchKey = QLatin1String("^$");
    }
  }
}

void MyMoneyPayee::setMatchData(eMyMoney::Payee::MatchType type, bool ignorecase, const QString& keys)
{
  if (keys.contains(QLatin1Char('\n')))
    setMatchData(type, ignorecase, keys.split(QLatin1Char('\n')));
  else
    setMatchData(type, ignorecase, keys.split(QLatin1Char(';'))); // for compatibility with 4.8.0
}

QString MyMoneyPayee::defaultAccountId() const
{
  Q_D(const MyMoneyPayee);
  return d->m_defaultAccountId;
}

void MyMoneyPayee::setDefaultAccountId(const QString& id)
{
  Q_D(MyMoneyPayee);
  d->m_defaultAccountId = id;
}

// vim:cin:si:ai:et:ts=2:sw=2:
