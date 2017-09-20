/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
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

#ifndef MYMONEYPAYEEIDENTIFIERCONTAINER_H
#define MYMONEYPAYEEIDENTIFIERCONTAINER_H

#include "kmm_mymoney_export.h"

#include <QString>
#include <QStringList>
#include <QMap>

#include "payeeidentifier/payeeidentifier.h"
#include "payeeidentifier/payeeidentifiertyped.h"

/**
 *
 *
 * @internal payeeIdentifiers should get their own id. So they can be created as all other MyMoneyObjects.
 * But adding a MyMoneyObject to MyMoneyFile and its storage backends is so time-consuming,
 * I won't do that - sorry. So all payeeIdentifiers have to be created when a MyMoneyPayeeIdentifierContainer
 * is loaded. Optimal would be if they are only created if needed (which won't be often).
 */
class KMM_MYMONEY_EXPORT MyMoneyPayeeIdentifierContainer
{
public:
  MyMoneyPayeeIdentifierContainer();

  unsigned int payeeIdentifierCount() const;
  ::payeeIdentifier payeeIdentifier(unsigned int) const;
  QList< ::payeeIdentifier > payeeIdentifiers() const;

  template< class type >
  QList< ::payeeIdentifierTyped<type> > payeeIdentifiersByType() const;

  void addPayeeIdentifier(const ::payeeIdentifier& ident);
  void addPayeeIdentifier(const unsigned int position, const ::payeeIdentifier& ident);

  void removePayeeIdentifier(const ::payeeIdentifier& ident);
  void removePayeeIdentifier(const int index);

  void modifyPayeeIdentifier(const ::payeeIdentifier& ident);
  void modifyPayeeIdentifier(const int index, const ::payeeIdentifier& ident);

  void resetPayeeIdentifiers(const QList< ::payeeIdentifier >& list = QList< ::payeeIdentifier >());

protected:
  void loadXML(QDomElement node);
  void writeXML(QDomDocument document, QDomElement parent) const;

private:
  QList< ::payeeIdentifier > m_payeeIdentifiers;
};

template< class type >
QList< payeeIdentifierTyped<type> > MyMoneyPayeeIdentifierContainer::payeeIdentifiersByType() const
{
  QList< payeeIdentifierTyped<type> > typedList;
  return typedList;
}

#endif // MYMONEYPAYEEIDENTIFIERCONTAINER_H
