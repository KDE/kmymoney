/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INVESTTRANSACTION_H
#define INVESTTRANSACTION_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "transaction.h"

namespace eMyMoney { namespace Split { enum class InvestmentTransactionType; } }

namespace KMyMoneyRegister
{

  class InvestTransactionPrivate;
  class KMM_OLDREGISTER_EXPORT InvestTransaction : public Transaction
  {
    Q_DISABLE_COPY(InvestTransaction)

  public:
    explicit InvestTransaction(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
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
    eWidgets::eRegister::Action actionType() const override;

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
    Q_DECLARE_PRIVATE(InvestTransaction)
  };
} // namespace

#endif
