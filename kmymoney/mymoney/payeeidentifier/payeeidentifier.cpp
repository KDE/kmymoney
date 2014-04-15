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

#include "payeeidentifier.h"

payeeIdentifier::ptr payeeIdentifier::cloneSharedPtr() const
{
  return payeeIdentifier::ptr( this->clone() );
}

QHash< unsigned int, payeeIdentifier::ptr > payeeIdentifier::cloneList(QHash< unsigned int, payeeIdentifier::ptr > list)
{
  QHash< unsigned int, payeeIdentifier::ptr > clone;
  clone.reserve( list.count() );

  const QHash< unsigned int, payeeIdentifier::ptr >::const_iterator end = list.constEnd();
  for( QHash< unsigned int, payeeIdentifier::ptr >::const_iterator iter = list.constBegin(); iter != end; ++iter ) {
    clone.insert( iter.key(), iter.value()->cloneSharedPtr() );
  }
  return clone;
}
