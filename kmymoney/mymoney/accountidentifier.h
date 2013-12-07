/*
  This file is part of KMyMoney, A Personal Finance Manager for KDE
  Copyright (C) 2013 Christian Dávid <christian-david@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef ACCOUNTIDENTIFIER_H
#define ACCOUNTIDENTIFIER_H

#include <QString>
#include <QIntValidator>

/**
 * @brief Identifies a real world account
 * 
 * All accounts world wide seem to have an owner identified by a string.
 * Also they are always hosted somewhere.
 *
 * This class is meant to be really abstract to support things like a paypal account or
 * credit cards in future.
 *
 * @todo allow setting of account identifier for each KMyMoneyAccount individually
 */
class accountIdentifier
{ 
public:
  accountIdentifier() {}
  accountIdentifier(const accountIdentifier& other)
    : _ownerName(other._ownerName)
  {}
  virtual ~accountIdentifier() {}
  
  /**
   * @brief Name of the hosting bank
   * Can be QString() if no name is set or known. It should be determined by a bank code, not user input.
   */
  virtual QString getBankName() const = 0;
  
  /**
   * @brief Verifies the accountIdentifier
   * The rules if a accountIdentifier is valid depend on the type of accountIdentifier
   */
  virtual bool isValid() const = 0;
  
  void setOwnerName( const QString& name ) { _ownerName = name; }
  QString ownerName() const { return _ownerName; }
  
protected:
  QString _ownerName;
};

#endif // ACCOUNTIDENTIFIER_H
