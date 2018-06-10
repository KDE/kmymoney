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
#include <QDomDocument>
#include <QDomElement>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include <mymoneyexception.h>
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;
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

MyMoneyInstitution::MyMoneyInstitution(const QDomElement& node) :
  MyMoneyObject(*new MyMoneyInstitutionPrivate, node),
  MyMoneyKeyValueContainer(node.elementsByTagName(nodeNames[nnKeyValuePairs]).item(0).toElement())
{
  if (nodeNames[nnInstitution] != node.tagName())
    throw MYMONEYEXCEPTION_CSTRING("Node was not INSTITUTION");

  Q_D(MyMoneyInstitution);
  d->m_sortcode = node.attribute(d->getAttrName(Institution::Attribute::SortCode));
  d->m_name = node.attribute(d->getAttrName(Institution::Attribute::Name));
  d->m_manager = node.attribute(d->getAttrName(Institution::Attribute::Manager));

  QDomNodeList nodeList = node.elementsByTagName(d->getElName(Institution::Element::Address));
  if (nodeList.isEmpty())
    throw MYMONEYEXCEPTION(QString::fromLatin1("No ADDRESS in institution %1").arg(d->m_name));

  QDomElement addrNode = nodeList.item(0).toElement();
  d->m_street = addrNode.attribute(d->getAttrName(Institution::Attribute::Street));
  d->m_town = addrNode.attribute(d->getAttrName(Institution::Attribute::City));
  d->m_postcode = addrNode.attribute(d->getAttrName(Institution::Attribute::Zip));
  d->m_telephone = addrNode.attribute(d->getAttrName(Institution::Attribute::Telephone));

  d->m_accountList.clear();

  nodeList = node.elementsByTagName(d->getElName(Institution::Element::AccountIDS));
  if (nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName(d->getElName(Institution::Element::AccountID));
    for (int i = 0; i < nodeList.count(); ++i) {
      d->m_accountList << nodeList.item(i).toElement().attribute(d->getAttrName(Institution::Attribute::ID));
    }
  }
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

void MyMoneyInstitution::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_D(const MyMoneyInstitution);
  auto el = document.createElement(nodeNames[nnInstitution]);

  d->writeBaseXML(document, el);

  el.setAttribute(d->getAttrName(Institution::Attribute::Name), d->m_name);
  el.setAttribute(d->getAttrName(Institution::Attribute::Manager), d->m_manager);
  el.setAttribute(d->getAttrName(Institution::Attribute::SortCode), d->m_sortcode);

  auto address = document.createElement(d->getElName(Institution::Element::Address));
  address.setAttribute(d->getAttrName(Institution::Attribute::Street), d->m_street);
  address.setAttribute(d->getAttrName(Institution::Attribute::City), d->m_town);
  address.setAttribute(d->getAttrName(Institution::Attribute::Zip), d->m_postcode);
  address.setAttribute(d->getAttrName(Institution::Attribute::Telephone), d->m_telephone);
  el.appendChild(address);

  auto accounts = document.createElement(d->getElName(Institution::Element::AccountIDS));
  foreach (const auto accountID, accountList()) {
    auto temp = document.createElement(d->getElName(Institution::Element::AccountID));
    temp.setAttribute(d->getAttrName(Institution::Attribute::ID), accountID);
    accounts.appendChild(temp);
  }
  el.appendChild(accounts);

  //Add in Key-Value Pairs for institutions.
  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneyInstitution::hasReferenceTo(const QString& /* id */) const
{
  return false;
}

QPixmap MyMoneyInstitution::pixmap(const int size)
{
  QPixmap pxIcon;
  auto kyIcon = QString::fromLatin1("view_institution%1").arg(QString::number(size));
  if (!QPixmapCache::find(kyIcon, pxIcon)) {
    pxIcon = Icons::get(Icon::ViewInstitutions).pixmap(size);
    QPixmapCache::insert(kyIcon, pxIcon);
  }
  return pxIcon;
}
