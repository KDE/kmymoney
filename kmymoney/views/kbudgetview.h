/***************************************************************************
                          kbudgetview.h
                          -------------
    begin                : Thu Jan 10 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
                           Alvaro Soliverez <asoliverez@gmail.com>
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

#ifndef KBUDGETVIEW_H
#define KBUDGETVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountsviewbase.h"

#include "mymoneybudget.h"

/**
  * @author Darren Gould
  * @author Thomas Baumgart
  */

class QString;
class QTreeWidgetItem;

class KBudgetViewPrivate;
class KBudgetView : public KMyMoneyAccountsViewBase
{
  Q_OBJECT

public:
  explicit KBudgetView(QWidget *parent = nullptr);
  ~KBudgetView();

  void setDefaultFocus() override;
  void refresh() override;

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
  void slotSelectBudget();
  void slotHideUnused(bool);
  void slotRefreshHideUnusedButton();
  void slotStartRename();

  /**
    *This is to update the information about the checkbox "budget amount integrates subaccounts" into the file, when the user clicks the check box
   */
  void cb_includesSubaccounts_clicked();

protected:
  KBudgetView(KBudgetViewPrivate &dd, QWidget *parent);
  void showEvent(QShowEvent * event) override;
  void loadAccounts();

  /**
   * This method loads all available budgets into the budget list widget. If a budget is
   * currently selected it remains selected if it is still present.
   */
  void loadBudgets();
  void ensureBudgetVisible(const QString& id);
  const MyMoneyBudget& selectedBudget() const;
  void askSave();

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
  void slotBudgetedAmountChanged();

  /**
    */
  void slotSelectAccount(const MyMoneyObject &);

  void AccountEnter();

  void slotUpdateBudget();

  void slotResetBudget();

  void slotNewBudget();

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

private:
  Q_DECLARE_PRIVATE(KBudgetView)

  typedef enum {
    eNone = -1,
    eYearly = 0,
    eMonthly = 1,
    eMonthByMonth = 2
  } eTimePeriodColumn;

  MyMoneyBudget                       m_budget;
  QMap<QString, unsigned long>        m_transactionCountMap;
  QStringList                         m_yearList;

  /**
    * Set if we are in the selection of a different budget
    **/
  bool                                m_inSelection;

  void adaptHideUnusedButton();

  static const int m_iBudgetYearsAhead;
  static const int m_iBudgetYearsBack;

  /**
    * This signals whether a budget is being edited
    **/
  bool m_budgetInEditing;
};

#endif
