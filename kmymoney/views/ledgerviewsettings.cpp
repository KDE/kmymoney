/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgerviewsettings.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class LedgerViewSettingsPrivate
{
public:
    LedgerViewSettingsPrivate()
        : m_showLedgerLens(false)
        , m_showTransactionDetails(false)
        , m_showAllSplits(false)
        , m_hideReconciledTransactions(false)
    {
    }

    QDate m_hideTransactionsBefore;
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
        emit settingsChanged();
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
        emit settingsChanged();
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
        emit settingsChanged();
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
        emit settingsChanged();
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
        emit settingsChanged();
    }
}
