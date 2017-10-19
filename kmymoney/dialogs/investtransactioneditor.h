/***************************************************************************
                             investtransactioneditor.h
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

#ifndef INVESTTRANSACTIONEDITOR_H
#define INVESTTRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "transactioneditor.h"

class InvestTransactionEditor : public TransactionEditor
{
  friend class InvestTransactionEditorPrivate;

  Q_OBJECT

public:
  typedef enum {
    Price = 0,
    PricePerShare,
    PricePerTransaction
  } priceModeE;

  InvestTransactionEditor();
  InvestTransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::InvestTransaction* item, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate);
  virtual ~InvestTransactionEditor();

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
  virtual bool isComplete(QString& reason) const;

  virtual QWidget* firstWidget() const;

  virtual bool fixTransactionCommodity(const MyMoneyAccount& /* account */) {
    return true;
  }

  void totalAmount(MyMoneyMoney& amount) const;

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
  bool createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool skipPriceDialog = false);

  priceModeE priceMode() const;

  const MyMoneySecurity& security() const {
    return m_security;
  }

  const QList<MyMoneySplit>& feeSplits() const {
    return m_feeSplits;
  }

  const QList<MyMoneySplit>& interestSplits() const {
    return m_interestSplits;
  }

protected slots:
  void slotCreateSecurity(const QString& name, QString& id);
  void slotCreateFeeCategory(const QString& name, QString& id);
  void slotCreateInterestCategory(const QString& name, QString& id);

  int slotEditInterestSplits();
  int slotEditFeeSplits();
  void slotReloadEditWidgets();

  void slotUpdateActivity(MyMoneySplit::investTransactionTypeE);
  void slotUpdateSecurity(const QString& stockId);
  void slotUpdateInterestCategory(const QString& id);
  void slotUpdateInterestVisibility(const QString&);
  void slotUpdateFeeCategory(const QString& id);
  void slotUpdateFeeVisibility(const QString&);
  void slotUpdateTotalAmount();
  void slotTransactionContainerGeometriesUpdated();
  void slotUpdateInvestMemoState();

protected:
  /**
    * This method creates all necessary widgets for this transaction editor.
    * All signals will be connected to the relevant slots.
    */
  void createEditWidgets();

  /**
    * This method (re-)loads the widgets with the transaction information
    * contained in @a m_transaction and @a m_split.
    *
    * @param action preset the edit wigdets for @a action if no transaction
    *               is present
    */
  void loadEditWidgets(KMyMoneyRegister::Action action = KMyMoneyRegister::ActionNone);

  void activityFactory(MyMoneySplit::investTransactionTypeE type);

  MyMoneyMoney subtotal(const QList<MyMoneySplit>& splits) const;

  /**
   * This method creates a transaction to be used for the split fee/interest editor.
   * It has a reference to a phony account and the splits contained in @a splits .
   */
  bool createPseudoTransaction(MyMoneyTransaction& t, const QList<MyMoneySplit>& splits);

  /**
   * Convenience method used by slotEditInterestSplits() and slotEditFeeSplits().
   *
   * @param categoryWidgetName name of the category widget
   * @param amountWidgetName name of the amount widget
   * @param splits the splits that make up the transaction to be edited
   * @param isIncome @c false for fees, @c true for interest
   * @param slotEditSplits name of the slot to be connected to the focusIn signal of the
   *                       category widget named @p categoryWidgetName in case of multiple splits
   *                       in @p splits .
   */
  int editSplits(const QString& categoryWidgetName, const QString& amountWidgetName, QList<MyMoneySplit>& splits, bool isIncome, const char* slotEditSplits);

  void updatePriceMode(const MyMoneySplit& split = MyMoneySplit());

  void setupFinalWidgets();

private:
  MyMoneySplit                              m_assetAccountSplit;
  QList<MyMoneySplit>                       m_interestSplits;
  QList<MyMoneySplit>                       m_feeSplits;
  MyMoneySecurity                           m_security;
  MyMoneySecurity                           m_currency;
  MyMoneySplit::investTransactionTypeE      m_transactionType;
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

#endif // INVESTTRANSACTIONEDITOR_H
