/*
  This file is part of KMyMoney, A Personal Finance Manager by KDE
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


#ifndef CREDITTRANSFER_H
#define CREDITTRANSFER_H

#include <QtPlugin>
#include "payeeidentifier/payeeidentifier.h"

class QValidator;
class MyMoneyMoney;
class MyMoneySecurity;

/**
 * @brief Describes an online credit-transfer (or similar)
 *
 * This class is used by KMyMoney to create a MyMoneySchedule
 * after a task was sent to the bank.
 */
class creditTransfer
{

public:
  virtual ~creditTransfer();

  virtual MyMoneyMoney value() const = 0;
  /** @brief The currency the transfer value is in */
  virtual MyMoneySecurity currency() const = 0;

  virtual QString purpose() const = 0;

  virtual QString responsibleAccount() const = 0;

  /**
   * @brief payeeIdentifier of recipient
   *
   * The return must never be null_ptr!
   */
  virtual payeeIdentifier beneficiary() const = 0;

  /**
   * @brief
   * @return
   * @todo Move (logic) to a utils class?
   */
  virtual QString jobTypeName() const;
};

Q_DECLARE_INTERFACE(creditTransfer, "org.kmymoney.onlineTasks.creditTransfer")

#endif // CREDITTRANSFER_H
