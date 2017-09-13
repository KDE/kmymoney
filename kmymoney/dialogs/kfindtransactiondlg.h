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

#include <QDateTime>
#include <QMap>
#include <QResizeEvent>
#include <QEvent>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneytransactionfilter.h"

class QTreeWidget;
class QTreeWidgetItem;
class DateRangeDlg;

/**
  * @author Thomas Baumgart
  */
class KSortOptionDlg : public QDialog
{
public:
  KSortOptionDlg(QWidget *parent);
  ~KSortOptionDlg();
  void init();
  void setSortOption(const QString& option, const QString& def);
  QString sortOption() const;
  void hideDefaultButton();

private:
  struct Private;
  Private* const d;
};

namespace Ui
{
class KFindTransactionDlgDecl;
}

class KFindTransactionDlg : public QDialog
{
  Q_OBJECT

public:
  /**
   @param withEquityAccounts set to false to hide equity accounts in account page
  */
  KFindTransactionDlg(QWidget *parent = 0, bool withEquityAccounts = false);
  ~KFindTransactionDlg();

  virtual bool eventFilter(QObject *o, QEvent *e);

protected:
  void resizeEvent(QResizeEvent*);
  void showEvent(QShowEvent* event);

protected slots:
  virtual void slotReset();
  virtual void slotSearch();

  /**
    * This slot opens the detailed help page in khelpcenter. The
    * anchor for the information is taken from m_helpAnchor.
    */
  virtual void slotShowHelp();


  void slotUpdateSelections();

  virtual void slotAmountSelected();
  virtual void slotAmountRangeSelected();

  virtual void slotSelectAllPayees();
  virtual void slotDeselectAllPayees();

  virtual void slotSelectAllTags();
  virtual void slotDeselectAllTags();

  virtual void slotNrSelected();
  virtual void slotNrRangeSelected();

  void slotRefreshView();

  /**
    * This slot selects the current selected transaction/split and emits
    * the signal @a transactionSelected(const QString& accountId, const QString& transactionId)
    */
  void slotSelectTransaction();

  void slotRightSize();

  void slotSortOptions();

signals:
  void transactionSelected(const QString& accountId, const QString& transactionId);

  /**
    * This signal is sent out when a selection has been made. It is
    * used to control the state of the Search button.
    * The Search button is only active when a selection has been made
    * (i.e. notEmpty == true)
    */
  void selectionNotEmpty(bool);

protected:
  enum opTypeE {
    addAccountToFilter = 0,
    addCategoryToFilter,
    addPayeeToFilter,
    addTagToFilter
  };

  void setupCategoriesPage();
  void setupAccountsPage(bool withEquityAccounts=false);
  void setupAmountPage();
  void setupPayeesPage();
  void setupTagsPage();
  void setupDetailsPage();

  void setupFilter();

  void selectAllItems(QTreeWidget* view, const bool state);
  void selectAllSubItems(QTreeWidgetItem* item, const bool state);
  void selectItems(QTreeWidget* view, const QStringList& list, const bool state);
  void selectSubItems(QTreeWidgetItem* item, const QStringList& list, const bool state);

  /**
    * This method loads the m_payeesView with the payees name
    * found in the engine.
    */
  void loadPayees();

  /**
    * This method loads the m_tagsView with the tags name
    * found in the engine.
    */
  void loadTags();

  /**
    * This method loads the register with the matching transactions
    */
  void loadView();

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
  bool allItemsSelected(const QTreeWidget* view) const;
  bool allItemsSelected(const QTreeWidgetItem *item) const;

  void scanCheckListItems(const QTreeWidget* view, const opTypeE op);
  void scanCheckListItems(const QTreeWidgetItem* item, const opTypeE op);
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

  Ui::KFindTransactionDlgDecl*    m_ui;
  DateRangeDlg *m_dateRange;
};

#endif
