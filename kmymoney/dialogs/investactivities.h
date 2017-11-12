/***************************************************************************
                             investactivities.h
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

#ifndef INVESTACTIVITIES_H
#define INVESTACTIVITIES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QWidget;
class QStringList;

class KMyMoneyEdit;
class KMyMoneyCategory;

class MyMoneyMoney;
class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneySecurity;

class InvestTransactionEditor;

namespace eMyMoney { namespace Split { enum class InvestmentTransactionType; } }
namespace eDialogs { enum class PriceMode; }

template <typename T> class QList;
template <class Key, class Value> class QMap;

namespace Invest
{

class ActivityPrivate;
class Activity
{
  Q_DISABLE_COPY(Activity)

public:
  virtual eMyMoney::Split::InvestmentTransactionType type() const = 0;
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
  virtual ~Activity();

  bool &memoChanged();
  QString& memoText();

protected:
  explicit Activity(InvestTransactionEditor* editor);
  QWidget* haveWidget(const QString& name) const;
  bool haveAssetAccount() const;
  bool haveFees(bool optional = false) const;
  bool haveInterest(bool optional = false) const;
  bool haveShares() const;
  bool havePrice() const;
  bool isMultiSelection() const;
  QString priceLabel() const;
  bool createCategorySplits(const MyMoneyTransaction& t, KMyMoneyCategory* cat, KMyMoneyEdit* amount, MyMoneyMoney factor, QList<MyMoneySplit>&splits, const QList<MyMoneySplit>& osplits) const;
  void createAssetAccountSplit(MyMoneySplit& split, const MyMoneySplit& stockSplit) const;
  MyMoneyMoney sumSplits(const MyMoneySplit& s0, const QList<MyMoneySplit>& feeSplits, const QList<MyMoneySplit>& interestSplits) const;
  bool haveCategoryAndAmount(const QString& category, const QString& amount, bool optional) const;
  void setLabelText(const QString& idx, const QString& txt) const;
  void setWidgetVisibility(const QStringList& widgetIds, bool visible) const;
  eDialogs::PriceMode priceMode() const;

protected:
  ActivityPrivate* d_ptr;
  Q_DECLARE_PRIVATE(Activity)
};

class Buy : public Activity
{
public:
  explicit Buy(InvestTransactionEditor* editor);
  ~Buy() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class Sell : public Activity
{
public:
  explicit Sell(InvestTransactionEditor* editor);
  ~Sell() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class Div : public Activity
{
public:
  explicit Div(InvestTransactionEditor* editor);
  ~Div() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class Reinvest : public Activity
{
public:
  explicit Reinvest(InvestTransactionEditor* editor);
  ~Reinvest() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class Add : public Activity
{
public:
  explicit Add(InvestTransactionEditor* editor);
  ~Add() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class Remove : public Activity
{
public:
  explicit Remove(InvestTransactionEditor* editor);
  ~Remove() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class Split : public Activity
{
public:
  explicit Split(InvestTransactionEditor* editor);
  ~Split() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class IntInc : public Activity
{
public:
  explicit IntInc(InvestTransactionEditor* editor);
  ~IntInc() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

} // namespace Invest

#endif // INVESTACTIVITIES_H

