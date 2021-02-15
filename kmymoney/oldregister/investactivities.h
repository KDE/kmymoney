/*
    SPDX-FileCopyrightText: 2007-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OLDINVESTACTIVITIES_H
#define OLDINVESTACTIVITIES_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QWidget;
class QStringList;

class AmountEdit;
class KMyMoneyCategory;

class MyMoneyMoney;
class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneySecurity;

class OldInvestTransactionEditor;

namespace eMyMoney { namespace Split { enum class InvestmentTransactionType; } }
namespace eDialogs { enum class PriceMode; }

template <typename T> class QList;
template <class Key, class Value> class QMap;

namespace Invest
{

class OldActivityPrivate;
class KMM_OLDREGISTER_EXPORT OldActivity
{
  Q_DISABLE_COPY(OldActivity)

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
  virtual ~OldActivity();

  bool &memoChanged();
  QString& memoText();

protected:
  explicit OldActivity(OldInvestTransactionEditor* editor);
  bool haveAssetAccount() const;
  bool haveFees(bool optional = false) const;
  bool haveInterest(bool optional = false) const;
  bool haveShares() const;
  bool havePrice() const;
  bool isMultiSelection() const;
  QString priceLabel() const;
  bool createCategorySplits(const MyMoneyTransaction& t, KMyMoneyCategory* cat, AmountEdit* amount, MyMoneyMoney factor, QList<MyMoneySplit>&splits, const QList<MyMoneySplit>& osplits) const;
  void createAssetAccountSplit(MyMoneySplit& split, const MyMoneySplit& stockSplit) const;
  MyMoneyMoney sumSplits(const MyMoneySplit& s0, const QList<MyMoneySplit>& feeSplits, const QList<MyMoneySplit>& interestSplits) const;
  bool haveCategoryAndAmount(const QString& category, const QString& amount, bool optional) const;
  void setLabelText(const QString& idx, const QString& txt) const;
  void setWidgetVisibility(const QStringList& widgetIds, bool visible) const;
  eDialogs::PriceMode priceMode() const;

protected:
  OldActivityPrivate* d_ptr;
  Q_DECLARE_PRIVATE(OldActivity)
};

class KMM_OLDREGISTER_EXPORT Buy : public OldActivity
{
public:
  explicit Buy(OldInvestTransactionEditor* editor);
  ~Buy() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class KMM_OLDREGISTER_EXPORT Sell : public OldActivity
{
public:
  explicit Sell(OldInvestTransactionEditor* editor);
  ~Sell() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class KMM_OLDREGISTER_EXPORT Div : public OldActivity
{
public:
  explicit Div(OldInvestTransactionEditor* editor);
  ~Div() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class KMM_OLDREGISTER_EXPORT Reinvest : public OldActivity
{
public:
  explicit Reinvest(OldInvestTransactionEditor* editor);
  ~Reinvest() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class KMM_OLDREGISTER_EXPORT Add : public OldActivity
{
public:
  explicit Add(OldInvestTransactionEditor* editor);
  ~Add() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class KMM_OLDREGISTER_EXPORT Remove : public OldActivity
{
public:
  explicit Remove(OldInvestTransactionEditor* editor);
  ~Remove() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class KMM_OLDREGISTER_EXPORT Split : public OldActivity
{
public:
  explicit Split(OldInvestTransactionEditor* editor);
  ~Split() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

class KMM_OLDREGISTER_EXPORT IntInc : public OldActivity
{
public:
  explicit IntInc(OldInvestTransactionEditor* editor);
  ~IntInc() override;
  eMyMoney::Split::InvestmentTransactionType type() const override;
  void showWidgets() const override;
  bool isComplete(QString& reason) const override;
  bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) override;
};

} // namespace Invest

#endif // INVESTACTIVITIES_H

