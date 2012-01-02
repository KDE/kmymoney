/***************************************************************************
                          kbudgetview.h
                          -------------
    begin                : Thu Jan 10 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
                           Alvaro Soliverez <asoliverez@gmail.com>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBUDGETVIEW_H
#define KBUDGETVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
#include <QResizeEvent>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kbudgetviewdecl.h"
#include "mymoneybudget.h"
#include "mymoneysecurity.h"
#include "kmymoneyaccounttreeview.h"

/**
  * This proxy model implements all the functionality needed by the budgets
  * account tree based on the @ref AccountsModel. One such functionality is
  * obtaining the account balance and value base on the budget.
  *
  * @author Cristin Oneț
  */
class BudgetAccountsProxyModel : public AccountsViewFilterProxyModel
{
  Q_OBJECT

public:
  BudgetAccountsProxyModel(QObject *parent = 0);

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;

  void setBudget(const MyMoneyBudget& budget);

signals:
  /**
    * Emit this signal when the balance of the budget is changed.
    */
  void balanceChanged(const MyMoneyMoney &);

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
  MyMoneyMoney accountBalance(const QString &accountId) const;
  MyMoneyMoney accountValue(const MyMoneyAccount &account, const MyMoneyMoney &balance) const;
  MyMoneyMoney computeTotalValue(const QModelIndex &source_index) const;

private:
  void checkBalance();

private:
  MyMoneyBudget m_budget;
  MyMoneyMoney m_lastBalance;
};


/**
  * @author Darren Gould
  * @author Thomas Baumgart
  */
class KBudgetView : public QWidget, public Ui::KBudgetViewDecl
{
  Q_OBJECT

public:
  KBudgetView(QWidget *parent = 0);
  ~KBudgetView();

  /**
   * Override the base class behaviour to include all updates that
   * happened in the meantime and restore the layout.
   */
  void showEvent(QShowEvent * event);

  /**
    * This method is used to suppress updates for specific times
    * (e.g. during creation of a new MyMoneyFile object when the
    * default accounts are loaded). The behaviour of update() is
    * controlled with the parameter.
    *
    * @param suspend Suspend updates or not. Possible values are
    *
    * @li true updates are suspended
    * @li false updates will be performed immediately
    *
    * When a true/false transition of the parameter between
    * calls to this method is detected,
    * refresh() will be invoked once automatically.
    */
  void suspendUpdate(const bool suspend);

public slots:
  void slotRefreshView(void);
  void slotSelectBudget(void);
  void slotHideUnused(bool);
  void slotRefreshHideUnusedButton();
  void slotStartRename(void);

  /**
    *This is to update the information about the checkbox "budget amount integrates subaccounts" into the file, when the user clicks the check box
   */
  void cb_includesSubaccounts_clicked();

protected:
  void loadAccounts(void);

  /**
   * This method loads all available budgets into the budget list widget. If a budget is
   * currently selected it remains selected if it is still present.
   */
  void loadBudgets(void);
  void ensureBudgetVisible(const QString& id);
  const MyMoneyBudget& selectedBudget(void) const;
  void askSave(void);

  bool collectSubBudgets(MyMoneyBudget::AccountGroup &destination, const QModelIndex &index) const;
  void clearSubBudgets(const QModelIndex &index);

protected slots:

  /**
    * This slot is called when the data of a budget is changed inside
    * the budget list view and only a single budget is selected.
    *
    * @param p The listviewitem containing the budget name
    * @param col The column that has changed
    */
  void slotItemChanged(QTreeWidgetItem* p, int col);

  /**
    * This slot is called when the amount of a budget is changed. It
    * updates the budget and stores it in the engine
    */
  void slotBudgetedAmountChanged(void);

  /**
    */
  void slotSelectAccount(const MyMoneyObject &);

  void slotExpandCollapse(void);

  void AccountEnter();

  void slotUpdateBudget(void);

  void slotResetBudget(void);

  void slotNewBudget(void);

  void languageChange(void);

  void slotBudgetBalanceChanged(const MyMoneyMoney &);

private slots:
  /**
    * This slot receives the signal from the listview control that an
    * item was right-clicked,
    * If @p p points to a real budget item, emits openContextMenu().
    *
    * @param p position of the pointing device
    */
  void slotOpenContextMenu(const QPoint& p);

signals:
  /**
    * This signal serves as proxy for KMyMoneyBudgetList::selectObject()
    */
  void openContextMenu(const MyMoneyObject& obj);
  void selectObjects(const QList<MyMoneyBudget>& budget);

  /**
    * This signal is emitted whenever the view is about to be shown.
    */
  void aboutToShow();

private:
  typedef enum {
    eNone = -1,
    eYearly = 0,
    eMonthly = 1,
    eMonthByMonth = 2
  } eTimePeriodColumn;

  MyMoneyBudget                       m_budget;
  QMap<QString, unsigned long>        m_transactionCountMap;
  QStringList                         m_yearList;

  BudgetAccountsProxyModel            *m_filterProxyModel;
  /**
    * Set if a view needs to be reloaded during showEvent()
    **/
  bool                                m_needReload;

  /**
    * Set if we are in the selection of a different budget
    **/
  bool                                m_inSelection;

  void adaptHideUnusedButton(void);

  static const int m_iBudgetYearsAhead;
  static const int m_iBudgetYearsBack;

  /**
    * This signals whether a budget is being edited
    **/
  bool m_budgetInEditing;
};

#endif
