/***************************************************************************
                          kbudgetview.h
                          -------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
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

#include <qwidget.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>
#include <kmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbudgetviewdecl.h"
#include "../mymoney/mymoneybudget.h"
#include "../mymoney/mymoneysecurity.h"
class KMyMoneyAccountTreeBudgetItem;

/**
  * @author Darren Gould
  * @author Thomas Baumgart
  *
  *
  * This class represents an item in the budgets list view.
  */
class KBudgetListItem : public K3ListViewItem
{
public:
  /**
    * Constructor to be used to construct a budget entry object.
    *
    * @param parent pointer to the K3ListView object this entry should be
    *               added to.
    * @param budget const reference to MyMoneyBudget for which
    *               the K3ListView entry is constructed
    */
  KBudgetListItem(K3ListView *parent, const MyMoneyBudget& budget);
  ~KBudgetListItem();

  /**
    * This method is re-implemented from QListViewItem::paintCell().
    * Besides the standard implementation, the QPainter is set
    * according to the applications settings.
    */
  void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);

  const MyMoneyBudget& budget(void) { return m_budget; };
  void setBudget(const MyMoneyBudget& budget) { m_budget = budget; }

private:
  MyMoneyBudget  m_budget;
};

/**
  * @author Darren Gould
  * @author Thomas Baumgart
  */
class KBudgetView : public KBudgetViewDecl
{
   Q_OBJECT
public:
  KBudgetView(QWidget *parent=0, const char *name=0);
  ~KBudgetView();
  void show();

  /**
   * Override the base class behaviour to restore the layout. Do not
   * do this in show() because show() itself may change the layout
   * in undesired ways.
   */
  void polish(void);

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
  void slotReloadView(void);
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
  void resizeEvent(QResizeEvent*);
  void loadAccounts(void);
  bool loadSubAccounts(KMyMoneyAccountTreeBudgetItem* parent, QStringList& accountList, const MyMoneyBudget& budget);

  /**
   * This method loads all available budgets into the budget list widget. If a budget is
   * currently selected it remains selected if it is still present.
   */
  void loadBudgets(void);
  void ensureBudgetVisible(const QString& id);
  const MyMoneyBudget& selectedBudget(void) const;
  KMyMoneyAccountTreeBudgetItem* selectedAccount(void) const;
  void setTimeSpan(KMyMoneyAccountTreeBudgetItem *account, MyMoneyBudget::AccountGroup& accountGroup, int iTimeSpan);
  void askSave(void);

protected slots:

  /**
    * This slot is called when the name of a budget is changed inside
    * the budget list view and only a single budget is selected.
    *
    * @param p The listviewitem containing the budget name
    * @param col The column where the name is located
    * @param txt The text of the new name
    */
  void slotRenameBudget(Q3ListViewItem *p, int col, const QString& txt);

  /**
    * This slot is called when the amount of a budget is changed. It
    * updates the budget and stores it in the engine
    */
  void slotBudgetedAmountChanged(void);

  /**
    */
  void slotSelectAccount(Q3ListViewItem*);

  void AccountEnter();

  void slotUpdateBudget(void);

  void slotResetBudget(void);

  void slotNewBudget(void);

  void languageChange(void);

private slots:
  void slotRearrange(void);

  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * If @p item points to a real budget item, emits openContextMenu().
    *
    * @param lv pointer to the listview
    * @param i the item on which the cursor resides
    * @param p position of the pointing device
    */
  void slotOpenContextMenu(K3ListView* lv, Q3ListViewItem* i, const QPoint& p);

signals:
  /**
    * This signal serves as proxy for KMyMoneyBudgetList::selectObject()
    */
  void openContextMenu(const MyMoneyObject& obj);
  void selectObjects(const Q3ValueList<MyMoneyBudget>& budget);

private:
  typedef enum {
    eNone=-1,
    eYearly=0,
    eMonthly=1,
    eMonthByMonth=2
  } eTimePeriodColumn;

  MyMoneyBudget                       m_budget;

  QMap<QString, unsigned long>        m_transactionCountMap;
  QStringList                         m_yearList;

  KMyMoneyAccountTreeBudgetItem*      m_incomeItem;
  KMyMoneyAccountTreeBudgetItem*      m_expenseItem;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload;

  // set if we are in the selection of a different budget
  bool                                m_inSelection;

  void adaptHideUnusedButton(void);

  static const int m_iBudgetYearsAhead;
  static const int m_iBudgetYearsBack;
};

#endif
