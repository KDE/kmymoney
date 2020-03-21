/*
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
