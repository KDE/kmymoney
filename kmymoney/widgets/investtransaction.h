/***************************************************************************
                          investtransaction.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
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

#ifndef INVESTTRANSACTION_H
#define INVESTTRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QPalette>
#include <QList>
#include <QColor>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "transaction.h"

namespace eMyMoney { namespace Split { enum class InvestmentTransactionType; } }

namespace KMyMoneyRegister
{

  class InvestTransactionPrivate;
  class InvestTransaction : public Transaction
  {
  public:
    explicit InvestTransaction(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    InvestTransaction(const InvestTransaction & other);
    InvestTransaction(InvestTransaction && other);
    InvestTransaction & operator=(InvestTransaction other);
    friend void swap(InvestTransaction& first, InvestTransaction& second);
    ~InvestTransaction() override;

    virtual const QString sortSecurity() const override;
    virtual const char* className() override;

    bool formCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0) override;
    void registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0) override;

    int registerColWidth(int col, const QFontMetrics& cellFontMetrics) override;
    void setupForm(KMyMoneyTransactionForm::TransactionForm* form) override;

    /**
    * provide NOP here as the investment transaction form does not supply a tab
    */
    void loadTab(KMyMoneyTransactionForm::TransactionForm* /* form */) override;

    int numColsForm() const override;

    void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets) override;
    void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets) override;
    void tabOrderInForm(QWidgetList& tabOrderWidgets) const override;
    void tabOrderInRegister(QWidgetList& tabOrderWidgets) const override;
    KMyMoneyRegister::Action actionType() const override;

    int numRowsRegister(bool expanded) const override;

    /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
    int numRowsRegister() const override;

    TransactionEditor* createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate) override;

    void splits(MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& feeSplits) const;

  protected:
    bool haveShares() const;
    bool haveFees() const;
    bool haveInterest() const;
    bool havePrice() const;
    bool haveAmount() const;
    bool haveAssetAccount() const;
    bool haveSplitRatio() const;

    /**
    * Returns textual representation of the activity identified
    * by @p type.
    *
    * @param txt reference to QString where to store the result
    * @param type activity represented as investTransactionTypeE
    */
    void activity(QString& txt, eMyMoney::Split::InvestmentTransactionType type) const;

  private:
    InvestTransaction();
    Q_DECLARE_PRIVATE(InvestTransaction)
  };

  inline void swap(InvestTransaction& first, InvestTransaction& second) // krazy:exclude=inline
  {
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
  }

  inline InvestTransaction::InvestTransaction(InvestTransaction && other) : InvestTransaction() // krazy:exclude=inline
  {
    swap(*this, other);
  }

  inline InvestTransaction & InvestTransaction::operator=(InvestTransaction other) // krazy:exclude=inline
  {
    swap(*this, other);
    return *this;
  }

} // namespace

#endif
