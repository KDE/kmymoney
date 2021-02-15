/*
    SPDX-FileCopyrightText: 2013 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
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
