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

#ifndef EMPTY_H
#define EMPTY_H

#include "emptyidentifiermacros.h"
#include "mymoney/payeeidentifier/payeeidentifier.h"

namespace payeeIdentifiers {

/**
 * @brief A dummy payeeIdentifier which has no content
 *
 * It can be used if a null_ptr should not be used. It is also possible to
 * save it.
 */
class EMPTY_IDENTIFIER_EXPORT empty : public payeeIdentifier
{
public:
  PAYEEIDENTIFIER_ID(empty, "org.kmymoney.payeeIdentifier.empty");

  virtual void writeXML(QDomDocument& document, QDomElement& parent) const;
  virtual payeeIdentifier* createFromXml(const QDomElement& element) const;
  virtual bool isValid() const;
  virtual bool operator==(const payeeIdentifier& other) const;
protected:
  virtual empty* clone() const;
};

} // namespace payeeidentifiers

#endif // EMPTY_H
