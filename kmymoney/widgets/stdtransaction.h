/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef STDTRANSACTION_H
#define STDTRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "transaction.h"

namespace KMyMoneyRegister
{
  class StdTransactionPrivate;
  class StdTransaction : public Transaction
  {
    Q_DISABLE_COPY(StdTransaction)

  public:
    explicit StdTransaction(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    ~StdTransaction() override;

    virtual const char* className() override;

    bool formCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0) override;
    void registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0) override;

    int registerColWidth(int col, const QFontMetrics& cellFontMetrics) override;
    void setupForm(KMyMoneyTransactionForm::TransactionForm* form) override;
    void loadTab(KMyMoneyTransactionForm::TransactionForm* form) override;

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

    /**
    * Return information if @a row should be shown (@a true )
    * or hidden (@a false ) in the form. Default is true.
    */
    virtual bool showRowInForm(int row) const override;

    /**
    * Control visibility of @a row in the transaction form.
    * Only row 0 has an effect, others return @a true.
    */
    virtual void setShowRowInForm(int row, bool show) override;

  protected:
    Q_DECLARE_PRIVATE(StdTransaction)
    void setupFormHeader(const QString& id);
  };
} // namespace

#endif
