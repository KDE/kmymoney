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

#include <QString>
#include <QModelIndexList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

struct ContactData;
class MyMoneyContact;
class MyMoneyPayee;
class QItemSelection;

/**
  * @author Michael Edwardes, Thomas Baumgart
  */

/**
  * This class represents an item in the payees list view.
  */
class KPayeesViewPrivate;
class KPayeesView : public KMyMoneyViewBase
{
  Q_DECLARE_PRIVATE(KPayeesView)
  Q_OBJECT

public:
  explicit KPayeesView(QWidget *parent = nullptr);
  ~KPayeesView() override;

  void executeCustomAction(eView::Action action) override;

public Q_SLOTS:
  void slotHelp();

  /**
   * @brief proxy slot to close a model based on file open/close
   */
  void slotClosePayeeIdentifierSource();

  void slotSelectByVariant(const QVariantList& variant, eView::Intent intent) override;

  void updateActions(const SelectedObjects& selections) override;

Q_SIGNALS:
  void transactionSelected(const QString& accountId, const QString& transactionId);

protected:
  void showEvent(QShowEvent* event) override;

private Q_SLOTS:
  /**
   * This slot is called whenever the selection in m_payeesList
   * is about to change.
   */
  void slotPayeeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  /**
    * This slot marks the current selected payee as modified (dirty).
    */
  void slotPayeeDataChanged();
  void slotKeyListChanged();

  void slotRenameSinglePayee(const QModelIndex& idx, const QVariant& value);

  /**
    * Updates the payee data in m_payee from the information in the
    * payee information widget.
    */
  void slotUpdatePayee();

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

  void slotModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

};

#endif
