/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgerviewsettings.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QHash>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"

class LedgerViewSettingsPrivate
{
public:
    LedgerViewSettingsPrivate()
        : m_showLedgerLens(false)
        , m_showTransactionDetails(false)
        , m_showAllSplits(false)
        , m_hideReconciledTransactions(false)
    {
        m_settingsChangedTimer.setSingleShot(true);
        m_settingsChangedTimer.setInterval(20);
    }

    void settingsChanged()
    {
        m_settingsChangedTimer.start();
    }

    QDate m_hideTransactionsBefore;
    QHash<LedgerViewSettings::SortOrderType, LedgerSortOrder> m_sortOrder;
    QTimer m_settingsChangedTimer;
    bool m_showLedgerLens;
    bool m_showTransactionDetails;
    bool m_showAllSplits;
    bool m_hideReconciledTransactions;
};

LedgerViewSettings* LedgerViewSettings::instance()
{
    static LedgerViewSettings* m_instance = nullptr;

    if (m_instance == nullptr) {
        m_instance = new LedgerViewSettings;
    }
    return m_instance;
}

LedgerViewSettings::LedgerViewSettings()
    : QObject(nullptr)
    , d(new LedgerViewSettingsPrivate)
{
    connect(&d->m_settingsChangedTimer, &QTimer::timeout, this, [&]() {
        Q_EMIT settingsChanged();
    });
}

LedgerViewSettings::~LedgerViewSettings()
{
    delete d;
}

bool LedgerViewSettings::showLedgerLens() const
{
    return d->m_showLedgerLens;
}

void LedgerViewSettings::setShowLedgerLens(bool show)
{
    if (d->m_showLedgerLens != show) {
        d->m_showLedgerLens = show;
        d->settingsChanged();
    }
}

bool LedgerViewSettings::showTransactionDetails() const
{
    return d->m_showTransactionDetails;
}

void LedgerViewSettings::setShowTransactionDetails(bool show)
{
    if (d->m_showTransactionDetails != show) {
        d->m_showTransactionDetails = show;
        d->settingsChanged();
    }
}

bool LedgerViewSettings::showAllSplits() const
{
    return d->m_showAllSplits;
}

void LedgerViewSettings::setShowAllSplits(bool show)
{
    if (d->m_showAllSplits != show) {
        d->m_showAllSplits = show;
        d->settingsChanged();
    }
}

bool LedgerViewSettings::hideReconciledTransactions() const
{
    return d->m_hideReconciledTransactions;
}

void LedgerViewSettings::setHideReconciledTransactions(bool hide)
{
    if (d->m_hideReconciledTransactions != hide) {
        d->m_hideReconciledTransactions = hide;
        d->settingsChanged();
    }
}

QDate LedgerViewSettings::hideTransactionsBefore() const
{
    return d->m_hideTransactionsBefore;
}

void LedgerViewSettings::setHideTransactionsBefore(const QDate& date)
{
    if (d->m_hideTransactionsBefore != date) {
        d->m_hideTransactionsBefore = date;
        d->settingsChanged();
    }
}

LedgerSortOrder LedgerViewSettings::sortOrder(SortOrderType type) const
{
    return d->m_sortOrder.value(type, LedgerSortOrder());
}

void LedgerViewSettings::setSortOrder(LedgerViewSettings::SortOrderType type, const QString& sortOrder)
{
    LedgerSortOrder sortOrderItemList;
    sortOrderItemList.setSortOrder(sortOrder);
    if (d->m_sortOrder[type] != sortOrderItemList) {
        d->m_sortOrder[type] = sortOrderItemList;
        d->settingsChanged();
    }
}

void LedgerViewSettings::flushChanges()
{
    if (d->m_settingsChangedTimer.isActive()) {
        d->m_settingsChangedTimer.stop();
        Q_EMIT settingsChanged();
    }
}
