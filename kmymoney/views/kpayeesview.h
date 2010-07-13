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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmenu.h>
#include <klistwidgetsearchline.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kpayeesviewdecl.h"
#include "mymoneypayee.h"

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

  const MyMoneyPayee& payee(void) const {
    return m_payee;
  };

private:
  MyMoneyPayee  m_payee;
};

class KPayeesView : public QWidget, public Ui::KPayeesViewDecl
{
  Q_OBJECT

public:
  KPayeesView(QWidget *parent = 0);
  ~KPayeesView();
  void showEvent(QShowEvent* event);

public slots:
  void slotSelectPayeeAndTransaction(const QString& payeeId, const QString& accountId = QString(), const QString& transactionId = QString());
  void slotLoadPayees(void);
  void slotStartRename(QListWidgetItem*);
  void slotHelp(void);

protected:
  void loadPayees(void);
  void selectedPayees(QList<MyMoneyPayee>& payeesList) const;
  void ensurePayeeVisible(const QString& id);
  void clearItemData(void);

protected slots:
  /**
    * This method loads the m_transactionList, clears
    * the m_TransactionPtrVector and rebuilds and sorts
    * it according to the current settings. Then it
    * loads the m_transactionView with the transaction data.
    */
  void showTransactions(void);

  /**
    * This slot is called whenever the selection in m_payeesList
    * has been changed.
    */
  void slotSelectPayee(void);

  /**
    * This slot marks the current selected payee as modified (dirty).
    */
  void slotPayeeDataChanged(void);
  void slotKeyListChanged(void);

  /**
    * This slot is called when the name of a payee is changed inside
    * the payee list view and only a single payee is selected.
    */
  void slotRenamePayee(QListWidgetItem *p);

  /**
    * Updates the payee data in m_payee from the information in the
    * payee information widget.
    */
  void slotUpdatePayee(void);

  void slotSelectTransaction(void);

  void slotPayeeNew(void);

  void slotRenameButtonCliked(void);

private slots:
  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * If @p points to a real payee item, emits openContextMenu().
    *
    * @param p position of the pointer device
    */
  void slotOpenContextMenu(const QPoint& p);

  void slotChooseDefaultAccount(void);

signals:
  void transactionSelected(const QString& accountId, const QString& transactionId);
  void openContextMenu(const MyMoneyObject& obj);
  void selectObjects(const QList<MyMoneyPayee>& payees);

private:
  MyMoneyPayee m_payee;
  QString      m_newName;

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
    * Search widget for the list
    */
  KListWidgetSearchLine*  m_searchWidget;
  bool m_needConnection;

  /**
    * Counting semaphore to collect updates
    */
  int m_updatesQueued;

  /**
   * Semaphore to suppress loading during selection
   */
  bool m_inSelection;

  /**
   * This signals whether a payee is being edited
   **/
  bool m_payeeInEditing;

  AccountNamesFilterProxyModel *m_filterProxyModel;
};

#endif
