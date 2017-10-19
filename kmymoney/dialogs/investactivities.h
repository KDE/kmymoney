/***************************************************************************
                             investactivities.h
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

#ifndef INVESTACTIVITIES_H
#define INVESTACTIVITIES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "investtransactioneditor.h"

class kMyMoneyEdit;

namespace Invest
{

class Activity
{
public:
  virtual MyMoneySplit::investTransactionTypeE type() const = 0;
  virtual void showWidgets() const = 0;
  virtual bool isComplete(QString& reason) const = 0;

  /**
    * Create a transaction @p t based on the split @p s0 and the data contained
    * in the widgets. In multiselection mode, @p assetAccountSplit, @p feeSplits, @p
    * interestSplits, @p security and @p currency are taken from the original
    * transaction and should be used as well.
    *
    * @return @p true if creation was successful, @p false otherwise
    */
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) = 0;

  virtual void preloadAssetAccount();
  virtual ~Activity() {}
  bool    m_memoChanged;
  QString m_memoText;

protected:
  Activity(InvestTransactionEditor* editor) : m_memoChanged(false) {
    m_parent = editor;
  }
  QWidget* haveWidget(const QString& name) const {
    return m_parent->haveWidget(name);
  }
  bool haveAssetAccount() const;
  bool haveFees(bool optional = false) const {
    return haveCategoryAndAmount("fee-account", "fee-amount", optional);
  }
  bool haveInterest(bool optional = false) const {
    return haveCategoryAndAmount("interest-account", "interest-amount", optional);
  }
  bool haveShares() const;
  bool havePrice() const;
  bool isMultiSelection() const {
    return m_parent->isMultiSelection();
  }
  QString priceLabel() const;
  bool createCategorySplits(const MyMoneyTransaction& t, KMyMoneyCategory* cat, kMyMoneyEdit* amount, MyMoneyMoney factor, QList<MyMoneySplit>&splits, const QList<MyMoneySplit>& osplits) const;
  void createAssetAccountSplit(MyMoneySplit& split, const MyMoneySplit& stockSplit) const;
  MyMoneyMoney sumSplits(const MyMoneySplit& s0, const QList<MyMoneySplit>& feeSplits, const QList<MyMoneySplit>& interestSplits) const;
  bool haveCategoryAndAmount(const QString& category, const QString& amount, bool optional) const;
  void setLabelText(const QString& idx, const QString& txt) const;
  void setWidgetVisibility(const QStringList& widgetIds, bool visible) const;
  InvestTransactionEditor::priceModeE priceMode() const {
    return m_parent->priceMode();
  }

protected:
  InvestTransactionEditor*      m_parent;
  QMap<QString, MyMoneyMoney>   m_priceInfo;
};

class Buy : public Activity
{
public:
  Buy(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual ~Buy() {}
  virtual MyMoneySplit::investTransactionTypeE type() const {
    return MyMoneySplit::BuyShares;
  }
  virtual void showWidgets() const;
  virtual bool isComplete(QString& reason) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Sell : public Activity
{
public:
  Sell(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual ~Sell() {}
  virtual MyMoneySplit::investTransactionTypeE type() const {
    return MyMoneySplit::SellShares;
  }
  virtual void showWidgets() const;
  virtual bool isComplete(QString& reason) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Div : public Activity
{
public:
  Div(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual ~Div() {}
  virtual MyMoneySplit::investTransactionTypeE type() const {
    return MyMoneySplit::Dividend;
  }
  virtual void showWidgets() const;
  virtual bool isComplete(QString& reason) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Reinvest : public Activity
{
public:
  Reinvest(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual ~Reinvest() {}
  virtual MyMoneySplit::investTransactionTypeE type() const {
    return MyMoneySplit::ReinvestDividend;
  }
  virtual void showWidgets() const;
  virtual bool isComplete(QString& reason) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Add : public Activity
{
public:
  Add(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual ~Add() {}
  virtual MyMoneySplit::investTransactionTypeE type() const {
    return MyMoneySplit::AddShares;
  }
  virtual void showWidgets() const;
  virtual bool isComplete(QString& reason) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Remove : public Activity
{
public:
  Remove(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual ~Remove() {}
  virtual MyMoneySplit::investTransactionTypeE type() const {
    return MyMoneySplit::RemoveShares;
  }
  virtual void showWidgets() const;
  virtual bool isComplete(QString& reason) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Split : public Activity
{
public:
  Split(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual ~Split() {}
  virtual MyMoneySplit::investTransactionTypeE type() const {
    return MyMoneySplit::SplitShares;
  }
  virtual void showWidgets() const;
  virtual bool isComplete(QString& reason) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class IntInc : public Activity
{
public:
  IntInc(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual ~IntInc() {}
  virtual MyMoneySplit::investTransactionTypeE type() const {
    return MyMoneySplit::InterestIncome;
  }
  virtual void showWidgets() const;
  virtual bool isComplete(QString& reason) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

} // namespace Invest



#endif // INVESTACTIVITIES_H

