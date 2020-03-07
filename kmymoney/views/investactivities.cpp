/*
 * Copyright 2007-2020  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "investactivities.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "investtransactioneditor.h"
#include "mymoneymoney.h"
#include "kmymoneyaccountcombo.h"
#include "amountedit.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneycompletion.h"
#include "kmymoneysettings.h"
#include "mymoneyfile.h"
#include "mymoneysplit.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "dialogenums.h"
#include "mymoneyenums.h"

using namespace Invest;

class Invest::ActivityPrivate
{
  Q_DISABLE_COPY(ActivityPrivate)

public:
  ActivityPrivate(InvestTransactionEditor* parent) :
    m_parent(parent),
    m_memoChanged(false)
  {
  }

  template <typename T>
  inline T* haveWidget(const QString &aName) const
  {
    return m_parent->findChild<T*>(aName);
  }

  template <typename T>
  inline T* haveVisibleWidget(const QString &aName) const
  {
    auto widget = m_parent->findChild<T*>(aName);
    if (!widget) {
      qDebug() << "Widget with name" << aName << "not found";
    }
    if (widget && widget->isVisible())
      return widget;
    return nullptr;
  }


  InvestTransactionEditor*     m_parent;
  QMap<QString, MyMoneyMoney>  m_priceInfo;
  QString                      m_memoText;
  bool                         m_memoChanged;
};


Activity::Activity(InvestTransactionEditor* editor) :
   d_ptr(new ActivityPrivate(editor))
{
}

Activity::~Activity()
{
  Q_D(Activity);
  delete d;
}

void Invest::Activity::setupWidgets(const QStringList& activityWidgets) const
{
  Q_D(const Activity);

  static const QStringList dynamicWidgetNames = {
    "sharesLabel", "sharesAmountEdit",
    "accountLabel", "accountCombo",
    "priceLabel", "priceAmountEdit",
    "feesLabel", "feesCombo", "feesSplitEditorButton", "feesAmountLabel", "feesAmountEdit",
    "interestLabel", "interestCombo", "interestSplitEditorButton", "interestAmountLabel", "interestAmountEdit",
    "totalLabel", "totalAmountEdit",
  };

  setWidgetVisibility(dynamicWidgetNames, false);

  setLabelText("priceLabel", priceLabelText());

  static const QStringList standardWidgetNames = {
    "activityLabel", "activityCombo",
    "dateLabel", "dateEdit",
    "securityLabel", "securityCombo",

    // the ones in between are dynamically handled

    "memoLabel", "memoEdit",
    "statusLabel", "statusCombo",
    "enterButton", "cancelButton",
  };

  setWidgetVisibility(standardWidgetNames, true);
  setWidgetVisibility(activityWidgets, true);

}

bool& Activity::memoChanged()
{
  Q_D(Activity);
  return d->m_memoChanged;
}

QString& Activity::memoText()
{
  Q_D(Activity);
  return d->m_memoText;
}

bool Activity::isComplete(QString& reason) const
{
  Q_UNUSED(reason)

  Q_D(const Activity);
  auto rc = false;
  auto security = d->haveWidget<KMyMoneyAccountCombo>("security");

  if (security && !security->getSelected().isEmpty()) {
    // rc = (security->selector()->contains(security->currentText()) || (isMultiSelection() && d->m_memoChanged));
    rc = true;
  }
  return rc;
}

bool Activity::haveAssetAccount() const
{
  Q_D(const Activity);
  auto rc = true;
  auto cat = d->haveWidget<KMyMoneyAccountCombo>("accountCombo");
  if (!cat)
    return false;

  if (!isMultiSelection())
    rc = !cat->getSelected().isEmpty();

  return rc;
}

bool Activity::haveCategoryAndAmount(const QString& categoryWidget, const QString& amountWidget, bool optional) const
{
  Q_D(const Activity);
  auto cat = d->haveVisibleWidget<KMyMoneyAccountCombo>(categoryWidget);
  auto amount = d->haveVisibleWidget<AmountEdit>(amountWidget);
  auto rc = true;

  if (cat && amount) {
    if (cat->isEnabled()) {
      rc = !cat->getSelected().isEmpty() || optional;
    }
    rc = !amount->value().isZero() || optional;
  }
  /// @todo port to new model code
#if 0
  if (cat && !cat->currentText().isEmpty()) {
    rc = cat->selector()->contains(cat->currentText()) || cat->isSplitTransaction();
    if (rc && !amount.isEmpty() && !isMultiSelection()) {
      if (cat->isSplitTransaction()) {
          /// @todo port to new model code
#if 0
        QList<MyMoneySplit>::const_iterator split;
        QList<MyMoneySplit>::const_iterator splitEnd;

        if (category == "feesCombo") {
          split = d->m_parent->feeSplits().cbegin();
          splitEnd = d->m_parent->feeSplits().cend();
        } else if (category == "interestCombo") {
          split = d->m_parent->interestSplits().cbegin();
          splitEnd = d->m_parent->interestSplits().cend();
        }

        for (; split != splitEnd; ++split) {
          if ((*split).value().isZero())
            rc = false;
        }
#endif
      } else {
        if (auto valueWidget = d->haveWidget<AmountEdit>(amount))
          rc = !valueWidget->value().isZero();
      }
    }
  } else if (!isMultiSelection() && !optional) {
    rc = false;
  }
#endif
  return rc;
}

bool Activity::haveFees(bool optional) const
{
  return haveCategoryAndAmount("feesCombo", "feesAmountEdit", optional);
}

bool Activity::haveInterest(bool optional) const
{
  return haveCategoryAndAmount("interestCombo", "interestAmountEdit", optional);
}

bool Activity::haveShares() const
{
  Q_D(const Activity);
  if (auto amount = d->haveWidget<AmountEdit>("sharesAmountEdit")) {
    if (isMultiSelection() && amount->value().isZero())
      return true;

    return !amount->value().isZero();
  }
  return false;
}

bool Activity::havePrice() const
{
  Q_D(const Activity);
  if (auto amount = d->haveWidget<AmountEdit>("priceAmountEdit")) {
    if (isMultiSelection() && amount->value().isZero())
      return true;

    return !amount->value().isZero();
  }
  return false;
}

bool Activity::isMultiSelection() const
{
  Q_D(const Activity);
  /// @todo port to new model code
  // return d->m_parent->isMultiSelection();
  return false;
}

bool Activity::createCategorySplits(const MyMoneyTransaction& t, KMyMoneyAccountCombo* cat, AmountEdit* amount, MyMoneyMoney factor, QList<MyMoneySplit>&splits, const QList<MyMoneySplit>& osplits) const
{
  Q_D(const Activity);
  auto rc = true;
  if (!isMultiSelection() || !cat->currentText().isEmpty()) {
    /// @todo port to new model code
#if 0
    if (!cat->isSplitTransaction()) {
      splits.clear();
      MyMoneySplit s1;
      // base the resulting split on the original split
      // that was provided which avoids loosing data
      // stored in the split
      if (!osplits.isEmpty()) {
        s1 = osplits.first();
        s1.clearId();
      }
      QString categoryId;
      categoryId = cat->selectedItem();
      if (!categoryId.isEmpty()) {
        s1.setAccountId(categoryId);
        s1.setValue(amount->value() * factor);
        /// @todo port to new model code
#if 0
        if (!s1.value().isZero()) {
          rc = d->m_parent->setupPrice(t, s1);
        }
#endif
        splits.append(s1);
      }
    } else {
      splits = osplits;
    }
#endif
  }
  return rc;
}

void Activity::createAssetAccountSplit(MyMoneySplit& split, const MyMoneySplit& stockSplit) const
{
  Q_D(const Activity);
  auto cat = d->haveWidget<KMyMoneyAccountCombo>("accountCombo");
  /// @todo port to new model code
#if 0
  if (cat && (!isMultiSelection() || !cat->currentText().isEmpty())) {
    auto categoryId = cat->selectedItem();
    split.setAccountId(categoryId);
  }
#endif
  split.setMemo(stockSplit.memo());
}

MyMoneyMoney Activity::sumSplits(const MyMoneySplit& s0, const QList<MyMoneySplit>& feeSplits, const QList<MyMoneySplit>& interestSplits) const
{
  auto total = s0.value();
  /// @todo port to new model code
#if 0
  foreach (const auto feeSplit, feeSplits)
    total += feeSplit.value();

  foreach (const auto interestSplit, interestSplits)
    total += interestSplit.value();
#endif
  return total;
}

MyMoneyMoney Activity::totalAmount() const
{
  Q_D(const Activity);
  auto result = shareValue().abs();
  qDebug() << "Start";
  qDebug() << "result " << result.formatMoney(100);
  const auto fees = d->haveVisibleWidget<AmountEdit>("feesAmountEdit");
  const auto interest = d->haveVisibleWidget<AmountEdit>("interestAmountEdit");
  if (interest) {
    result += interest->value().abs();
  }
  qDebug() << "result " << result.formatMoney(100);
  if (fees) {
    result -= fees->value().abs();
  }
  qDebug() << "result " << result.formatMoney(100);
  return result;
}

MyMoneyMoney Activity::shareValue() const
{
  Q_D(const Activity);
  const auto shares = d->haveVisibleWidget<AmountEdit>("sharesAmountEdit");
  const auto price = d->haveVisibleWidget<AmountEdit>("priceAmountEdit");
  MyMoneyMoney result;
  if (shares && price) {
    if (priceMode() == eDialogs::PriceMode::PricePerShare) {
      result = shares->value() * price->value();
    } else {
      result = price->value();
    }
  }
  return result;
}

void Activity::setLabelText(const QString& idx, const QString& txt) const
{
  Q_D(const Activity);
  auto w = d->haveWidget<QLabel>(idx);
  if (w) {
    w->setText(txt);
  }
}

void Activity::preloadAssetAccount()
{
  Q_D(Activity);
  auto cat = d->haveWidget<KMyMoneyAccountCombo>("accountCombo");
  if (cat && cat->isVisible()) {
    /// @todo port to new model code
#if 0
    if (cat->currentText().isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->accountByName(i18n("%1 (Brokerage)", d->m_parent->account().name()));
      if (!acc.id().isEmpty()) {
        bool blocked = cat->signalsBlocked();
        // block signals, so that the focus does not go crazy
        cat->blockSignals(true);
        cat->completion()->setSelected(acc.id());
        cat->slotItemSelected(acc.id());
        cat->blockSignals(blocked);
      }
    }
#endif
  }
}

void Activity::setWidgetVisibility(const QStringList& widgetIds, bool visible) const
{
  Q_D(const Activity);
  for (QStringList::const_iterator it_w = widgetIds.constBegin(); it_w != widgetIds.constEnd(); ++it_w) {
    auto w = d->haveWidget<QWidget>(*it_w);
    if (w) {
      /// @todo port to new model code
#if 0
      // in case we hit a category with a split button,
      // we need to manipulate the enclosing QFrame
      auto cat = qobject_cast<KMyMoneyAccountCombo>(w);
      if (cat && cat->splitButton()) {
        cat->parentWidget()->setVisible(visible);
      } else {
#endif
        w->setVisible(visible);
      // }
    }
  }
}

QString Activity::priceLabelText() const
{
  QString label;
  if (priceMode() == eDialogs::PriceMode::Price) {
    label = i18n("Price");
  } else if (priceMode() == eDialogs::PriceMode::PricePerShare) {
    label = i18n("Price/share");
  } else if (priceMode() == eDialogs::PriceMode::PricePerTransaction) {
    label = i18n("Transaction amount");
  }
  return label;
}

eDialogs::PriceMode Activity::priceMode() const
{
  Q_D(const Activity);
  eDialogs::PriceMode mode = eDialogs::PriceMode::Price;
  auto sec = d->haveWidget<QComboBox>("securityCombo");
  // auto sec = dynamic_cast<KMyMoneySecurity*>(m_editWidgets["security"]);

  QString accId;
  if (sec && !sec->currentText().isEmpty()) {
    const auto idx = sec->model()->index(sec->currentIndex(), 0);
    accId = idx.data(eMyMoney::Model::IdRole).toString();
#if 0
    if (accId.isEmpty())
      accId = d->m_account.id();
#endif
  }
  while (!accId.isEmpty() && mode == eDialogs::PriceMode::Price) {
    auto acc = MyMoneyFile::instance()->account(accId);
    if (acc.value("priceMode").isEmpty())
      accId = acc.parentAccountId();
    else
      mode = static_cast<eDialogs::PriceMode>(acc.value("priceMode").toInt());
  }

  // if mode is still <Price> then use that
  if (mode == eDialogs::PriceMode::Price)
    mode = eDialogs::PriceMode::PricePerShare;
  return mode;
}

void Activity::loadPriceWidget(const MyMoneySplit & split)
{
  Q_D(const Activity);
  auto priceEdit = d->haveWidget<AmountEdit>("priceAmountEdit");

  if (priceEdit) {
    if (priceMode() == eDialogs::PriceMode::PricePerTransaction) {
      priceEdit->setValue(split.value());
    } else {
      priceEdit->setValue(split.price());
    }
  }
}

MyMoneyMoney Activity::sharesFactor() const
{
  return MyMoneyMoney::ONE;
}

MyMoneyMoney Activity::feesFactor() const
{
  return MyMoneyMoney::ONE;
}

MyMoneyMoney Activity::interestFactor() const
{
  return MyMoneyMoney::MINUS_ONE;
}





Buy::Buy(InvestTransactionEditor* editor) :
  Activity(editor)
{
}

Buy::~Buy()
{
}

eMyMoney::Split::InvestmentTransactionType Buy::type() const
{
  return eMyMoney::Split::InvestmentTransactionType::BuyShares;
}

void Buy::showWidgets() const
{
  Q_D(const Activity);
  static const QStringList activityWidgets = {
    "sharesLabel", "sharesAmountEdit",
    "accountLabel", "accountCombo",
    "priceLabel", "priceAmountEdit",
    "feesLabel", "feesCombo", "feesSplitEditorButton", "feesAmountLabel", "feesAmountEdit",
    // "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
    "totalLabel", "totalAmountEdit",
  };

  setupWidgets(activityWidgets);
}

MyMoneyMoney Buy::totalAmount() const
{
  Q_D(const Activity);
  auto result = shareValue().abs();
  const auto fees = d->haveVisibleWidget<AmountEdit>("feesAmountEdit");
  if (fees) {
    result += fees->value().abs();
  }
  return result;
}

bool Buy::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveFees(true);
  rc &= haveShares();
  rc &= havePrice();

  return rc;
}

bool Buy::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(m_interestSplits)
  Q_UNUSED(security)
  Q_UNUSED(currency)

  Q_D(Activity);
  QString reason;
  if (!isComplete(reason))
    return false;

  auto sharesEdit = d->haveWidget<AmountEdit>("sharesAmountEdit");
  auto priceEdit = d->haveWidget<AmountEdit>("priceAmountEdit");

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::BuyShares);

  MyMoneyMoney shares = s0.shares();
  MyMoneyMoney price;
  if (!s0.shares().isZero())
    price = (s0.value() / s0.shares()).reduce();

  if (sharesEdit && (!isMultiSelection() || !sharesEdit->value().isZero())) {
    shares = sharesEdit->value().abs();
    s0.setShares(shares);
    s0.setValue((shares * price).reduce());
    s0.setPrice(price);
  }
  if (priceEdit && (!isMultiSelection() || !priceEdit->value().isZero())) {
    price = priceEdit->value().abs();
    if (priceMode() == eDialogs::PriceMode::PricePerTransaction) {
      s0.setValue(price.reduce());
      if (!s0.shares().isZero())
        s0.setPrice((price / s0.shares()).reduce());
    } else {
      s0.setValue((shares * price).reduce());
      s0.setPrice(price);
    }
  }

  auto feeAccountWidget = d->haveWidget<KMyMoneyAccountCombo>("feesCombo");
  auto feeAmountWidget = d->haveWidget<AmountEdit>("feesAmountEdit");
  if (!feeAccountWidget || !feeAmountWidget ||
      !createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, QList<MyMoneySplit>());

  //  Clear any leftover value from previous Dividend.
  interestSplits.clear();

  assetAccountSplit.setValue(-total);

  /// @todo port to new model code
#if 0
  if (!d->m_parent->setupPrice(t, assetAccountSplit))
    return false;
#endif

  return true;
}

Sell::Sell(InvestTransactionEditor* editor) :
  Activity(editor)
{
}

Sell::~Sell()
{
}

eMyMoney::Split::InvestmentTransactionType Sell::type() const
{
  return eMyMoney::Split::InvestmentTransactionType::SellShares;
}

void Sell::showWidgets() const
{
  Q_D(const Activity);

  static const QStringList activityWidgets = {
    "sharesLabel", "sharesAmountEdit",
    "accountLabel", "accountCombo",
    "priceLabel", "priceAmountEdit",
    "feesLabel", "feesCombo", "feesSplitEditorButton", "feesAmountLabel", "feesAmountEdit",
    "interestLabel", "interestCombo", "interestSplitEditorButton", "interestAmountLabel", "interestAmountEdit",
    "totalLabel", "totalAmountEdit",
  };

  setupWidgets(activityWidgets);
}

bool Sell::isComplete(QString& reason) const
{
  Q_D(const Activity);

  auto rc = Activity::isComplete(reason);
  rc &= haveFees(true);
  rc &= haveInterest(true);
  rc &= haveShares();
  rc &= havePrice();

  // Allow a sell operation to be saved without specifying a brokerage
  // account, when the proceeds equal the fees. This will handle sales
  // made solely to cover annual account fees, where there is no money
  // transferred.
  if (rc) {
    if (!d->m_parent->totalAmount().isZero()) {
      rc &= haveAssetAccount();
    }
  }
  return rc;
}

bool Sell::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(m_interestSplits)
  Q_UNUSED(security)
  Q_UNUSED(currency)

  Q_D(Activity);
  QString reason;
  if (!isComplete(reason))
    return false;

  auto sharesEdit = d->haveWidget<AmountEdit>("sharesAmountEdit");
  auto priceEdit = d->haveWidget<AmountEdit>("priceAmountEdit");

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::BuyShares);

  MyMoneyMoney shares = s0.shares();
  MyMoneyMoney price;
  if (!s0.shares().isZero())
    price = (s0.value() / s0.shares()).reduce();

  if (sharesEdit && (!isMultiSelection() || !sharesEdit->value().isZero())) {
    shares = -sharesEdit->value().abs();
    s0.setShares(shares);
    s0.setValue((shares * price).reduce());
    s0.setPrice(price);
  }
  if (priceEdit && (!isMultiSelection() || !priceEdit->value().isZero())) {
    price = priceEdit->value().abs();
    if (priceMode() == eDialogs::PriceMode::PricePerTransaction) {
      price = -price;
      s0.setValue(price.reduce());
      if (!s0.shares().isZero())
        s0.setPrice((price / s0.shares()).reduce());
    } else {
      s0.setValue((shares * price).reduce());
      s0.setPrice(price);
    }
  }

  auto feeAccountWidget = d->haveWidget<KMyMoneyAccountCombo>("feesCombo");
  auto feeAmountWidget = d->haveWidget<AmountEdit>("feesAmountEdit");
  if (!feeAccountWidget || !feeAmountWidget ||
      !createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  auto interestAccountWidget = d->haveWidget<KMyMoneyAccountCombo>("interestCombo");
  auto interestAmountWidget = d->haveWidget<AmountEdit>("interestAmountEdit");
  if (!interestAccountWidget || !interestAmountWidget ||
      !createCategorySplits(t, interestAccountWidget, interestAmountWidget, MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  const auto total = sumSplits(s0, feeSplits, interestSplits);
  if (!total.isZero()) {
    createAssetAccountSplit(assetAccountSplit, s0);
    assetAccountSplit.setValue(-total);

    /// @todo port to new model code
#if 0
    if (!d->m_parent->setupPrice(t, assetAccountSplit))
      return false;
#endif
  }

  return true;
}

MyMoneyMoney Sell::sharesFactor() const
{
  return MyMoneyMoney::MINUS_ONE;
}

Div::Div(InvestTransactionEditor* editor) :
  Activity(editor)
{
}

Div::~Div()
{
}

eMyMoney::Split::InvestmentTransactionType Div::type() const
{
  return eMyMoney::Split::InvestmentTransactionType::Dividend;
}

void Div::showWidgets() const
{
  static const QStringList activityWidgets = {
    "accountLabel", "accountCombo",
    "feesLabel", "feesCombo", "feesSplitEditorButton", "feesAmountLabel", "feesAmountEdit",
    "interestLabel", "interestCombo", "interestSplitEditorButton", "interestAmountLabel", "interestAmountEdit",
    "totalLabel", "totalAmountEdit",
  };

  setupWidgets(activityWidgets);
}

bool Div::isComplete(QString& reason) const
{
  Q_UNUSED(reason)

  auto rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveCategoryAndAmount("interestCombo", QString(), false);
  rc &= haveInterest(false);
  return rc;
}

bool Div::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(m_feeSplits)
  Q_UNUSED(security)
  Q_UNUSED(currency)

  Q_D(Activity);
  QString reason;
  if (!isComplete(reason))
    return false;

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::Dividend);

  // for dividends, we only use the stock split as a marker
  MyMoneyMoney shares;
  s0.setShares(shares);
  s0.setValue(shares);
  s0.setPrice(MyMoneyMoney::ONE);

  auto feeAccountWidget = d->haveWidget<KMyMoneyAccountCombo>("feesCombo");
  auto feeAmountWidget = d->haveWidget<AmountEdit>("feesAmountEdit");
  if (!feeAccountWidget || !feeAmountWidget ||
      !createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  auto interestAccountWidget = d->haveWidget<KMyMoneyAccountCombo>("interestCombo");
  auto interestAmountWidget = d->haveWidget<AmountEdit>("interestAmountEdit");
  if (!interestAccountWidget || !interestAmountWidget ||
      !createCategorySplits(t, interestAccountWidget, interestAmountWidget, MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
  assetAccountSplit.setValue(-total);

  /// @todo port to new model code
#if 0
  if (!d->m_parent->setupPrice(t, assetAccountSplit))
    return false;
#endif

  return true;
}

Reinvest::Reinvest(InvestTransactionEditor* editor) :
  Activity(editor)
{
}

Reinvest::~Reinvest()
{
}

eMyMoney::Split::InvestmentTransactionType Reinvest::type() const
{
  return eMyMoney::Split::InvestmentTransactionType::ReinvestDividend;
}

void Reinvest::showWidgets() const
{
  Q_D(const Activity);

  static const QStringList activityWidgets = {
    "sharesLabel", "sharesAmountEdit",
    "accountLabel", "accountCombo",
    "priceLabel", "priceAmountEdit",
    "feesLabel", "feesCombo", "feesSplitEditorButton", "feesAmountLabel", "feesAmountEdit",
    "interestLabel", "interestCombo", "interestSplitEditorButton", "interestAmountLabel", "interestAmountEdit",
    "totalLabel", "totalAmountEdit",
  };

  setupWidgets(activityWidgets);

  if (auto shareEdit = d->haveWidget<AmountEdit>("sharesAmountEdit")) {
    shareEdit->show();
    /// @todo port to new model code
#if 0
    shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));
#endif
  }
}

bool Reinvest::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveCategoryAndAmount("interestCombo", QString(), false);
  rc &= haveFees(true);
  rc &= haveShares();
  rc &= havePrice();
  return rc;
}

bool Reinvest::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(assetAccountSplit)
  Q_UNUSED(security)
  Q_UNUSED(currency)

  Q_D(Activity);
  QString reason;
  if (!isComplete(reason))
    return false;

  auto sharesEdit = d->haveWidget<AmountEdit>("sharesAmountEdit");
  auto priceEdit = d->haveWidget<AmountEdit>("priceAmountEdit");

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::ReinvestDividend);

  MyMoneyMoney shares = s0.shares();
  MyMoneyMoney price;
  if (!s0.shares().isZero())
    price = (s0.value() / s0.shares()).reduce();

  if (sharesEdit && (!isMultiSelection() || !sharesEdit->value().isZero())) {
    shares = sharesEdit->value().abs();
    s0.setShares(shares);
    s0.setValue((shares * price).reduce());
    s0.setPrice(price);
  }
  if (priceEdit && (!isMultiSelection() || !priceEdit->value().isZero())) {
    price = priceEdit->value().abs();
    if (priceMode() == eDialogs::PriceMode::PricePerTransaction) {
      s0.setValue(price.reduce());
      if (!s0.shares().isZero())
        s0.setPrice((price / s0.shares()).reduce());
    } else {
      s0.setValue((shares * price).reduce());
      s0.setPrice(price);
    }
  }

  auto feeAccountWidget = d->haveWidget<KMyMoneyAccountCombo>("feesCombo");
  auto feeAmountWidget = d->haveWidget<AmountEdit>("feesAmountEdit");
  if (feeAmountWidget && feeAccountWidget) {
    if (!createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
      return false;
  }

  auto interestAccountWidget = d->haveWidget<KMyMoneyAccountCombo>("interestCombo");
  auto interestAmountWidget = d->haveWidget<AmountEdit>("interestAmountEdit");
  if (!interestAccountWidget || !interestAmountWidget ||
      !createCategorySplits(t, interestAccountWidget, interestAmountWidget, MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  if (interestSplits.count() != 1) {
    qDebug("more or less than one interest split in Reinvest::createTransaction. Not created.");
    return false;
  }
  assetAccountSplit.setAccountId(QString());

  MyMoneySplit& s1 = interestSplits[0];

  MyMoneyMoney total = sumSplits(s0, feeSplits, QList<MyMoneySplit>());

  s1.setValue(-total);

  /// @todo port to new model code
#if 0
  if (!d->m_parent->setupPrice(t, s1))
    return false;
#endif

  return true;
}

Add::Add(InvestTransactionEditor* editor) :
  Activity(editor)
{
}

Add::~Add()
{
}

eMyMoney::Split::InvestmentTransactionType Add::type() const
{
  return eMyMoney::Split::InvestmentTransactionType::AddShares;
}

void Add::showWidgets() const
{
  Q_D(const Activity);
  static const QStringList activityWidgets = {
    "sharesLabel", "sharesAmountEdit",
  };

  setupWidgets(activityWidgets);

  if (auto shareEdit = d->haveWidget<AmountEdit>("sharesAmountEdit")) {
    shareEdit->show();
    /// @todo port to new model code
#if 0
    shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));
#endif
  }
}

bool Add::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveShares();
  return rc;
}

bool Add::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(t)
  Q_UNUSED(assetAccountSplit)
  Q_UNUSED(m_feeSplits)
  Q_UNUSED(m_interestSplits)
  Q_UNUSED(security)
  Q_UNUSED(currency)

  Q_D(Activity);
  QString reason;
  if (!isComplete(reason))
    return false;

  auto sharesEdit = d->haveWidget<AmountEdit>("sharesAmountEdit");

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
  if (sharesEdit)
    s0.setShares(sharesEdit->value().abs());
  s0.setValue(MyMoneyMoney());
  s0.setPrice(MyMoneyMoney());

  assetAccountSplit.setValue(MyMoneyMoney());//  Clear any leftover value from previous Dividend.

  feeSplits.clear();
  interestSplits.clear();

  return true;
}

Remove::Remove(InvestTransactionEditor* editor) :
  Activity(editor)
{
}

Remove::~Remove()
{
}

eMyMoney::Split::InvestmentTransactionType Remove::type() const
{
  return eMyMoney::Split::InvestmentTransactionType::RemoveShares;
}

void Remove::showWidgets() const
{
  Q_D(const Activity);
  static const QStringList activityWidgets = {
    "sharesLabel", "sharesAmountEdit",
  };

  setupWidgets(activityWidgets);

  if (auto shareEdit = d->haveWidget<AmountEdit>("sharesAmountEdit")) {
    shareEdit->show();
    /// @todo port to new model code
#if 0
    shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));
#endif
  }
}

bool Remove::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveShares();
  return rc;
}

bool Remove::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(t)
  Q_UNUSED(assetAccountSplit)
  Q_UNUSED(m_feeSplits)
  Q_UNUSED(m_interestSplits)
  Q_UNUSED(security)
  Q_UNUSED(currency)

  Q_D(Activity);
  QString reason;
  if (!isComplete(reason))
    return false;

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
  if (auto sharesEdit = d->haveWidget<AmountEdit>("sharesAmountEdit"))
    s0.setShares(-(sharesEdit->value().abs()));
  s0.setValue(MyMoneyMoney());
  s0.setPrice(MyMoneyMoney());

  assetAccountSplit.setValue(MyMoneyMoney());//  Clear any leftover value from previous Dividend.

  feeSplits.clear();
  interestSplits.clear();

  return true;
}

MyMoneyMoney Remove::sharesFactor() const
{
  return MyMoneyMoney::MINUS_ONE;
}



Invest::Split::Split(InvestTransactionEditor* editor) :
  Activity(editor)
{
}

Invest::Split::~Split()
{
}

eMyMoney::Split::InvestmentTransactionType Invest::Split::type() const
{
  return eMyMoney::Split::InvestmentTransactionType::SplitShares;
}

void Invest::Split::showWidgets() const
{
  Q_D(const Activity);
  static const QStringList activityWidgets = {
    "sharesLabel", "sharesAmountEdit",
    "priceLabel", "priceAmountEdit"
  };

  setupWidgets(activityWidgets);

  // TODO do we need a special split ratio widget?
  // TODO maybe yes, currently the precision is the one of the fraction and might differ from it
  if (auto shareEdit = d->haveWidget<AmountEdit>("sharesAmountEdit")) {
    shareEdit->show();
    shareEdit->setPrecision(-1);
  }
}

QString Invest::Split::priceLabelText() const
{
  return i18n("Ratio 1/");
}

bool Invest::Split::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveShares();
  return rc;
}

bool Invest::Split::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(t)
  Q_UNUSED(assetAccountSplit)
  Q_UNUSED(m_feeSplits)
  Q_UNUSED(m_interestSplits)
  Q_UNUSED(security)
  Q_UNUSED(currency)

  Q_D(Activity);
  auto sharesEdit = d->haveWidget<AmountEdit>("sharesAmountEdit");

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::SplitShares);
  if (sharesEdit)
    s0.setShares(sharesEdit->value().abs());
  s0.setValue(MyMoneyMoney());
  s0.setPrice(MyMoneyMoney());

  feeSplits.clear();
  interestSplits.clear();

  return true;
}

IntInc::IntInc(InvestTransactionEditor* editor) :
  Activity(editor)
{
}

IntInc::~IntInc()
{
}

eMyMoney::Split::InvestmentTransactionType IntInc::type() const
{
  return eMyMoney::Split::InvestmentTransactionType::InterestIncome;
}

void IntInc::showWidgets() const
{
  static const QStringList activityWidgets = {
    "accountLabel", "accountCombo",
    "feesLabel", "feesCombo", "feesSplitEditorButton", "feesAmountLabel", "feesAmountEdit",
    "interestLabel", "interestCombo", "interestSplitEditorButton", "interestAmountLabel", "interestAmountEdit",
    "totalLabel", "totalAmountEdit",
  };

  setupWidgets(activityWidgets);
}

bool IntInc::isComplete(QString& reason) const
{
  Q_UNUSED(reason)

  auto rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveCategoryAndAmount("interestCombo", QString(), false);
  rc &= haveInterest(false);
  return rc;
}

bool IntInc::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(security)
  Q_UNUSED(currency)

  Q_D(Activity);
  QString reason;
  if (!isComplete(reason))
    return false;

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::InterestIncome);

  // for dividends, we only use the stock split as a marker
  MyMoneyMoney shares;
  s0.setShares(shares);
  s0.setValue(shares);
  s0.setPrice(MyMoneyMoney::ONE);

  auto feeAccountWidget = d->haveWidget<KMyMoneyAccountCombo>("feesCombo");
  auto feeAmountWidget = d->haveWidget<AmountEdit>("feesAmountEdit");
  if (!feeAccountWidget || !feeAmountWidget ||
      !createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  auto interestAccountWidget = d->haveWidget<KMyMoneyAccountCombo>("interestCombo");
  auto interestAmountWidget = d->haveWidget<AmountEdit>("interestAmountEdit");
  if (!interestAccountWidget || !interestAmountWidget ||
      !createCategorySplits(t, interestAccountWidget, interestAmountWidget, MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
  assetAccountSplit.setValue(-total);

  /// @todo port to new model code
#if 0
  if (!d->m_parent->setupPrice(t, assetAccountSplit))
    return false;
#endif

  return true;
}
