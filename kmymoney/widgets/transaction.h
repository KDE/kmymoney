/***************************************************************************
                          transaction.h  -  description
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

#ifndef TRANSACTION_H
#define TRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QPalette>
#include <QList>
#include <QColor>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <registeritem.h>
#include <mymoneytransaction.h>
#include <mymoneysplit.h>
#include <mymoneysecurity.h>
#include <selectedtransaction.h>
#include <mymoneyaccount.h>

class QTableWidget;
class TransactionEditor;
class TransactionEditorContainer;
class MyMoneyTag;

namespace KMyMoneyTransactionForm
{
class TransactionForm;
} // namespace

namespace KMyMoneyRegister
{

// keep the following list in sync with code in the constructor
// of KMyMoneyRegister::Register in register.cpp
typedef enum {
  NumberColumn = 0,
  DateColumn,
  AccountColumn,
  SecurityColumn,
  DetailColumn,
  ReconcileFlagColumn,
  PaymentColumn,
  DepositColumn,
  QuantityColumn,
  PriceColumn,
  ValueColumn,
  BalanceColumn,
  // insert new values above this line
  MaxColumns
} Column;

class Transaction : public RegisterItem
{
public:
  Transaction(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~Transaction() {}

  virtual const char* className() {
    return "Transaction";
  }

  bool isSelectable() const {
    return true;
  }
  bool isSelected() const {
    return m_selected;
  }
  void setSelected(bool selected);

  bool canHaveFocus() const {
    return true;
  }
  bool hasFocus() const {
    return m_focus;
  }
  bool hasEditorOpen() const {
    return m_inEdit;
  }

  virtual bool isScheduled() const {
    return false;
  }

  void setFocus(bool focus, bool updateLens = true);

  bool isErroneous() const {
    return m_erroneous;
  }

  virtual const QDate& sortPostDate() const {
    return m_transaction.postDate();
  }
  virtual int sortSamePostDate() const {
    return 2;
  }
  virtual const QDate& sortEntryDate() const {
    return m_transaction.entryDate();
  }
  virtual const QString& sortPayee() const {
    return m_payee;
  }
  virtual const QStringList& sortTagIdList() const {
    return m_tagIdList;
  }
  virtual const MyMoneyMoney& sortValue() const {
    return m_split.shares();
  }
  virtual const QString& sortNumber() const {
    return m_split.number();
  }
  virtual const QString& sortEntryOrder() const {
    return m_uniqueId;
  }
  virtual CashFlowDirection sortType() const {
    return m_split.shares().isNegative() ? Payment : Deposit;
  }
  virtual const QString& sortCategory() const {
    return m_category;
  }
  virtual MyMoneySplit::reconcileFlagE sortReconcileState() const {
    return m_split.reconcileFlag();
  }

  virtual const QString& id() const {
    return m_uniqueId;
  }
  const MyMoneyTransaction& transaction() const {
    return m_transaction;
  }
  const MyMoneySplit& split() const {
    return m_split;
  }

  void setBalance(const MyMoneyMoney& balance) {
    m_balance = balance;
  }
  const MyMoneyMoney& balance() const {
    return m_balance;
  }

  virtual int rowHeightHint() const;

  virtual bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItemV4 &option, const QModelIndex &index);
  virtual void paintRegisterCell(QPainter* painter, QStyleOptionViewItemV4& option, const QModelIndex& index);

  virtual void paintFormCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);
  virtual bool formCellText(QString& /* txt */, Qt::Alignment& /* align */, int /* row */, int /* col */, QPainter* /* painter */) {
    return false;
  }
  virtual void registerCellText(QString& /* txt */, Qt::Alignment& /* align */, int /* row */, int /* col */, QPainter* /* painter */) {}
  virtual int registerColWidth(int /* col */, const QFontMetrics& /* cellFontMetrics */) {
    return 0;
  }

  /**
    * Helper method for the above method.
    */
  void registerCellText(QString& txt, int row, int col);

  virtual int formRowHeight(int row);
  virtual int formRowHeight() const;

  virtual void setupForm(KMyMoneyTransactionForm::TransactionForm* form);
  virtual void setupFormPalette(QMap<QString, QWidget*>& editWidgets);
  virtual void setupRegisterPalette(QMap<QString, QWidget*>& editWidgets);
  virtual void loadTab(KMyMoneyTransactionForm::TransactionForm* form) = 0;

  virtual void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets) = 0;
  virtual void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets) = 0;
  virtual void tabOrderInForm(QWidgetList& tabOrderWidgets) const = 0;
  virtual void tabOrderInRegister(QWidgetList& tabOrderWidgets) const = 0;

  virtual KMyMoneyRegister::Action actionType() const = 0;

  QWidget* focusWidget(QWidget*) const;
  void arrangeWidget(QTableWidget* tbl, int row, int col, QWidget* w) const;

  bool haveNumberField() const;

  bool matches(const RegisterFilter&) const;

  /**
    * Checks if the mouse hovered over an area that has a tooltip associated with it.
    * The mouse position is given in relative coordinates to the @a startRow and the
    * @a row and @a col of the item are also passed as relative values.
    *
    * If a tooltip shall be shown, this method presets the rectangle @a r with the
    * area in register coordinates and @a msg with the string that will be passed
    * to QToolTip::tip. @a true is returned in this case.
    *
    * If no tooltip is available, @a false will be returned.
    */
  virtual bool maybeTip(const QPoint& relpos, int row, int col, QRect& r, QString& msg);

  /**
    * This method returns the number of register rows required for a certain
    * item in expanded (@p expanded equals @a true) or collapsed (@p expanded
    * is @a false) mode.
    *
    * @param expanded returns number of maximum rows required for this item to
    *                 display all information (used for ledger lens and register
    *                 edit mode) or the minimum number of rows required.
    * @return number of rows required for mode selected by @p expanded
    */
  virtual int numRowsRegister(bool expanded) const = 0;

  virtual int numRowsRegister() const = 0;

  void leaveEditMode();
  void startEditMode();

  /**
    * This method creates an editor for the transaction
    */
  virtual TransactionEditor* createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate) = 0;

  virtual void setVisible(bool visible);

  virtual void setShowBalance(bool showBalance);

  /**
    * Return information if @a row should be shown (@a true )
    * or hidden (@a false ) in the form. Default is true.
    */
  virtual bool showRowInForm(int row) const {
    Q_UNUSED(row) return true;
  }

  /**
    * Control visibility of @a row in the transaction form.
    * Only row 0 has an effect, others return @a true.
    */
  virtual void setShowRowInForm(int row, bool show) {
    Q_UNUSED(row); Q_UNUSED(show)
  }

  virtual void setReducedIntensity(bool reduced) {
    m_reducedIntensity = reduced;
  }

