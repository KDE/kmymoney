/***************************************************************************
                             investactivities.cpp
                             ----------
    begin                : Fri Dec 15 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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
#include "kmymoneycategory.h"
#include "kmymoneyedit.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneycompletion.h"
#include <kmymoneysettings.h>
#include "mymoneyfile.h"
#include "mymoneysplit.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "dialogenums.h"
#include "mymoneyenums.h"

using namespace Invest;
using namespace KMyMoneyRegister;

class Invest::ActivityPrivate
{
  Q_DISABLE_COPY(ActivityPrivate)

public:
  ActivityPrivate()
  {
  }

  InvestTransactionEditor     *m_parent;
  QMap<QString, MyMoneyMoney>  m_priceInfo;
  bool                         m_memoChanged;
  QString                      m_memoText;
};


Activity::Activity(InvestTransactionEditor* editor) :
   d_ptr(new ActivityPrivate)
{
  Q_D(Activity);
  d->m_memoChanged = false;
  d->m_parent = editor;
}

Activity::~Activity()
{
  Q_D(Activity);
  delete d;
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
  Q_D(const Activity);
  Q_UNUSED(reason)

  auto rc = false;
  auto security = dynamic_cast<KMyMoneySecurity*>(haveWidget("security"));
  if (!security->currentText().isEmpty()) {
    rc = (security->selector()->contains(security->currentText()) || (isMultiSelection() && d->m_memoChanged));
  }
  return rc;
}

QWidget* Activity::haveWidget(const QString& name) const
{
  Q_D(const Activity);
  return d->m_parent->haveWidget(name);
}

bool Activity::haveAssetAccount() const
{
  auto cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));

  auto rc = true;
  if (!isMultiSelection())
    rc = !cat->currentText().isEmpty();

  if (rc && !cat->currentText().isEmpty()) {
    rc = cat->selector()->contains(cat->currentText());
  }
  return rc;
}

bool Activity::haveCategoryAndAmount(const QString& category, const QString& amount, bool optional) const
{
  Q_D(const Activity);
  auto cat = dynamic_cast<KMyMoneyCategory*>(haveWidget(category));

  auto rc = true;

  if (!cat->currentText().isEmpty()) {
    rc = cat->selector()->contains(cat->currentText()) || cat->isSplitTransaction();
    if (rc && !amount.isEmpty() && !isMultiSelection()) {
      if (cat->isSplitTransaction()) {
        QList<MyMoneySplit>::const_iterator split;
        QList<MyMoneySplit>::const_iterator splitEnd;

        if (category == "fee-account") {
          split = d->m_parent->feeSplits().cbegin();
          splitEnd = d->m_parent->feeSplits().cend();
        } else if (category == "interest-account") {
          split = d->m_parent->interestSplits().cbegin();
          splitEnd = d->m_parent->interestSplits().cend();
        }

        for (; split != splitEnd; ++split) {
          if ((*split).value().isZero())
            rc = false;
        }
      } else {
        MyMoneyMoney value = dynamic_cast<KMyMoneyEdit*>(haveWidget(amount))->value();
        rc = !value.isZero();
      }
    }
  } else if (!isMultiSelection() && !optional) {
    rc = false;
  }
  return rc;
}

bool Activity::haveFees(bool optional) const
{
  return haveCategoryAndAmount("fee-account", "fee-amount", optional);
}

bool Activity::haveInterest(bool optional) const
{
  return haveCategoryAndAmount("interest-account", "interest-amount", optional);
}

bool Activity::haveShares() const
{
  KMyMoneyEdit* amount = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));
  if (isMultiSelection() && amount->value().isZero())
    return true;

  return !amount->value().isZero();
}

bool Activity::havePrice() const
{
  KMyMoneyEdit* amount = dynamic_cast<KMyMoneyEdit*>(haveWidget("price"));
  if (isMultiSelection() && amount->value().isZero())
    return true;

  return !amount->value().isZero();
}

bool Activity::isMultiSelection() const
{
  Q_D(const Activity);
  return d->m_parent->isMultiSelection();
}

bool Activity::createCategorySplits(const MyMoneyTransaction& t, KMyMoneyCategory* cat, KMyMoneyEdit* amount, MyMoneyMoney factor, QList<MyMoneySplit>&splits, const QList<MyMoneySplit>& osplits) const
{
  Q_D(const Activity);
  auto rc = true;
  if (!isMultiSelection() || (isMultiSelection() && !cat->currentText().isEmpty())) {
    if (!cat->isSplitTransaction()) {
      splits.clear();
      MyMoneySplit s1;
      QString categoryId;
      categoryId = cat->selectedItem();
      if (!categoryId.isEmpty()) {
        s1.setAccountId(categoryId);
        s1.setValue(amount->value() * factor);
        if (!s1.value().isZero()) {
          rc = d->m_parent->setupPrice(t, s1);
        }
        splits.append(s1);
      }
    } else {
      splits = osplits;
    }
  }
  return rc;
}

void Activity::createAssetAccountSplit(MyMoneySplit& split, const MyMoneySplit& stockSplit) const
{
  auto cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
  if (!isMultiSelection() || (isMultiSelection() && !cat->currentText().isEmpty())) {
    QString categoryId;
    categoryId = cat->selectedItem();
    split.setAccountId(categoryId);
  }
  split.setMemo(stockSplit.memo());
}

MyMoneyMoney Activity::sumSplits(const MyMoneySplit& s0, const QList<MyMoneySplit>& feeSplits, const QList<MyMoneySplit>& interestSplits) const
{
  auto total = s0.value();
  foreach (const auto feeSplit, feeSplits)
    total += feeSplit.value();

  foreach (const auto interestSplit, interestSplits)
    total += interestSplit.value();

  return total;
}

void Activity::setLabelText(const QString& idx, const QString& txt) const
{
  auto w = dynamic_cast<QLabel*>(haveWidget(idx));
  if (w) {
    w->setText(txt);
  } else {
    if (KMyMoneySettings::transactionForm()) {
      // labels are only used in the transaction form
      qDebug("Unknown QLabel named '%s'", qPrintable(idx));
    }
  }
}

void Activity::preloadAssetAccount()
{
  Q_D(Activity);
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
  if (cat->isVisible()) {
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
  }
}

void Activity::setWidgetVisibility(const QStringList& widgetIds, bool visible) const
{
  for (QStringList::const_iterator it_w = widgetIds.constBegin(); it_w != widgetIds.constEnd(); ++it_w) {
    auto w = haveWidget(*it_w);
    if (w) {
      if (visible) {
        w->show();
      } else {
        w->hide();
      }
    }
  }
}

eDialogs::PriceMode Activity::priceMode() const
{
  Q_D(const Activity);
  return d->m_parent->priceMode();
}

QString Activity::priceLabel() const
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
  static const QStringList visibleWidgetIds = QStringList() << "asset-account" << "shares" << "price" << "total" << "interest-account" << "fee-account";
  setWidgetVisibility(visibleWidgetIds, true);

  setLabelText("interest-amount-label", i18n("Interest"));
  setLabelText("interest-label", i18n("Interest"));
  setLabelText("fee-label", i18n("Fees"));
  setLabelText("asset-label", i18n("Account"));
  setLabelText("shares-label", i18n("Shares"));
  if (dynamic_cast<QLabel*>(haveWidget("price-label")))
    setLabelText("price-label", priceLabel());
  setLabelText("total-label", i18nc("Total value", "Total"));
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
  Q_D(Activity);
  Q_UNUSED(m_interestSplits);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  QString reason;
  if (!isComplete(reason))
    return false;

  auto sharesEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));
  auto priceEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("price"));

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::BuyShares);

  MyMoneyMoney shares = s0.shares();
  MyMoneyMoney price;
  if (!s0.shares().isZero())
    price = (s0.value() / s0.shares()).reduce();

  if (!isMultiSelection() || (isMultiSelection() && !sharesEdit->value().isZero())) {
    shares = sharesEdit->value().abs();
    s0.setShares(shares);
    s0.setValue((shares * price).reduce());
    s0.setPrice(price);
  }
  if (!isMultiSelection() || (isMultiSelection() && !priceEdit->value().isZero())) {
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

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account")), dynamic_cast<KMyMoneyEdit*>(haveWidget("fee-amount")), MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, QList<MyMoneySplit>());

  //  Clear any leftover value from previous Dividend.
  interestSplits.clear();

  assetAccountSplit.setValue(-total);

  if (!d->m_parent->setupPrice(t, assetAccountSplit))
    return false;

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
  static const QStringList visibleWidgetIds = QStringList() << "asset-account" << "interest-amount" << "shares" << "price" << "total" << "interest-account" << "fee-account";
  setWidgetVisibility(visibleWidgetIds, true);

  auto shareEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));

  setLabelText("interest-amount-label", i18n("Interest"));
  setLabelText("interest-label", i18n("Interest"));
  setLabelText("fee-label", i18n("Fees"));
  setLabelText("asset-label", i18n("Account"));
  setLabelText("shares-label", i18n("Shares"));
  if (dynamic_cast<QLabel*>(haveWidget("price-label")))
    setLabelText("price-label", priceLabel());
  setLabelText("total-label", i18nc("Total value", "Total"));
}

bool Sell::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveFees(true);
  rc &= haveInterest(true);
  rc &= haveShares();
  rc &= havePrice();
  return rc;
}

bool Sell::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_D(Activity);
  Q_UNUSED(m_interestSplits);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  QString reason;
  if (!isComplete(reason))
    return false;

  auto sharesEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));
  auto priceEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("price"));

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::BuyShares);

  MyMoneyMoney shares = s0.shares();
  MyMoneyMoney price;
  if (!s0.shares().isZero())
    price = (s0.value() / s0.shares()).reduce();

  if (!isMultiSelection() || (isMultiSelection() && !sharesEdit->value().isZero())) {
    shares = -sharesEdit->value().abs();
    s0.setShares(shares);
    s0.setValue((shares * price).reduce());
    s0.setPrice(price);
  }
  if (!isMultiSelection() || (isMultiSelection() && !priceEdit->value().isZero())) {
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

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account")), dynamic_cast<KMyMoneyEdit*>(haveWidget("fee-amount")), MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account")), dynamic_cast<KMyMoneyEdit*>(haveWidget("interest-amount")), MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
  assetAccountSplit.setValue(-total);

  if (!d->m_parent->setupPrice(t, assetAccountSplit))
    return false;

  return true;
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
  static const QStringList visibleWidgetIds = QStringList() << "asset-account" << "interest-amount" << "total" << "interest-account" << "fee-account";
  setWidgetVisibility(visibleWidgetIds, true);
  static const QStringList hiddenWidgetIds = QStringList() << "shares" << "price";
  setWidgetVisibility(hiddenWidgetIds, false);

  setLabelText("interest-amount-label", i18n("Interest"));
  setLabelText("interest-label", i18n("Interest"));
  setLabelText("fee-label", i18n("Fees"));
  setLabelText("asset-label", i18n("Account"));
  setLabelText("total-label", i18nc("Total value", "Total"));
}

bool Div::isComplete(QString& reason) const
{
  Q_UNUSED(reason)

  auto rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveCategoryAndAmount("interest-account", QString(), false);
  rc &= haveInterest(false);
  return rc;
}

bool Div::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_D(Activity);
  Q_UNUSED(m_feeSplits);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  QString reason;
  if (!isComplete(reason))
    return false;

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::Dividend);

  // for dividends, we only use the stock split as a marker
  MyMoneyMoney shares;
  s0.setShares(shares);
  s0.setValue(shares);
  s0.setPrice(MyMoneyMoney::ONE);

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account")), dynamic_cast<KMyMoneyEdit*>(haveWidget("fee-amount")), MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account")), dynamic_cast<KMyMoneyEdit*>(haveWidget("interest-amount")), MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
  assetAccountSplit.setValue(-total);

  if (!d->m_parent->setupPrice(t, assetAccountSplit))
    return false;

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
  static const QStringList visibleWidgetIds = QStringList() << "price" << "interest-account";
  setWidgetVisibility(visibleWidgetIds, true);

  auto shareEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->show();
  shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));

  KMyMoneyEdit* intAmount = dynamic_cast<KMyMoneyEdit*>(haveWidget("interest-amount"));
  intAmount->hide();
  setLabelText("interest-amount-label", QString());
  intAmount->setValue(MyMoneyMoney());

  setLabelText("interest-label", i18n("Interest"));
  setLabelText("shares-label", i18n("Shares"));
  if (dynamic_cast<QLabel*>(haveWidget("price-label")))
    setLabelText("price-label", priceLabel());
  setLabelText("total-label", i18nc("Total value", "Total"));
}

bool Reinvest::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveCategoryAndAmount("interest-account", QString(), false);
  rc &= haveShares();
  rc &= havePrice();
  return rc;
}

bool Reinvest::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_D(Activity);
  Q_UNUSED(assetAccountSplit);
  Q_UNUSED(security);
  Q_UNUSED(currency);
  Q_UNUSED(m_feeSplits)

  QString reason;
  if (!isComplete(reason))
    return false;

  auto sharesEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));
  auto priceEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("price"));

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::ReinvestDividend);

  MyMoneyMoney shares = s0.shares();
  MyMoneyMoney price;
  if (!s0.shares().isZero())
    price = (s0.value() / s0.shares()).reduce();

  if (!isMultiSelection() || (isMultiSelection() && !sharesEdit->value().isZero())) {
    shares = sharesEdit->value().abs();
    s0.setShares(shares);
    s0.setValue((shares * price).reduce());
    s0.setPrice(price);
  }
  if (!isMultiSelection() || (isMultiSelection() && !priceEdit->value().isZero())) {
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

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account")), dynamic_cast<KMyMoneyEdit*>(haveWidget("interest-amount")), MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  if (interestSplits.count() != 1) {
    qDebug("more or less than one interest split in Reinvest::createTransaction. Not created.");
    return false;
  }

  MyMoneySplit& s1 = interestSplits[0];

  MyMoneyMoney total = sumSplits(s0, feeSplits, QList<MyMoneySplit>());

  s1.setValue(-total);

  if (!d->m_parent->setupPrice(t, s1))
    return false;

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
  auto shareEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->show();
  shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));

  setLabelText("shares-label", i18n("Shares"));
}

bool Add::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveShares();
  return rc;
}

bool Add::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(t);
  Q_UNUSED(assetAccountSplit);
  Q_UNUSED(m_feeSplits);
  Q_UNUSED(m_interestSplits);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  QString reason;
  if (!isComplete(reason))
    return false;

  auto sharesEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
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
  auto shareEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->show();
  shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));
  setLabelText("shares-label", i18n("Shares"));
}

bool Remove::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveShares();
  return rc;
}

bool Remove::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(t);
  Q_UNUSED(assetAccountSplit);
  Q_UNUSED(m_feeSplits);
  Q_UNUSED(m_interestSplits);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  QString reason;
  if (!isComplete(reason))
    return false;

  auto sharesEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
  s0.setShares(-(sharesEdit->value().abs()));
  s0.setValue(MyMoneyMoney());
  s0.setPrice(MyMoneyMoney());

  assetAccountSplit.setValue(MyMoneyMoney());//  Clear any leftover value from previous Dividend.

  feeSplits.clear();
  interestSplits.clear();

  return true;
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
  // TODO do we need a special split ratio widget?
  // TODO maybe yes, currently the precision is the one of the fraction and might differ from it
  auto shareEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->show();
  shareEdit->setPrecision(-1);
  setLabelText("shares-label", i18n("Ratio 1/"));
}

bool Invest::Split::isComplete(QString& reason) const
{
  auto rc = Activity::isComplete(reason);
  rc &= haveShares();
  return rc;
}

bool Invest::Split::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(t);
  Q_UNUSED(assetAccountSplit);
  Q_UNUSED(m_feeSplits);
  Q_UNUSED(m_interestSplits);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  auto sharesEdit = dynamic_cast<KMyMoneyEdit*>(haveWidget("shares"));

  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
  cat->parentWidget()->hide();
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
  cat->parentWidget()->hide();

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::SplitShares);
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
  static const QStringList visibleWidgetIds = QStringList() << "asset-account" << "interest-amount" << "total" << "interest-account" << "fee-account";
  setWidgetVisibility(visibleWidgetIds, true);
  static const QStringList hiddenWidgetIds = QStringList() << "shares" << "price" << "fee-amount";
  setWidgetVisibility(hiddenWidgetIds, false);

  setLabelText("interest-amount-label", i18n("Interest"));
  setLabelText("interest-label", i18n("Interest"));
  setLabelText("fee-label", i18n("Fees"));
  setLabelText("asset-label", i18n("Account"));
  setLabelText("total-label", i18nc("Total value", "Total"));
}

bool IntInc::isComplete(QString& reason) const
{
  Q_UNUSED(reason)

  auto rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveCategoryAndAmount("interest-account", QString(), false);
  rc &= haveInterest(false);
  return rc;
}

bool IntInc::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_D(Activity);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  QString reason;
  if (!isComplete(reason))
    return false;

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::InterestIncome);

  // for dividends, we only use the stock split as a marker
  MyMoneyMoney shares;
  s0.setShares(shares);
  s0.setValue(shares);
  s0.setPrice(MyMoneyMoney::ONE);

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account")), dynamic_cast<KMyMoneyEdit*>(haveWidget("fee-amount")), MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;
  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account")), dynamic_cast<KMyMoneyEdit*>(haveWidget("interest-amount")), MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
  assetAccountSplit.setValue(-total);

  if (!d->m_parent->setupPrice(t, assetAccountSplit))
    return false;

  return true;
}
