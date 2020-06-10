/*
 * Copyright 2009-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef STDTRANSACTIONEDITOR_H
#define STDTRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "transactioneditor.h"

class MyMoneyMoney;

namespace eWidgets { namespace eRegister { enum class CashFlowDirection; } }

class StdTransactionEditorPrivate;
class StdTransactionEditor : public TransactionEditor
{
  Q_OBJECT
public:
  StdTransactionEditor();
  explicit StdTransactionEditor(TransactionEditorContainer* regForm,
                                KMyMoneyRegister::Transaction* item,
                                const KMyMoneyRegister::SelectedTransactions& list,
                                const QDate& lastPostDate);
  ~StdTransactionEditor() override;

  bool isComplete(QString& reason) const override;
  QWidget* firstWidget() const override;

  /**
    * This method creates a transaction based on the contents of the current widgets,
    * the splits in m_split in single selection mode or an existing transaction/split
    * and the contents of the widgets in multi selection mode.
    *
    * The split referencing the current account is returned as the first split in the
    * transaction's split list.
    *
    * @param t reference to created transaction
    * @param torig the original transaction
    * @param sorig the original split
    * @param skipPriceDialog if @p true the user will not be requested for price information
    *                        (defaults to @p false)
    *
    * @return @p false if aborted by user, @p true otherwise
    *
    * @note Usually not used directly. If unsure, use enterTransactions() instead.
    */
  bool createTransaction(MyMoneyTransaction& t,
                         const MyMoneyTransaction& torig,
                         const MyMoneySplit& sorig,
                         bool skipPriceDialog = false) override;

public Q_SLOTS:
  int slotEditSplits() override;
  void slotUpdateAmount(const QString&);

protected Q_SLOTS:
  void slotReloadEditWidgets();
  void slotUpdatePayment(const QString&);
  void slotUpdateDeposit(const QString&);
  void slotUpdateCategory(const QString&);
  void slotUpdatePayee(const QString&);
  //void slotUpdateTag(const QString&);
  void slotUpdateCashFlow(eWidgets::eRegister::CashFlowDirection);
  void slotCreateCategory(const QString&, QString&);
  void slotUpdateAction(int action);
  void slotUpdateAccount(const QString& id) override;

protected:
  /**
    * This method creates all necessary widgets for this transaction editor.
    * All signals will be connected to the relevant slots.
    */
  void createEditWidgets() override;

  /**
    * This method (re-)loads the widgets with the transaction information
    * contained in @a m_transaction and @a m_split.
    *
    * @param action preset the edit widgets for @a action if no transaction
    *               is present
    */
  void loadEditWidgets(eWidgets::eRegister::Action action) override;
  void loadEditWidgets() override;

  void setupCategoryWidget(QString&);
  void updateAmount(const MyMoneyMoney& value);
  bool isTransfer(const QString& accId1, const QString& accId2) const;

  void checkPayeeInSplit(MyMoneySplit& s, const QString& payeeId);

  /**
    * This method fills the editor widgets with the last transaction
    * that can be found for payee @a payeeId in the account @a m_account.
    */
  void autoFill(const QString& payeeId);

  /**
    * Extracts the amount of the transaction from the widgets depending
    * if form or register based input method is used.
    * Returns if an amount has been found in @a update.
    *
    * @param update pointer to update information flag
    * @return amount of transaction (deposit positive, payment negative)
    */
  MyMoneyMoney amountFromWidget(bool* update = 0) const;

  /**
    * Create or update a VAT split
    */
  void updateVAT(bool amountChanged = true);

  MyMoneyMoney removeVatSplit();

  /**
    * This method adds a VAT split to transaction @a tr if necessary.
    *
    * @param tr transaction that the split should be added to
    * @param amount Amount to be used for the calculation. Depending upon the
    *               setting of the resp. category, this value is treated as
    *               either gross or net value.
    * @retval false VAT split has not been added
    * @retval true VAT split has been added
    */
  bool addVatSplit(MyMoneyTransaction& tr, const MyMoneyMoney& amount);

  void setupFinalWidgets() override;

  /**
   * This method returns the sum of all splits of transaction @a t that
   * reference account m_account.
   */
  MyMoneyMoney shares(const MyMoneyTransaction& t) const;

private:
  Q_DECLARE_PRIVATE(StdTransactionEditor)
};


#endif
