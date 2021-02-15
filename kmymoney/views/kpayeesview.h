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
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KPAYEESVIEW_H
#define KPAYEESVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class QListWidgetItem;
class KListWidgetSearchLine;
struct ContactData;
class MyMoneyContact;
class MyMoneyPayee;

/**
  * @author Michael Edwardes, Thomas Baumgart
  */

/**
  * This class represents an item in the payees list view.
  */
class KPayeesViewPrivate;
class KPayeesView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KPayeesView(QWidget *parent = nullptr);
  ~KPayeesView() override;

  void executeCustomAction(eView::Action action) override;
  void refresh();
  void updatePayeeActions(const QList<MyMoneyPayee>& payees);

public Q_SLOTS:
  void slotSelectPayeeAndTransaction(const QString& payeeId, const QString& accountId = QString(), const QString& transactionId = QString());
  void slotStartRename(QListWidgetItem*);
  void slotHelp();

  /**
   * @brief proxy slot to close a model based on file open/close
   */
  void slotClosePayeeIdentifierSource();

  void slotSelectByVariant(const QVariantList& variant, eView::Intent intent) override;

Q_SIGNALS:
  void transactionSelected(const QString& accountId, const QString& transactionId);
  void openContextMenu(const MyMoneyObject& obj);
  void selectObjects(const QList<MyMoneyPayee>& payees);

protected:
  void showEvent(QShowEvent* event) override;

private:
  Q_DECLARE_PRIVATE(KPayeesView)

private Q_SLOTS:
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
  void slotRenameSinglePayee(QListWidgetItem *p);

  /**
    * Updates the payee data in m_payee from the information in the
    * payee information widget.
    */
  void slotUpdatePayee();

  void slotSelectTransaction();

  void slotChangeFilter(int index);

  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * If @p points to a real payee item, emits openContextMenu().
    *
    * @param p position of the pointer device
    */
  void slotShowPayeesMenu(const QPoint& p);

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

  void slotNewPayee();
  void slotRenamePayee();
  void slotDeletePayee();
  void slotMergePayee();
};

#endif
