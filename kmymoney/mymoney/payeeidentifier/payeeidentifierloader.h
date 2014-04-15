/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Christian David <c.david@christian-david.de>
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

#ifndef PAYEEIDENTIFIERLOADER_H
#define PAYEEIDENTIFIERLOADER_H

#include "payeeidentifier/payeeidentifier.h"

#include <QObject>
#include <QHash>
#include <QDomElement>

class payeeIdentifierLoader : public QObject
{
  Q_OBJECT

public:
  payeeIdentifierLoader();

  payeeIdentifier::ptr createPayeeIdentifier( const QString& payeeIdentifierId );
  payeeIdentifier::ptr createPayeeIdentifierFromXML( const QString& payeeIdentifierId, const QDomElement& element );
  
  /** I take ownership */
  void addPayeeIdentifier( payeeIdentifier *const identifier );
  
  static payeeIdentifierLoader* instance() { return &m_self; }
  
private:
  QHash<QString, payeeIdentifier*> m_identifiers;
  static payeeIdentifierLoader m_self;
};

#endif // PAYEEIDENTIFIERLOADER_H
