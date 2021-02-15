/*
    SPDX-FileCopyrightText: 2017 Marc Hübner <mahueb55@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#ifndef PLATFORMTOOLS_H
#define PLATFORMTOOLS_H


#include <QtGlobal>

class QString;


namespace platformTools
{

  /**
   * This function returns the os username of the user account
   * under which the application is being run.
   */
  QString osUsername();

  /**
   * This function returns the PID associated with the current process.
   */
  uint processId();

  enum currencySymbolPosition_t
  {
    BeforeQuantityMoney,
    BeforeQuantityMoneyWithSpace,
    AfterQuantityMoney,
    AfterQuantityMoneyWithSpace
  };

  currencySymbolPosition_t currencySymbolPosition(bool negativeValues = false);

  enum currencySignPosition_t
  {
    ParensAround,
    PreceedQuantityAndSymbol,
    SucceedQuantityAndSymbol,
    PreceedSymbol,
    SucceedSymbol
  };

  currencySignPosition_t currencySignPosition(bool negativeValues = false);
};

#endif // PLATFORMTOOLS_H
