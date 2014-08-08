/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "payeeidentifier/payeeidentifierloader.h"

#include "payeeidentifier/ibanandbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "payeeidentifier/emptyidentifier/empty.h"

#include "payeeidentifier/ibanandbic/widgets/ibanbicitemdelegate.h"
#include "payeeidentifier/emptyidentifier/widgets/typeselectiondelegate.h"

#include <QDebug>
#include <QAbstractItemDelegate>

#include <KLocalizedString>
#include <KServiceTypeTrader>

payeeIdentifierLoader payeeIdentifierLoader::m_self;

payeeIdentifierLoader::payeeIdentifierLoader()
  : m_identifiers( QHash<QString, payeeIdentifier*>() )
{
  addPayeeIdentifier( new payeeIdentifiers::ibanBic() );
  addPayeeIdentifier( new payeeIdentifiers::nationalAccount() );
  addPayeeIdentifier( new payeeIdentifiers::empty() );
}

void payeeIdentifierLoader::addPayeeIdentifier(payeeIdentifier* const identifier)
{
  m_identifiers.insert(identifier->payeeIdentifierId(), identifier);
}

payeeIdentifier::ptr payeeIdentifierLoader::createPayeeIdentifier(const QString& payeeIdentifierId)
{
  const payeeIdentifier* ident = m_identifiers.value( payeeIdentifierId );
  if ( ident != 0 ) {
    return ident->cloneSharedPtr();
  }

  return payeeIdentifier::ptr();
}

payeeIdentifier::ptr payeeIdentifierLoader::createPayeeIdentifierFromXML(const QString& payeeIdentifierId, const QDomElement& element)
{
  const payeeIdentifier* ident = m_identifiers.value( payeeIdentifierId );
  if ( ident != 0 ) {
    payeeIdentifier* newIdent = ident->createFromXml( element );
    return payeeIdentifier::ptr( newIdent );
  }

  return payeeIdentifier::ptr();
}

/**
 * @todo enable delegates again
 */
QAbstractItemDelegate* payeeIdentifierLoader::createItemDelegate(const QString& payeeIdentifierId, QObject* parent)
{
  /** @todo escape ' in  payeeIdentifierId */
  KService::List offers = KServiceTypeTrader::self()->query(QLatin1String("KMyMoney/PayeeIdentifierDelegate"), QString("'%1' ~in [X-KMyMoney-payeeIdentifierIds]").arg(payeeIdentifierId));
  qDebug() << "Got sooo many offers:" << QString("'%1' ~in [X-KMyMoney-payeeIdentifierIds]").arg(payeeIdentifierId) << offers;
  if ( !offers.isEmpty() ) {
    QString error;
    QAbstractItemDelegate* ptr = offers.at(0)->createInstance<QAbstractItemDelegate>(parent, QVariantList(), &error);
    qDebug() << "My ptr" << ptr << offers.at(0)->library() << offers.at(0)->pluginKeyword() << error;
    return ptr;
  }
  return 0;
}

bool payeeIdentifierLoader::hasItemEditDelegate(const QString& payeeIdentifierId)
{
  QAbstractItemDelegate* delegate = createItemDelegate(payeeIdentifierId);
  const bool ret = ( delegate != 0 );
  delete delegate;
  return ret;
}

QStringList payeeIdentifierLoader::availableDelegates()
{
  QStringList list;
  list << payeeIdentifiers::ibanBic::staticPayeeIdentifierId();
  return list;
}

QString payeeIdentifierLoader::translatedDelegateName(const QString& payeeIdentifierId)
{
  if ( payeeIdentifierId == payeeIdentifiers::ibanBic::staticPayeeIdentifierId() )
    return i18n("IBAN and BIC");
  else if ( payeeIdentifierId == payeeIdentifiers::nationalAccount::staticPayeeIdentifierId() )
    return i18n("National Account Number");

  return QString();
}
