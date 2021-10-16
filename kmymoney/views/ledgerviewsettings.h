/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERVIEWSETTINGS_H
#define LEDGERVIEWSETTINGS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class LedgerViewSettingsPrivate;
class LedgerViewSettings : public QObject
{
    Q_OBJECT

public:
    static LedgerViewSettings* instance();
    ~LedgerViewSettings();

    bool showLedgerLens() const;
    bool showTransactionDetails() const;
    bool showAllSplits() const;

public Q_SLOTS:
    void setShowLedgerLens(bool show);
    void setShowTransactionDetails(bool show);
    void setShowAllSplits(bool show);

Q_SIGNALS:
    void settingsChanged();

private:
    LedgerViewSettings();
    LedgerViewSettingsPrivate* const d;

    Q_DISABLE_COPY(LedgerViewSettings)
};

#endif // LEDGERVIEWSETTINGS_H
