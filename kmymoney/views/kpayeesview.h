/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005 Andrea Nicolai <Andreas.Nicolai@gmx.net>
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    void aboutToShow() override;
    void aboutToHide() override;

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
