/***************************************************************************
                          kfindtransactiondlg.h
                             -------------------
    copyright            : (C) 2003 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KFINDTRANSACTIONDLG_H
#define KFINDTRANSACTIONDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <q3listview.h>
#include <QDateTime>
#include <qmap.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QEvent>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

// #include "kledgerview.h"
#include <mymoneyutils.h>
#include <mymoneytransactionfilter.h>

#include "ui_kfindtransactiondlgdecl.h"
#include "ui_ksortoptiondlg.h"

class Q3ListView;
class Q3ListViewItem;

/**
  * @author Thomas Baumgart
  */


class KSortOptionDlg : public QDialog, public Ui::KSortOptionDlg
{
public:
    KSortOptionDlg( QWidget *parent );
    void init();
    void setSortOption(const QString& option, const QString& def);
    QString sortOption() const;
    void hideDefaultButton();
};



class KFindTransactionDlgDecl : public QDialog, public Ui::KFindTransactionDlgDecl
{
public:
  KFindTransactionDlgDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};

class KFindTransactionDlg : public KFindTransactionDlgDecl
{
  Q_OBJECT
public:

  /*
  // Make sure to keep the following enum valus in sync with the values
  // used by the GUI in kfindtransactiondlgdecl.ui
  enum dateOptionE {
    allDates = 0,
    asOfToday,
    currentMonth,
    currentYear,
    monthToDate,
    yearToDate,
    yearToMonth,
    lastMonth,
    lastYear,
    last7Days,
    last30Days,
    last3Months,
    last6Months,
    last12Months,
    next7Days,
    next30Days,
    next3Months,
    next6Months,
    next12Months,
    userDefined,
    last3ToNext3Months,
    last11Months,
    // insert new constants above of this line
    dateOptionCount
  };
*/
  KFindTransactionDlg(QWidget *parent=0);
  ~KFindTransactionDlg() {}

  virtual bool eventFilter( QObject *o, QEvent *e );

public slots:
  void show(void);

protected:
  void resizeEvent(QResizeEvent*);

protected slots:
  virtual void slotReset(void);
  virtual void slotSearch(void);

  /**
    * This slot opens the detailed help page in khelpcenter. The
    * anchor for the information is taken from m_helpAnchor.
    */
  virtual void slotShowHelp(void);


  void slotUpdateSelections(void);

  virtual void slotDateRangeChanged(int);
  virtual void slotDateChanged(void);

  virtual void slotAmountSelected(void);
  virtual void slotAmountRangeSelected(void);

  virtual void slotSelectAllPayees(void);
  virtual void slotDeselectAllPayees(void);

  virtual void slotNrSelected(void);
  virtual void slotNrRangeSelected(void);

  void slotRefreshView(void);

  /**
    * This slot selects the current selected transaction/split and emits
    * the signal @a transactionSelected(const QString& accountId, const QString& transactionId)
    */
  void slotSelectTransaction(void);

  void slotRightSize(void);

  void slotSortOptions(void);

signals:
  void transactionSelected(const QString& accountId, const QString& transactionId);

  /**
    * This signal is sent out when no selection has been made. It is
    * used to control the state of the Search button.
    */
  void selectionEmpty(bool);

protected:
  enum opTypeE {
    addAccountToFilter = 0,
    addCategoryToFilter,
    addPayeeToFilter
  };

  void setupCategoriesPage(void);
  void setupDatePage(void);
  void setupAccountsPage(void);
  void setupAmountPage(void);
  void setupPayeesPage(void);
  void setupDetailsPage(void);

  void setupFilter(void);

  void selectAllItems(Q3ListView* view, const bool state);
  void selectAllSubItems(Q3ListViewItem* item, const bool state);
  void selectItems(Q3ListView* view, const QStringList& list, const bool state);
  void selectSubItems(Q3ListViewItem* item, const QStringList& list, const bool state);

  /**
    * This method loads the m_payeesView with the payees name
    * found in the engine.
    */
  void loadPayees(void);

  /**
    * This method loads the register with the matching transactions
    */
  void loadView(void);

  /**
    * This method returns information about the selection state
    * of the items in the m_accountsView.
    *
    * @param view pointer to the listview to scan
    *
    * @retval true if all items in the view are marked
    * @retval false if at least one item is not marked
    *
    * @note If the view contains no items the method returns @p true.
    */
  bool allItemsSelected(const Q3ListView* view) const;
  bool allItemsSelected(const Q3ListViewItem *item) const;

  void scanCheckListItems(const Q3ListView* view, const opTypeE op);
  void scanCheckListItems(const Q3ListViewItem* item, const opTypeE op);
  void addItemToFilter(const opTypeE op, const QString& id);

protected:
  QDate                m_startDates[MyMoneyTransactionFilter::dateOptionCount];
  QDate                m_endDates[MyMoneyTransactionFilter::dateOptionCount];

  /**
    * This member holds a list of all transactions matching the filter criteria
    */
  QList<QPair<MyMoneyTransaction, MyMoneySplit> > m_transactionList;

  MyMoneyTransactionFilter        m_filter;

  QMap<QWidget*, QString>         m_helpAnchor;

  bool                            m_needReload;
};

#endif
