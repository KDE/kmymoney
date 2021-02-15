/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "amounteditcurrencyhelper.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountcombo.h"
#include "amountedit.h"
#include "creditdebithelper.h"
#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "securitiesmodel.h"
#include "mymoneyexception.h"

class AmountEditCurrencyHelperPrivate
{
  Q_DECLARE_PUBLIC(AmountEditCurrencyHelper)

public:
  AmountEditCurrencyHelperPrivate(AmountEditCurrencyHelper* qq)
    : q_ptr(qq)
    , category(nullptr)
    , amount(nullptr)
    , creditDebitHelper(nullptr)
  {
  }

  AmountEditCurrencyHelper*       q_ptr;
  KMyMoneyAccountCombo*           category;
  AmountEdit*                     amount;
  CreditDebitHelper*              creditDebitHelper;
  QString                         commodityId;

  void init(KMyMoneyAccountCombo* _category, AmountEdit* _amount, CreditDebitHelper* _creditDebitHelper, const QString& _commodityId)
  {
    Q_Q(AmountEditCurrencyHelper);
    category = _category;
    amount = _amount;
    creditDebitHelper = _creditDebitHelper;
    commodityId = _commodityId;

    q->connect(category, &KMyMoneyAccountCombo::accountSelected, q, &AmountEditCurrencyHelper::categoryChanged);

    q->categoryChanged(category->getSelected());
  }
};


AmountEditCurrencyHelper::AmountEditCurrencyHelper(KMyMoneyAccountCombo* category, AmountEdit* amount, const QString& commodityId)
  : QObject(category)
  , d_ptr(new AmountEditCurrencyHelperPrivate(this))
{
  Q_D(AmountEditCurrencyHelper);
  connect(amount, &QObject::destroyed, this, &QObject::deleteLater);
  d->init(category, amount, nullptr, commodityId);
}

AmountEditCurrencyHelper::AmountEditCurrencyHelper(KMyMoneyAccountCombo* category, CreditDebitHelper* amount, const QString& commodityId)
  : QObject(category)
  , d_ptr(new AmountEditCurrencyHelperPrivate(this))
{
  Q_D(AmountEditCurrencyHelper);
  connect(amount, &QObject::destroyed, this, &QObject::deleteLater);
  d->init(category, nullptr, amount, commodityId);
}

AmountEditCurrencyHelper::~AmountEditCurrencyHelper()
{
  Q_D(AmountEditCurrencyHelper);
  delete d;
}

void AmountEditCurrencyHelper::setCommodity(const QString& commodityId)
{
  Q_D(AmountEditCurrencyHelper);
  d->commodityId = commodityId;
}

void AmountEditCurrencyHelper::categoryChanged(const QString& id)
{
  Q_D(AmountEditCurrencyHelper);
  QString currencySymbol;

  try {
    const auto category = MyMoneyFile::instance()->account(id);
    if (category.isIncomeExpense()) {
      const auto security = MyMoneyFile::instance()->security(category.currencyId());
      if (security.id() != d->commodityId) {
        currencySymbol = security.tradingSymbol();
      }
    }
  } catch(MyMoneyException& e) {
  }

  if (d->amount) {
    d->amount->showCurrencySymbol(currencySymbol);
  } else if(d->creditDebitHelper) {
    d->creditDebitHelper->showCurrencySymbol(currencySymbol);
  }
}
