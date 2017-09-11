/***************************************************************************
                          kpayeesview.h
                          -------------
    begin                : Thu Jan 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                                    2005 by Andrea Nicolai <Andreas.Nicolai@gmx.net>
                                    2006 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KPAYEESVIEW_H
#define KPAYEESVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QVector>
#include <QWidget>
#include <QResizeEvent>
#include <QList>
#include <QMenu>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistwidgetsearchline.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kpayeesviewdecl.h"
#include "mymoneypayee.h"
#include "mymoneycontact.h"

/**
  * @author Michael Edwardes, Thomas Baumgart
  */

/**
  * This class represents an item in the payees list view.
  */
class KPayeeListItem : public QListWidgetItem
{
public:
  /**
    * Constructor to be used to construct a payee entry object.
    *
    * @param parent pointer to the QListWidget object this entry should be
    *               added to.
    * @param payee const reference to MyMoneyPayee for which
    *               the QListWidget entry is constructed
    */
  KPayeeListItem(QListWidget *parent, const MyMoneyPayee& payee);
  ~KPayeeListItem();

  const MyMoneyPayee& payee() const {
    return m_payee;
  };

private:
  MyMoneyPayee  m_payee;
};

class KPayeesView : public QWidget, private Ui::KPayeesViewDecl
{
  Q_OBJECT

public:
  KPayeesView(QWidget *parent = 0);
  ~KPayeesView();

  void setDefaultFocus();

  void showEvent(QShowEvent* event);

  enum filterTypeE { eAllPayees = 0, eReferencedPayees = 1, eUnusedPayees = 2 };

public slots:
  void slotSelectPayeeAndTransaction(const QString& payeeId, const QString& accountId = QString(), const QString& transactionId = QString());
  void slotLoadPayees();
  void slotStartRename(QListWidgetItem*);
  void slotHelp();

protected:
  void loadPayees();
  void selectedPayees(QList<MyMoneyPayee>& payeesList) const;
  void ensurePayeeVisible(const QString& id);
  void clearItemData();

protected slots:
  /**
    * This method loads the m_transactionList, clears
    * the m_TransactionPtrVector and rebuilds and sorts
    * it according to the current settings. Then it
    * loads the m_transactionView with the transaction data.
    */
  void showTransactions();

  /**
    * This slot is called whenever the selection in m_payeesList
    * is about to change.
    */
  void slotSelectPayee(QListWidgetItem* cur, QListWidgetItem* prev);

  /**
    * This slot is called whenever the selection in m_payeesList
    * has been changed.
    */
  void slotSelectPayee();

  /**
    * This slot marks the current selected payee as modified (dirty).
    */
  void slotPayeeDataChanged();
  void slotKeyListChanged();

  /**
    * This slot is called when the name of a payee is changed inside
    * the payee list view and only a single payee is selected.
    */
  void slotRenamePayee(QListWidgetItem *p);

  /**
    * Updates the payee data in m_payee from the information in the
    * payee information widget.
    */
  void slotUpdatePayee();

  void slotSelectTransaction();

  void slotPayeeNew();

  void slotRenameButtonCliked();

  void slotChangeFilter(int index);

private slots:
  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * If @p points to a real payee item, emits openContextMenu().
    *
    * @param p position of the pointer device
    */
  void slotOpenContextMenu(const QPoint& p);

  void slotChooseDefaultAccount();

  /**
    * Fetches the payee data from addressbook.
    */
  void slotSyncAddressBook();
  void slotContactFetched(const ContactData &identity);

  /**
    * Creates mail to payee.
    */
  void slotSendMail();

signals:
  void transactionSelected(const QString& accountId, const QString& transactionId);
  void openContextMenu(const MyMoneyObject& obj);
  void selectObjects(const QList<MyMoneyPayee>& payees);

  /**
    * This signal is emitted whenever the view is about to be shown.
    */
  void aboutToShow();

private:
  MyMoneyPayee m_payee;
  QString      m_newName;
  MyMoneyContact *m_contact;
  int          m_payeeRow;
  QList<int>   m_payeeRows;

  /**
    * List of selected payees
    */
  QList<MyMoneyPayee> m_selectedPayeesList;

  /**
    * This member holds a list of all transactions
    */
  QList<QPair<MyMoneyTransaction, MyMoneySplit> > m_transactionList;


  /**
    * This member holds the state of the toggle switch used
    * to suppress updates due to MyMoney engine data changes
    */
  bool m_needReload;

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;

  /**
    * Search widget for the list
    */
  KListWidgetSearchLine*  m_searchWidget;

  /**
   * Semaphore to suppress loading during selection
   */
  bool m_inSelection;

  /**
   * This signals whether a payee can be edited
   **/
  bool m_allowEditing;

  /**
    * This holds the filter type
    */
  int m_payeeFilterType;

  AccountNamesFilterProxyModel *m_filterProxyModel;

  /** Checks whether the currently selected payee is "dirty"
   * @return true, if payee is modified (is "dirty"); false otherwise
   */
  bool isDirty() const;

  /** Sets the payee's "dirty" (modified) status
   * @param dirty if true (default), payee will be set to dirty
   */
  void setDirty(bool dirty = true);

  /** Initializes page and sets its load status to initialized
   */
  void init();
};

#endif
