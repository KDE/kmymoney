/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgerviewsettings.h"

// ----------------------------------------------------------------------------
// QT Includes

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
    {
    }

    bool m_showLedgerLens;
    bool m_showTransactionDetails;
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
