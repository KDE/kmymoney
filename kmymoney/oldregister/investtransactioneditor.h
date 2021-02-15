/*
 * SPDX-FileCopyrightText: 2007-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef OLDINVESTTRANSACTIONEDITOR_H
#define OLDINVESTTRANSACTIONEDITOR_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "transactioneditor.h"

class MyMoneyMoney;
class MyMoneySecurity;

namespace eDialogs { enum class PriceMode; }

namespace KMyMoneyRegister { class InvestTransaction; }

namespace eMyMoney { namespace Split {
    enum class InvestmentTransactionType; } }

class OldInvestTransactionEditorPrivate;
class KMM_OLDREGISTER_EXPORT OldInvestTransactionEditor : public TransactionEditor
{
  Q_OBJECT
  Q_DISABLE_COPY(OldInvestTransactionEditor)

public:
  OldInvestTransactionEditor();
  explicit OldInvestTransactionEditor(TransactionEditorContainer* regForm,
                                   KMyMoneyRegister::InvestTransaction* item,
                                   const KMyMoneyRegister::SelectedTransactions& list,
                                   const QDate& lastPostDate);
  ~OldInvestTransactionEditor() override;

  /**
    * This method returns information about the completeness of the data
    * entered. This can be used to control the availability of the
    * 'Enter transaction' action.
    *
    * @retval true if entering the transaction into the engine
    * @retval false if not enough information is present to enter the
    * transaction into the engine
    *
    * @param reason will be filled with a string about the reason why the
    *               completeness is not reached.  Empty if the return value
    *               is @c true.
    *
    * @sa transactionDataSufficient()
    */
  bool isComplete(QString& reason) const override;

  QWidget* firstWidget() const override;

  bool fixTransactionCommodity(const MyMoneyAccount& /* account */) override;

  MyMoneyMoney totalAmount() const;

  bool setupPrice(const MyMoneyTransaction& t, MyMoneySplit& split);

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
    *
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

  eDialogs::PriceMode priceMode() const;

  MyMoneySecurity security() const;

  QList<MyMoneySplit> feeSplits() const;
  QList<MyMoneySplit> interestSplits() const;

protected Q_SLOTS:
  void slotCreateSecurity(const QString& name, QString& id);
  void slotCreateFeeCategory(const QString& name, QString& id);
  void slotCreateInterestCategory(const QString& name, QString& id);

  int slotEditInterestSplits();
  int slotEditFeeSplits();
  void slotReloadEditWidgets();

  void slotUpdateActivity(eMyMoney::Split::InvestmentTransactionType);
  void slotUpdateSecurity(const QString& stockId);
  void slotUpdateInterestCategory(const QString& id);
  void slotUpdateFeeCategory(const QString& id);
  void slotUpdateTotalAmount();
  void slotTransactionContainerGeometriesUpdated();
  void slotUpdateInvestMemoState();

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

  void setupFinalWidgets() override;

private:
  Q_DECLARE_PRIVATE(OldInvestTransactionEditor)
};

#endif // INVESTTRANSACTIONEDITOR_H
