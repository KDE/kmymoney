/*
 * Copyright 2000-2001  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2003       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mymoneyinstitution.h"
#include "mymoneyinstitution_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QPixmapCache>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include <mymoneyexception.h>

using namespace Icons;

MyMoneyInstitution::MyMoneyInstitution() :
  MyMoneyObject(*new MyMoneyInstitutionPrivate),
  MyMoneyKeyValueContainer()
{
}

MyMoneyInstitution::MyMoneyInstitution(const QString &id) :
  MyMoneyObject(*new MyMoneyInstitutionPrivate, id),
  MyMoneyKeyValueContainer()
{
}

MyMoneyInstitution::MyMoneyInstitution(const QString& name,
                                       const QString& town,
                                       const QString& street,
                                       const QString& postcode,
                                       const QString& telephone,
                                       const QString& manager,
                                       const QString& sortcode) :
  MyMoneyObject(*new MyMoneyInstitutionPrivate),
  MyMoneyKeyValueContainer()
{
  Q_D(MyMoneyInstitution);
  clearId();
  d->m_name = name;
  d->m_town = town;
  d->m_street = street;
  d->m_postcode = postcode;
  d->m_telephone = telephone;
  d->m_manager = manager;
  d->m_sortcode = sortcode;
}

MyMoneyInstitution::MyMoneyInstitution(const MyMoneyInstitution& other) :
  MyMoneyObject(*new MyMoneyInstitutionPrivate(*other.d_func()), other.id()),
  MyMoneyKeyValueContainer(other)
{
}

MyMoneyInstitution::MyMoneyInstitution(const QString& id, const MyMoneyInstitution& other) :
    MyMoneyObject(*new MyMoneyInstitutionPrivate(*other.d_func()), id),
    MyMoneyKeyValueContainer(other)
{
}

MyMoneyInstitution::~MyMoneyInstitution()
{
}

QString MyMoneyInstitution::manager() const
{
  Q_D(const MyMoneyInstitution);
  return d->m_manager;
}

void MyMoneyInstitution::setManager(const QString& manager)
{
  Q_D(MyMoneyInstitution);
  d->m_manager = manager;
}

QString MyMoneyInstitution::name() const
{
  Q_D(const MyMoneyInstitution);
  return d->m_name;
}

void MyMoneyInstitution::setName(const QString& name)
{
  Q_D(MyMoneyInstitution);
  d->m_name = name;
}

QString MyMoneyInstitution::postcode() const
{
  Q_D(const MyMoneyInstitution);
  return d->m_postcode;
}

void MyMoneyInstitution::setPostcode(const QString& code)
{
  Q_D(MyMoneyInstitution);
  d->m_postcode = code;
}

QString MyMoneyInstitution::street() const
{
  Q_D(const MyMoneyInstitution);
  return d->m_street;
}

void MyMoneyInstitution::setStreet(const QString& street)
{
  Q_D(MyMoneyInstitution);
  d->m_street = street;
}

QString MyMoneyInstitution::telephone() const
{
  Q_D(const MyMoneyInstitution);
  return d->m_telephone;
}

void MyMoneyInstitution::setTelephone(const QString& tel)
{
  Q_D(MyMoneyInstitution);
  d->m_telephone = tel;
}

QString MyMoneyInstitution::town() const
{
  Q_D(const MyMoneyInstitution);
  return d->m_town;
}

void MyMoneyInstitution::setTown(const QString& town)
{
  Q_D(MyMoneyInstitution);
  d->m_town = town;
}

QString MyMoneyInstitution::city() const
{
  return town();
}

void MyMoneyInstitution::setCity(const QString& town)
{
  setTown(town);
}

QString MyMoneyInstitution::sortcode() const
{
  Q_D(const MyMoneyInstitution);
  return d->m_sortcode;
}

void MyMoneyInstitution::setSortcode(const QString& code)
{
  Q_D(MyMoneyInstitution);
  d->m_sortcode = code;
}

void MyMoneyInstitution::addAccountId(const QString& account)
{
  Q_D(MyMoneyInstitution);
  // only add this account if it is not yet presently in the list
  if (d->m_accountList.contains(account) == 0)
    d->m_accountList.append(account);
}

QString MyMoneyInstitution::removeAccountId(const QString& account)
{
  Q_D(MyMoneyInstitution);
  QString rc;

  auto pos = d->m_accountList.indexOf(account);
  if (pos != -1) {
    d->m_accountList.removeAt(pos);
    rc = account;
  }
  return rc;
}

QStringList MyMoneyInstitution::accountList() const
{
  Q_D(const MyMoneyInstitution);
  return d->m_accountList;
}

/**
  * This method returns the number of accounts known to
  * this institution
  * @return number of accounts
  */
unsigned int MyMoneyInstitution::accountCount() const
{
  Q_D(const MyMoneyInstitution);
  return d->m_accountList.count();
}

bool MyMoneyInstitution::operator < (const MyMoneyInstitution& right) const
{
  Q_D(const MyMoneyInstitution);
  auto d2 = static_cast<const MyMoneyInstitutionPrivate *>(right.d_func());
  return d->m_name < d2->m_name;
}

bool MyMoneyInstitution::operator == (const MyMoneyInstitution& right) const
{
  Q_D(const MyMoneyInstitution);
  auto d2 = static_cast<const MyMoneyInstitutionPrivate *>(right.d_func());
  if (MyMoneyObject::operator==(right) &&
      ((d->m_name.length() == 0 && d2->m_name.length() == 0) || (d->m_name == d2->m_name)) &&
      ((d->m_town.length() == 0 && d2->m_town.length() == 0) || (d->m_town == d2->m_town)) &&
      ((d->m_street.length() == 0 && d2->m_street.length() == 0) || (d->m_street == d2->m_street)) &&
      ((d->m_postcode.length() == 0 && d2->m_postcode.length() == 0) || (d->m_postcode == d2->m_postcode)) &&
      ((d->m_telephone.length() == 0 && d2->m_telephone.length() == 0) || (d->m_telephone == d2->m_telephone)) &&
      ((d->m_sortcode.length() == 0 && d2->m_sortcode.length() == 0) || (d->m_sortcode == d2->m_sortcode)) &&
      ((d->m_manager.length() == 0 && d2->m_manager.length() == 0) || (d->m_manager == d2->m_manager)) &&
      (d->m_accountList == d2->m_accountList)) {
    return true;
  } else
    return false;
}

bool MyMoneyInstitution::hasReferenceTo(const QString& /* id */) const
{
  return false;
}

QPixmap MyMoneyInstitution::pixmap(const int size)
{
  QPixmap pxIcon;
  auto kyIcon = QString::fromLatin1("view_institution%1").arg(QString::number(size));
  if (!QPixmapCache::find(kyIcon, &pxIcon)) {
    pxIcon = Icons::get(Icon::ViewInstitutions).pixmap(size);
    QPixmapCache::insert(kyIcon, pxIcon);
  }
  return pxIcon;
}
