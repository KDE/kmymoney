/***************************************************************************
                             investactivities.cpp
                             ----------
    begin                : Fri Dec 15 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "kmymoneycategory.h"
#include "kmymoneyedit.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneycompletion.h"
#include <kmymoneysettings.h>
#include "mymoneyfile.h"

using namespace Invest;
using namespace KMyMoneyRegister;

bool Activity::isComplete(QString& reason) const
{
  Q_UNUSED(reason)

  bool rc = false;
  KMyMoneySecurity* security = dynamic_cast<KMyMoneySecurity*>(haveWidget("security"));
  if (!security->currentText().isEmpty()) {
    rc = (security->selector()->contains(security->currentText()) || (isMultiSelection() && m_memoChanged));
  }
  return rc;
}

bool Activity::haveAssetAccount() const
{
  KMyMoneyCategory* cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));

  bool rc = true;
  if (!isMultiSelection())
    rc = !cat->currentText().isEmpty();

  if (rc && !cat->currentText().isEmpty()) {
    rc = cat->selector()->contains(cat->currentText());
  }
  return rc;
}

bool Activity::haveCategoryAndAmount(const QString& category, const QString& amount, bool optional) const
{
  KMyMoneyCategory* cat = dynamic_cast<KMyMoneyCategory*>(haveWidget(category));

  bool rc = true;

  if (!cat->currentText().isEmpty()) {
    rc = cat->selector()->contains(cat->currentText()) || cat->isSplitTransaction();
    if (rc && !amount.isEmpty() && !isMultiSelection()) {
      if (cat->isSplitTransaction()) {
        QList<MyMoneySplit>::const_iterator split;
        QList<MyMoneySplit>::const_iterator splitEnd;

        if (category == "fee-account") {
          split = m_parent->feeSplits().cbegin();
          splitEnd = m_parent->feeSplits().cend();
        } else if (category == "interest-account") {
          split = m_parent->interestSplits().cbegin();
          splitEnd = m_parent->interestSplits().cend();
        }

        for (; split != splitEnd; ++split) {
          if ((*split).value().isZero())
            rc = false;
        }
      } else {
        MyMoneyMoney value = dynamic_cast<kMyMoneyEdit*>(haveWidget(amount))->value();
        rc = !value.isZero();
      }
    }
  } else if (!isMultiSelection() && !optional) {
    rc = false;
  }
  return rc;
}

bool Activity::haveShares() const
{
  kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  if (isMultiSelection() && amount->value().isZero())
    return true;

  return !amount->value().isZero();
}

bool Activity::havePrice() const
{
  kMyMoneyEdit* amount = dynamic_cast<kMyMoneyEdit*>(haveWidget("price"));
  if (isMultiSelection() && amount->value().isZero())
    return true;

  return !amount->value().isZero();
}

bool Activity::createCategorySplits(const MyMoneyTransaction& t, KMyMoneyCategory* cat, kMyMoneyEdit* amount, MyMoneyMoney factor, QList<MyMoneySplit>&splits, const QList<MyMoneySplit>& osplits) const
{
  bool rc = true;
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
          rc = m_parent->setupPrice(t, s1);
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
  KMyMoneyCategory* cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
  if (!isMultiSelection() || (isMultiSelection() && !cat->currentText().isEmpty())) {
    QString categoryId;
    categoryId = cat->selectedItem();
    split.setAccountId(categoryId);
  }
  split.setMemo(stockSplit.memo());
}

MyMoneyMoney Activity::sumSplits(const MyMoneySplit& s0, const QList<MyMoneySplit>& feeSplits, const QList<MyMoneySplit>& interestSplits) const
{
  MyMoneyMoney total;
  total = s0.value();
  QList<MyMoneySplit>::const_iterator it_s;
  for (it_s = feeSplits.begin(); it_s != feeSplits.end(); ++it_s) {
    total += (*it_s).value();
  }
  for (it_s = interestSplits.begin(); it_s != interestSplits.end(); ++it_s) {
    total += (*it_s).value();
  }
  return total;
}

void Activity::setLabelText(const QString& idx, const QString& txt) const
{
  QLabel* w = dynamic_cast<QLabel*>(haveWidget(idx));
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
  KMyMoneyCategory* cat;
  cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
  if (cat->isVisible()) {
    if (cat->currentText().isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->accountByName(i18n("%1 (Brokerage)", m_parent->account().name()));
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
    QWidget* w = haveWidget(*it_w);
    if (w) {
      if (visible) {
        w->show();
      } else {
        w->hide();
      }
    }
  }
}

QString Activity::priceLabel() const
{
  QString label;
  if (priceMode() == InvestTransactionEditor::Price) {
    label = i18n("Price");
  } else if (priceMode() == InvestTransactionEditor::PricePerShare) {
    label = i18n("Price/share");
  } else if (priceMode() == InvestTransactionEditor::PricePerTransaction) {
    label = i18n("Transaction amount");
  }
  return label;
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
  bool rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveFees(true);
  rc &= haveShares();
  rc &= havePrice();

  return rc;
}

bool Buy::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(m_interestSplits);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  QString reason;
  if (!isComplete(reason))
    return false;

  kMyMoneyEdit* sharesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  kMyMoneyEdit* priceEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("price"));

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
    if (priceMode() == InvestTransactionEditor::PricePerTransaction) {
      s0.setValue(price.reduce());
      if (!s0.shares().isZero())
        s0.setPrice((price / s0.shares()).reduce());
    } else {
      s0.setValue((shares * price).reduce());
      s0.setPrice(price);
    }
  }

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account")), dynamic_cast<kMyMoneyEdit*>(haveWidget("fee-amount")), MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, QList<MyMoneySplit>());

  //  Clear any leftover value from previous Dividend.
  interestSplits.clear();

  assetAccountSplit.setValue(-total);

  if (!m_parent->setupPrice(t, assetAccountSplit))
    return false;

  return true;
}

void Sell::showWidgets() const
{
  static const QStringList visibleWidgetIds = QStringList() << "asset-account" << "interest-amount" << "shares" << "price" << "total" << "interest-account" << "fee-account";
  setWidgetVisibility(visibleWidgetIds, true);

  kMyMoneyEdit* shareEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->setPrecision(MyMoneyMoney::denomToPrec(m_parent->security().smallestAccountFraction()));

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
  bool rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveFees(true);
  rc &= haveInterest(true);
  rc &= haveShares();
  rc &= havePrice();
  return rc;
}

bool Sell::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(m_interestSplits);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  QString reason;
  if (!isComplete(reason))
    return false;

  kMyMoneyEdit* sharesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  kMyMoneyEdit* priceEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("price"));

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
    if (priceMode() == InvestTransactionEditor::PricePerTransaction) {
      price = -price;
      s0.setValue(price.reduce());
      if (!s0.shares().isZero())
        s0.setPrice((price / s0.shares()).reduce());
    } else {
      s0.setValue((shares * price).reduce());
      s0.setPrice(price);
    }
  }

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account")), dynamic_cast<kMyMoneyEdit*>(haveWidget("fee-amount")), MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account")), dynamic_cast<kMyMoneyEdit*>(haveWidget("interest-amount")), MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
  assetAccountSplit.setValue(-total);

  if (!m_parent->setupPrice(t, assetAccountSplit))
    return false;

  return true;
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

  bool rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveCategoryAndAmount("interest-account", QString(), false);
  rc &= haveInterest(false);
  return rc;
}

bool Div::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
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

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account")), dynamic_cast<kMyMoneyEdit*>(haveWidget("fee-amount")), MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account")), dynamic_cast<kMyMoneyEdit*>(haveWidget("interest-amount")), MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
  assetAccountSplit.setValue(-total);

  if (!m_parent->setupPrice(t, assetAccountSplit))
    return false;

  return true;
}

void Reinvest::showWidgets() const
{
  static const QStringList visibleWidgetIds = QStringList() << "price" << "interest-account";
  setWidgetVisibility(visibleWidgetIds, true);

  kMyMoneyEdit* shareEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->show();
  shareEdit->setPrecision(MyMoneyMoney::denomToPrec(m_parent->security().smallestAccountFraction()));

  kMyMoneyEdit* intAmount = dynamic_cast<kMyMoneyEdit*>(haveWidget("interest-amount"));
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
  bool rc = Activity::isComplete(reason);
  rc &= haveCategoryAndAmount("interest-account", QString(), false);
  rc &= haveShares();
  rc &= havePrice();
  return rc;
}

bool Reinvest::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(assetAccountSplit);
  Q_UNUSED(security);
  Q_UNUSED(currency);
  Q_UNUSED(m_feeSplits)

  QString reason;
  if (!isComplete(reason))
    return false;

  kMyMoneyEdit* sharesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  kMyMoneyEdit* priceEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("price"));

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
    if (priceMode() == InvestTransactionEditor::PricePerTransaction) {
      s0.setValue(price.reduce());
      if (!s0.shares().isZero())
        s0.setPrice((price / s0.shares()).reduce());
    } else {
      s0.setValue((shares * price).reduce());
      s0.setPrice(price);
    }
  }

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account")), dynamic_cast<kMyMoneyEdit*>(haveWidget("interest-amount")), MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  if (interestSplits.count() != 1) {
    qDebug("more or less than one interest split in Reinvest::createTransaction. Not created.");
    return false;
  }

  MyMoneySplit& s1 = interestSplits[0];

  MyMoneyMoney total = sumSplits(s0, feeSplits, QList<MyMoneySplit>());

  s1.setValue(-total);

  if (!m_parent->setupPrice(t, s1))
    return false;

  return true;
}

void Add::showWidgets() const
{
  kMyMoneyEdit* shareEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->show();
  shareEdit->setPrecision(MyMoneyMoney::denomToPrec(m_parent->security().smallestAccountFraction()));

  setLabelText("shares-label", i18n("Shares"));
}

bool Add::isComplete(QString& reason) const
{
  bool rc = Activity::isComplete(reason);
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

  kMyMoneyEdit* sharesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
  s0.setShares(sharesEdit->value().abs());
  s0.setValue(MyMoneyMoney());
  s0.setPrice(MyMoneyMoney());

  assetAccountSplit.setValue(MyMoneyMoney());//  Clear any leftover value from previous Dividend.

  feeSplits.clear();
  interestSplits.clear();

  return true;
}

void Remove::showWidgets() const
{
  kMyMoneyEdit* shareEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->show();
  shareEdit->setPrecision(MyMoneyMoney::denomToPrec(m_parent->security().smallestAccountFraction()));
  setLabelText("shares-label", i18n("Shares"));
}

bool Remove::isComplete(QString& reason) const
{
  bool rc = Activity::isComplete(reason);
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

  kMyMoneyEdit* sharesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));

  s0.setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
  s0.setShares(-(sharesEdit->value().abs()));
  s0.setValue(MyMoneyMoney());
  s0.setPrice(MyMoneyMoney());

  assetAccountSplit.setValue(MyMoneyMoney());//  Clear any leftover value from previous Dividend.

  feeSplits.clear();
  interestSplits.clear();

  return true;
}

void Split::showWidgets() const
{
  // TODO do we need a special split ratio widget?
  // TODO maybe yes, currently the precision is the one of the fraction and might differ from it
  kMyMoneyEdit* shareEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));
  shareEdit->show();
  shareEdit->setPrecision(-1);
  setLabelText("shares-label", i18n("Ratio 1/"));
}

bool Split::isComplete(QString& reason) const
{
  bool rc = Activity::isComplete(reason);
  rc &= haveShares();
  return rc;
}

bool Split::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
  Q_UNUSED(t);
  Q_UNUSED(assetAccountSplit);
  Q_UNUSED(m_feeSplits);
  Q_UNUSED(m_interestSplits);
  Q_UNUSED(security);
  Q_UNUSED(currency);

  kMyMoneyEdit* sharesEdit = dynamic_cast<kMyMoneyEdit*>(haveWidget("shares"));

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

  bool rc = Activity::isComplete(reason);
  rc &= haveAssetAccount();
  rc &= haveCategoryAndAmount("interest-account", QString(), false);
  rc &= haveInterest(false);
  return rc;
}

bool IntInc::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
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

  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account")), dynamic_cast<kMyMoneyEdit*>(haveWidget("fee-amount")), MyMoneyMoney::ONE, feeSplits, m_feeSplits))
    return false;
  if (!createCategorySplits(t, dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account")), dynamic_cast<kMyMoneyEdit*>(haveWidget("interest-amount")), MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
    return false;

  createAssetAccountSplit(assetAccountSplit, s0);

  MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
  assetAccountSplit.setValue(-total);

  if (!m_parent->setupPrice(t, assetAccountSplit))
    return false;

  return true;
}