protected:
  /**
    * This method converts m_split.reconcileFlag() into a readable string
    *
    * @param text Return textual representation e.g. "Cleared" (@a true) or just
    *             a flag e.g. "C" (@a false). Defaults to textual representation.
    * @return Textual representation or flag as selected via @p text of the
    *         reconciliation state of the split
    */
  QString reconcileState(bool text = true) const;

  /**
    * Helper method to reduce a multi line memo text into a single line.
    *
    * @param txt QString that will receive the single line memo text
    * @param split const reference to the split to take the memo from
    */
  void singleLineMemo(QString& txt, const MyMoneySplit& split) const;

  virtual void setupPalette(const QPalette& palette, QMap<QString, QWidget*>& editWidgets);

protected:
  MyMoneyTransaction      m_transaction;
  MyMoneySplit            m_split;
  MyMoneyAccount          m_account;
  MyMoneyMoney            m_balance;
  QTableWidget*           m_form;
  QString                 m_category;
  QString                 m_payee;
  QString                 m_payeeHeader;
  QStringList             m_tagIdList;
  QString                 m_categoryHeader;
  QString                 m_splitCurrencyId;
  QString                 m_uniqueId;
  int                     m_formRowHeight;
  bool                    m_selected;
  bool                    m_focus;
  bool                    m_erroneous;
  bool                    m_inEdit;
  bool                    m_inRegisterEdit;
  bool                    m_showBalance;
  bool                    m_reducedIntensity;
};

class StdTransaction : public Transaction
{
public:
  StdTransaction(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~StdTransaction() {}

  virtual const char* className() {
    return "StdTransaction";
  }

  bool formCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0);
  void registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0);

  int registerColWidth(int col, const QFontMetrics& cellFontMetrics);
  void setupForm(KMyMoneyTransactionForm::TransactionForm* form);
  void loadTab(KMyMoneyTransactionForm::TransactionForm* form);

  int numColsForm() const {
    return 4;
  }

  void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets);
  void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets);
  void tabOrderInForm(QWidgetList& tabOrderWidgets) const;
  void tabOrderInRegister(QWidgetList& tabOrderWidgets) const;
  KMyMoneyRegister::Action actionType() const;

  int numRowsRegister(bool expanded) const;

  /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
  int numRowsRegister() const {
    return RegisterItem::numRowsRegister();
  }

  TransactionEditor* createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate);

  /**
    * Return information if @a row should be shown (@a true )
    * or hidden (@a false ) in the form. Default is true.
    */
  virtual bool showRowInForm(int row) const;

  /**
    * Control visibility of @a row in the transaction form.
    * Only row 0 has an effect, others return @a true.
    */
  virtual void setShowRowInForm(int row, bool show);

protected:
  void setupFormHeader(const QString& id);
  QString formatTagList() const;
  QString formatTagListAsHtml() const;

private:
  bool m_showAccountRow;
};

class InvestTransaction : public Transaction
{
public:
  InvestTransaction(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~InvestTransaction() {}

  virtual const QString& sortSecurity() const {
    return m_security.name();
  }
  virtual const char* className() {
    return "InvestTransaction";
  }

  bool formCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0);
  void registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0);

  int registerColWidth(int col, const QFontMetrics& cellFontMetrics);
  void setupForm(KMyMoneyTransactionForm::TransactionForm* form);

  /**
    * provide NOP here as the investment transaction form does not supply a tab
    */
  void loadTab(KMyMoneyTransactionForm::TransactionForm* /* form */) {}

  int numColsForm() const {
    return 4;
  }

  void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets);
  void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets);
  void tabOrderInForm(QWidgetList& tabOrderWidgets) const;
  void tabOrderInRegister(QWidgetList& tabOrderWidgets) const;
  KMyMoneyRegister::Action actionType() const {
    return KMyMoneyRegister::ActionNone;
  }

  int numRowsRegister(bool expanded) const;

  /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
  int numRowsRegister() const {
    return RegisterItem::numRowsRegister();
  }

  TransactionEditor* createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate);

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
  void activity(QString& txt, MyMoneySplit::investTransactionTypeE type) const;

private:
  QList<MyMoneySplit>       m_feeSplits;
  QList<MyMoneySplit>       m_interestSplits;
  MyMoneySplit              m_assetAccountSplit;
  MyMoneySecurity           m_security;
  MyMoneySecurity           m_currency;
  MyMoneySplit::investTransactionTypeE    m_transactionType;
  QString                   m_feeCategory;
  QString                   m_interestCategory;
  MyMoneyMoney              m_feeAmount;
  MyMoneyMoney              m_interestAmount;
  MyMoneyMoney              m_totalAmount;
};

} // namespace

#endif
