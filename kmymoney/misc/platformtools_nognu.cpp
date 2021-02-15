/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2017 Marc Hübner <mahueb55@gmail.com>
 * SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
 *
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
 */

#include "platformtools.h"

#include <windows.h>
#include <lmcons.h>
#include <process.h>
#include <locale.h>

#include <QString>

QString platformTools::osUsername()
{
  QString name;
  DWORD size = UNLEN+1;
  wchar_t  wcname[UNLEN+1];
  if(GetUserNameW((LPWSTR) wcname, &size)) {
    name = QString::fromWCharArray(wcname);
  }
  return name;
}

uint platformTools::processId()
{
    return _getpid();
}

static struct lconv* localeconv_with_init()
{
  static bool needInit = true;
  if (needInit) {
    setlocale(LC_ALL, ".UTF8");
    needInit = false;
  }
  return localeconv();
}

platformTools::currencySymbolPosition_t platformTools::currencySymbolPosition(bool negativeValues)
{
  platformTools::currencySymbolPosition_t  rc = platformTools::AfterQuantityMoneyWithSpace;
  struct lconv* lc = localeconv_with_init();
  if (lc) {
    const char precedes = negativeValues ? lc->n_cs_precedes : lc->p_cs_precedes;
    const char space = negativeValues ? lc->n_sep_by_space : lc->p_sep_by_space;
    if (precedes != 0) {
      rc = (space != 0) ? platformTools::BeforeQuantityMoneyWithSpace : platformTools::BeforeQuantityMoney;
    } else {
      rc = (space != 0) ? platformTools::AfterQuantityMoneyWithSpace : platformTools::AfterQuantityMoney;
    }
  }
  return rc;
}

platformTools::currencySignPosition_t platformTools::currencySignPosition(bool negativeValues)
{
  platformTools::currencySignPosition_t rc = platformTools::PreceedQuantityAndSymbol;
  struct lconv* lc = localeconv_with_init();
  if (lc) {
    rc = static_cast<platformTools::currencySignPosition_t>(negativeValues ? lc->n_sign_posn : lc->p_sign_posn);
  }
  return rc;
}
