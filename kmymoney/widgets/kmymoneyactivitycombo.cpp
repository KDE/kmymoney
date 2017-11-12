/***************************************************************************
                          kmymoneyactivitycombo.cpp  -  description
                             -------------------
    begin                : Sat Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneyactivitycombo.h"
#include "kmymoneymvccombo_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

using namespace eMyMoney;

class KMyMoneyActivityComboPrivate : public KMyMoneyMVCComboPrivate
{
  Q_DISABLE_COPY(KMyMoneyActivityComboPrivate)

public:
  KMyMoneyActivityComboPrivate() :
    m_activity(Split::InvestmentTransactionType::UnknownTransactionType)
  {
  }

  eMyMoney::Split::InvestmentTransactionType  m_activity;
};

KMyMoneyActivityCombo::KMyMoneyActivityCombo(QWidget* w) :
    KMyMoneyMVCCombo(*new KMyMoneyActivityComboPrivate, false, w)
{
  addItem(i18n("Buy shares"), QVariant((int)Split::InvestmentTransactionType::BuyShares));
  addItem(i18n("Sell shares"), QVariant((int)Split::InvestmentTransactionType::SellShares));
  addItem(i18n("Dividend"), QVariant((int)Split::InvestmentTransactionType::Dividend));
  addItem(i18n("Reinvest dividend"), QVariant((int)Split::InvestmentTransactionType::ReinvestDividend));
  addItem(i18n("Yield"), QVariant((int)Split::InvestmentTransactionType::Yield));
  addItem(i18n("Add shares"), QVariant((int)Split::InvestmentTransactionType::AddShares));
  addItem(i18n("Remove shares"), QVariant((int)Split::InvestmentTransactionType::RemoveShares));
  addItem(i18n("Split shares"), QVariant((int)Split::InvestmentTransactionType::SplitShares));
  addItem(i18n("Interest Income"), QVariant((int)Split::InvestmentTransactionType::InterestIncome));

  connect(this, SIGNAL(itemSelected(QString)), this, SLOT(slotSetActivity(QString)));
}

KMyMoneyActivityCombo::~KMyMoneyActivityCombo()
{
}

void KMyMoneyActivityCombo::setActivity(Split::InvestmentTransactionType activity)
{
  Q_D(KMyMoneyActivityCombo);
  d->m_activity = activity;
  QString num;
  setSelectedItem(num.setNum((int)activity));
}

eMyMoney::Split::InvestmentTransactionType KMyMoneyActivityCombo::activity() const
{
  Q_D(const KMyMoneyActivityCombo);
  return d->m_activity;
}

void KMyMoneyActivityCombo::slotSetActivity(const QString& id)
{
  Q_D(KMyMoneyActivityCombo);
  QString num;
  for (auto i = (int)Split::InvestmentTransactionType::BuyShares; i <= (int)Split::InvestmentTransactionType::InterestIncome; ++i) {
    num.setNum(i);
    if (num == id) {
      d->m_activity = static_cast<Split::InvestmentTransactionType>(i);
      break;
    }
  }
  emit activitySelected(d->m_activity);
  update();
}
