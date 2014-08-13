/***************************************************************************
                          mymoneyinstitution.cpp
                          -------------------
    copyright            : (C) 2001 by Michael Edwardes <mte@users.sourceforge.net>
                               2002-2005 by Thomas Baumgart <ipwizard@users.sourceforge.net>

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

#include <QPixmap>
#include <QPixmapCache>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KIconLoader>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyexception.h>

MyMoneyInstitution::MyMoneyInstitution()
{
}

MyMoneyInstitution::MyMoneyInstitution(const QString& id, const MyMoneyInstitution& right) :
    MyMoneyObject(id)
{
  *this = right;
  m_id = id;
}

MyMoneyInstitution::MyMoneyInstitution(const QString& name,
                                       const QString& town,
                                       const QString& street,
                                       const QString& postcode,
                                       const QString& telephone,
                                       const QString& manager,
                                       const QString& sortcode)
{
  clearId();
  m_name = name;
  m_town = town;
  m_street = street;
  m_postcode = postcode;
  m_telephone = telephone;
  m_manager = manager;
  m_sortcode = sortcode;
}

MyMoneyInstitution::MyMoneyInstitution(const QDomElement& node) :
    MyMoneyObject(node),
    MyMoneyKeyValueContainer(node.elementsByTagName("KEYVALUEPAIRS").item(0).toElement())
{
  if ("INSTITUTION" != node.tagName())
    throw MYMONEYEXCEPTION("Node was not INSTITUTION");

  m_sortcode = node.attribute("sortcode");
  m_name = node.attribute("name");
  m_manager = node.attribute("manager");

  QDomNodeList nodeList = node.elementsByTagName("ADDRESS");
  if (nodeList.count() == 0) {
    QString msg = QString("No ADDRESS in institution %1").arg(m_name);
    throw MYMONEYEXCEPTION(msg);
  }

  QDomElement addrNode = nodeList.item(0).toElement();
  m_street = addrNode.attribute("street");
  m_town = addrNode.attribute("city");
  m_postcode = addrNode.attribute("zip");
  m_telephone = addrNode.attribute("telephone");

  m_accountList.clear();

  nodeList = node.elementsByTagName("ACCOUNTIDS");
  if (nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName("ACCOUNTID");
    for (int i = 0; i < nodeList.count(); ++i) {
      m_accountList << nodeList.item(i).toElement().attribute("id");
    }
  }
}

MyMoneyInstitution::~MyMoneyInstitution()
{
}

void MyMoneyInstitution::addAccountId(const QString& account)
{
  // only add this account if it is not yet presently in the list
  if (m_accountList.contains(account) == 0)
    m_accountList.append(account);
}

QString MyMoneyInstitution::removeAccountId(const QString& account)
{
  int pos;
  QString rc;

  pos = m_accountList.indexOf(account);
  if (pos != -1) {
    m_accountList.removeAt(pos);
    rc = account;
  }
  return rc;
}

bool MyMoneyInstitution::operator < (const MyMoneyInstitution& right) const
{
  return m_name < right.m_name;
}

bool MyMoneyInstitution::operator == (const MyMoneyInstitution& right) const
{
  if (MyMoneyObject::operator==(right) &&
      ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)) &&
      ((m_town.length() == 0 && right.m_town.length() == 0) || (m_town == right.m_town)) &&
      ((m_street.length() == 0 && right.m_street.length() == 0) || (m_street == right.m_street)) &&
      ((m_postcode.length() == 0 && right.m_postcode.length() == 0) || (m_postcode == right.m_postcode)) &&
      ((m_telephone.length() == 0 && right.m_telephone.length() == 0) || (m_telephone == right.m_telephone)) &&
      ((m_sortcode.length() == 0 && right.m_sortcode.length() == 0) || (m_sortcode == right.m_sortcode)) &&
      ((m_manager.length() == 0 && right.m_manager.length() == 0) || (m_manager == right.m_manager)) &&
      (m_accountList == right.m_accountList)) {
    return true;
  } else
    return false;
}

void MyMoneyInstitution::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("INSTITUTION");

  writeBaseXML(document, el);

  el.setAttribute("name", m_name);
  el.setAttribute("manager", m_manager);
  el.setAttribute("sortcode", m_sortcode);

  QDomElement address = document.createElement("ADDRESS");
  address.setAttribute("street", m_street);
  address.setAttribute("city", m_town);
  address.setAttribute("zip", m_postcode);
  address.setAttribute("telephone", m_telephone);
  el.appendChild(address);


  QDomElement accounts = document.createElement("ACCOUNTIDS");
  for (QStringList::ConstIterator it = accountList().begin(); it != accountList().end(); ++it) {
    QDomElement temp = document.createElement("ACCOUNTID");
    temp.setAttribute("id", (*it));
    accounts.appendChild(temp);
  }
  el.appendChild(accounts);

  //Add in Key-Value Pairs for institutions.
  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneyInstitution::hasReferenceTo(const QString& /* id */) const
{
  bool rc = false;
  return rc;
}

QPixmap MyMoneyInstitution::pixmap() const
{
  QPixmap institution;
  if (!QPixmapCache::find("view-bank", institution)) {
    institution = DesktopIcon("view-bank");
    QPixmapCache::insert("view-bank", institution);
  }
  return institution;
}

