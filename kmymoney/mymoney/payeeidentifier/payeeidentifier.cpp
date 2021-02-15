/*
 * SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "payeeidentifier.h"

#include <QDomDocument>
#include <QDomElement>

#include "payeeidentifierdata.h"

payeeIdentifier::payeeIdentifier()
    : m_id(0),
    m_payeeIdentifier(0)
{
}

payeeIdentifier::payeeIdentifier(const payeeIdentifier& other)
    : m_id(other.m_id),
    m_payeeIdentifier(0)
{
  if (other.m_payeeIdentifier != 0)
    m_payeeIdentifier = other.m_payeeIdentifier->clone();
}

payeeIdentifier::payeeIdentifier(payeeIdentifierData*const identifierdata)
    : m_id(0),
    m_payeeIdentifier(identifierdata)
{
}

payeeIdentifier::payeeIdentifier(const payeeIdentifier::id_t& id, payeeIdentifierData*const identifierdata)
    : m_id(id),
    m_payeeIdentifier(identifierdata)
{
}

payeeIdentifier::payeeIdentifier(const QString& id, payeeIdentifierData*const identifierdata)
    : m_id(id.mid(5).toUInt()),
    m_payeeIdentifier(identifierdata)
{
  bool ok = false; // hopefully the compiler optimizes this away if compiled in non-debug mode
  Q_ASSERT(id.mid(5).toUInt(&ok) && ok);
}

payeeIdentifier::payeeIdentifier(const payeeIdentifier::id_t& id, const payeeIdentifier& other)
    : m_id(id),
    m_payeeIdentifier(0)
{
  if (other.m_payeeIdentifier != 0)
    m_payeeIdentifier = other.m_payeeIdentifier->clone();
}

QString payeeIdentifier::idString() const
{
  QString rc;
  if (m_id != 0) {
    rc = QString("IDENT%1").arg(m_id, 6, 10, QLatin1Char('0'));
  }
  return rc;
}

payeeIdentifier::~payeeIdentifier()
{
  delete m_payeeIdentifier;
}

payeeIdentifierData* payeeIdentifier::operator->()
{
  if (m_payeeIdentifier == 0)
    throw PAYEEIDENTIFIEREMPTYEXCEPTION;
  return m_payeeIdentifier;
}

const payeeIdentifierData* payeeIdentifier::operator->() const
{
  if (m_payeeIdentifier == 0)
    throw PAYEEIDENTIFIEREMPTYEXCEPTION;
  return m_payeeIdentifier;
}

payeeIdentifierData* payeeIdentifier::data()
{
  return operator->();
}

const payeeIdentifierData* payeeIdentifier::data() const
{
  return operator->();
}

bool payeeIdentifier::isValid() const
{
  if (m_payeeIdentifier != 0)
    return m_payeeIdentifier->isValid();
  return false;
}

QString payeeIdentifier::iid() const
{
  if (m_payeeIdentifier != 0)
    return m_payeeIdentifier->payeeIdentifierId();
  return QString();
}

payeeIdentifier& payeeIdentifier::operator=(const payeeIdentifier & other)
{
  if (this == &other)
    return *this;

  m_id = other.m_id;
  if (other.m_payeeIdentifier == 0)
    m_payeeIdentifier = 0;
  else
    m_payeeIdentifier = other.m_payeeIdentifier->clone();

  return *this;
}

bool payeeIdentifier::operator==(const payeeIdentifier& other) const
{
  if (m_id != other.m_id)
    return false;

  if (isNull() || other.isNull()) {
    if (!isNull() ||  !other.isNull())
      return false;
    return true;
  }
  return (*data() == *(other.data()));
}

void payeeIdentifier::writeXML(QDomDocument& document, QDomElement& parent, const QString& elemenName) const
{
  // Important: type must be set before calling m_payeeIdentifier->writeXML()
  // the plugin for unavailable plugins must be able to set type itself
  QDomElement elem = document.createElement(elemenName);
  if (m_id != 0)
    elem.setAttribute("id", m_id);

  if (!isNull()) {
    elem.setAttribute("type", m_payeeIdentifier->payeeIdentifierId());
    m_payeeIdentifier->writeXML(document, elem);
  }
  parent.appendChild(elem);
}

