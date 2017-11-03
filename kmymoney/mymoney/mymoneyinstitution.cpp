/***************************************************************************
                          mymoneyinstitution.cpp
                          -------------------
    copyright            : (C) 2001 by Michael Edwardes <mte@users.sourceforge.net>
                               2002-2005 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyinstitution.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QPixmap>
#include <QPixmapCache>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include <mymoneyexception.h>
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;
using namespace Icons;

class MyMoneyInstitutionPrivate {

public:
  /**
    * This member variable keeps the name of the institution
    */
  QString m_name;

  /**
    * This member variable keeps the city of the institution
    */
  QString m_town;

  /**
    * This member variable keeps the street of the institution
    */
  QString m_street;

  /**
    * This member variable keeps the zip-code of the institution
    */
  QString m_postcode;

  /**
    * This member variable keeps the telephone number of the institution
    */
  QString m_telephone;

  /**
    * This member variable keeps the name of the representative of
    * the institution
    */
  QString m_manager;

  /**
    * This member variable keeps the sort code of the institution.
    * FIXME: I have no idea
    * what it is good for. I keep it because it was in the old engine.
    */
  QString m_sortcode;

  /**
    * This member variable keeps the sorted list of the account ids
    * available at this institution
    */
  QStringList m_accountList;
};

MyMoneyInstitution::MyMoneyInstitution() :
  MyMoneyObject(),
  MyMoneyKeyValueContainer(),
  d_ptr(new MyMoneyInstitutionPrivate)
{
}

MyMoneyInstitution::MyMoneyInstitution(const QString& name,
                                       const QString& town,
                                       const QString& street,
                                       const QString& postcode,
                                       const QString& telephone,
                                       const QString& manager,
                                       const QString& sortcode) :
  MyMoneyKeyValueContainer(),
  d_ptr(new MyMoneyInstitutionPrivate)
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
  MyMoneyObject(node),
  MyMoneyKeyValueContainer(node.elementsByTagName(nodeNames[nnKeyValuePairs]).item(0).toElement()),
  d_ptr(new MyMoneyInstitutionPrivate)
{
  if (nodeNames[nnInstitution] != node.tagName())
    throw MYMONEYEXCEPTION("Node was not INSTITUTION");

  Q_D(MyMoneyInstitution);
  d->m_sortcode = node.attribute(getAttrName(Attribute::SortCode));
  d->m_name = node.attribute(getAttrName(Attribute::Name));
  d->m_manager = node.attribute(getAttrName(Attribute::Manager));

  QDomNodeList nodeList = node.elementsByTagName(getElName(Element::Address));
  if (nodeList.count() == 0) {
    QString msg = QString("No ADDRESS in institution %1").arg(d->m_name);
    throw MYMONEYEXCEPTION(msg);
  }

  QDomElement addrNode = nodeList.item(0).toElement();
  d->m_street = addrNode.attribute(getAttrName(Attribute::Street));
  d->m_town = addrNode.attribute(getAttrName(Attribute::City));
  d->m_postcode = addrNode.attribute(getAttrName(Attribute::Zip));
  d->m_telephone = addrNode.attribute(getAttrName(Attribute::Telephone));

  d->m_accountList.clear();

  nodeList = node.elementsByTagName(getElName(Element::AccountIDS));
  if (nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName(getElName(Element::AccountID));
    for (int i = 0; i < nodeList.count(); ++i) {
      d->m_accountList << nodeList.item(i).toElement().attribute(getAttrName(Attribute::ID));
    }
  }
}

MyMoneyInstitution::MyMoneyInstitution(const MyMoneyInstitution& other) :
  MyMoneyObject(other.id()),
  MyMoneyKeyValueContainer(other),
  d_ptr(new MyMoneyInstitutionPrivate(*other.d_func()))
{
}

MyMoneyInstitution::MyMoneyInstitution(const QString& id, const MyMoneyInstitution& other) :
    MyMoneyObject(id),
    MyMoneyKeyValueContainer(other),
    d_ptr(new MyMoneyInstitutionPrivate(*other.d_func()))
{
}

MyMoneyInstitution::~MyMoneyInstitution()
{
  Q_D(MyMoneyInstitution);
  delete d;
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
  auto el = document.createElement(nodeNames[nnInstitution]);

  writeBaseXML(document, el);

  Q_D(const MyMoneyInstitution);
  el.setAttribute(getAttrName(Attribute::Name), d->m_name);
  el.setAttribute(getAttrName(Attribute::Manager), d->m_manager);
  el.setAttribute(getAttrName(Attribute::SortCode), d->m_sortcode);

  auto address = document.createElement(getElName(Element::Address));
  address.setAttribute(getAttrName(Attribute::Street), d->m_street);
  address.setAttribute(getAttrName(Attribute::City), d->m_town);
  address.setAttribute(getAttrName(Attribute::Zip), d->m_postcode);
  address.setAttribute(getAttrName(Attribute::Telephone), d->m_telephone);
  el.appendChild(address);

  auto accounts = document.createElement(getElName(Element::AccountIDS));
  foreach (const auto accountID, accountList()) {
    auto temp = document.createElement(getElName(Element::AccountID));
    temp.setAttribute(getAttrName(Attribute::ID), accountID);
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

QPixmap MyMoneyInstitution::pixmap(const int size) const
{
  QPixmap pxIcon;
  auto kyIcon = g_Icons.value(Icon::ViewInstitutions) + QString::number(size);
  if (!QPixmapCache::find(kyIcon, pxIcon)) {
    pxIcon = QIcon::fromTheme(g_Icons.value(Icon::ViewInstitutions)).pixmap(size);
    QPixmapCache::insert(kyIcon, pxIcon);
  }
  return pxIcon;
}

QString MyMoneyInstitution::getElName(const Element el)
{
  static const QMap<Element, QString> elNames = {
    {Element::AccountID,  QStringLiteral("ACCOUNTID")},
    {Element::AccountIDS, QStringLiteral("ACCOUNTIDS")},
    {Element::Address,    QStringLiteral("ADDRESS")}
  };
  return elNames[el];
}

QString MyMoneyInstitution::getAttrName(const Attribute attr)
{
  static const QHash<Attribute, QString> attrNames = {
    {Attribute::ID,         QStringLiteral("id")},
    {Attribute::Name,       QStringLiteral("name")},
    {Attribute::Manager,    QStringLiteral("manager")},
    {Attribute::SortCode,   QStringLiteral("sortcode")},
    {Attribute::Street,     QStringLiteral("street")},
    {Attribute::City,       QStringLiteral("city")},
    {Attribute::Zip,        QStringLiteral("zip")},
    {Attribute::Telephone,  QStringLiteral("telephone")}
  };
  return attrNames[attr];
}
