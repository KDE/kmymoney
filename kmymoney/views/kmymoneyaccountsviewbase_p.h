/***************************************************************************
                          kmymoneyaccountsviewbase_p.h
                             -------------------
    copyright            : (C) 2000-2001 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                               2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>

// ----------------------------------------------------------------------------
// KDE Includes

#include "KLocalizedString"

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase_p.h"
#include "kmymoneyglobalsettings.h"
#include "mymoneyfile.h"
#include "viewenums.h"

class AccountsViewProxyModel;
class KMyMoneyAccountTreeView;
class KMyMoneyAccountsViewBasePrivate : public KMyMoneyViewBasePrivate
{
public:
  KMyMoneyAccountsViewBasePrivate() :
    m_proxyModel(nullptr),
    m_accountTree(nullptr)
  {
  }

  ~KMyMoneyAccountsViewBasePrivate()
  {
  }

  void netBalProChanged(const MyMoneyMoney &val, QLabel *label, const View view)
  {
    QString s;
    const auto isNegative = val.isNegative();
    switch (view) {
      case View::Institutions:
      case View::Accounts:
        s = i18n("Net Worth: ");
        break;
      case View::Categories:
        if (isNegative)
          s = i18n("Loss: ");
        else
          s = i18n("Profit: ");
        break;
      case View::Budget:
        s = (i18nc("The balance of the selected budget", "Balance: "));
        break;
      default:
        return;
    }

    // FIXME figure out how to deal with the approximate
    // if(!(file->totalValueValid(assetAccount.id()) & file->totalValueValid(liabilityAccount.id())))
    //  s += "~ ";

    s.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));

    if (isNegative)
      s.append(QLatin1String("<b><font color=\"red\">"));
    const auto sec = MyMoneyFile::instance()->baseCurrency();
    QString v(MyMoneyUtils::formatMoney(val, sec));
    s.append((v.replace(QLatin1Char(' '), QLatin1String("&nbsp;"))));
    if (isNegative)
      s.append(QLatin1String("</font></b>"));

    label->setFont(KMyMoneyGlobalSettings::listCellFont());
    label->setText(s);
  }

  AccountsViewProxyModel  *m_proxyModel;
  KMyMoneyAccountTreeView **m_accountTree;
};
