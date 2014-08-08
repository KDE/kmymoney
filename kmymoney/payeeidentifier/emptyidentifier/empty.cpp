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

#include "empty.h"

namespace payeeIdentifiers {

empty* empty::clone() const
{
  return new empty;
}

void empty::writeXML(QDomDocument& document, QDomElement& parent) const
{

}

payeeIdentifier* empty::createFromXml(const QDomElement& element) const
{
  return new empty;
}

bool empty::isValid() const
{
  return true;
}

bool empty::operator==(const payeeIdentifier& other) const
{
  return ( payeeIdentifierId() == other.payeeIdentifierId() );
}

} // namespace payeeIdentifiers