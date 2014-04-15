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

#include "payeeidentifier/ibanandbic/internationalaccountidentifier.h"
#include "payeeidentifier/nationalaccount/nationalaccountid.h"

payeeIdentifierLoader payeeIdentifierLoader::m_self;

payeeIdentifierLoader::payeeIdentifierLoader()
{
  addPayeeIdentifier( new internationalAccountIdentifier() );
  addPayeeIdentifier( new nationalAccountId() );
}

void payeeIdentifierLoader::addPayeeIdentifier(payeeIdentifier* const identifier)
{
  m_identifiers.insert(identifier->payeeIdentifierId(), identifier);
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
