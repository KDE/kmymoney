/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RECONCILIATIONLEDGERVIEWPAGE_H
#define RECONCILIATIONLEDGERVIEWPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerviewpage.h"

class ReconciliationLedgerViewPage : public LedgerViewPage
{
    Q_OBJECT
public:
    explicit ReconciliationLedgerViewPage(QWidget* parent = 0, const QString& configGroupName = QString());
    virtual ~ReconciliationLedgerViewPage();

    /** overridden for internal reasons */
    void setAccount(const MyMoneyAccount& account) override;

    /// @copydoc LedgerViewPage::executeAction()
    bool executeAction(eMenu::Action action, const SelectedObjects& selections) override;

public Q_SLOTS:
    void updateSummaryInformation();

private:
    class Private;
};

#endif // RECONCILIATIONLEDGERVIEWPAGE_H
