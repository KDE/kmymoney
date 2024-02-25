/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSETTINGSONLINEQUOTES_H
#define KSETTINGSONLINEQUOTES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QListWidgetItem;

class KSettingsOnlineQuotesPrivate;
class KSettingsOnlineQuotes : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KSettingsOnlineQuotes)

public:
    explicit KSettingsOnlineQuotes(QWidget* parent = nullptr);
    ~KSettingsOnlineQuotes();

public Q_SLOTS:
    void saveSettings();
    void resetSettings();

protected Q_SLOTS:
    // void slotDumpCSVProfile();

Q_SIGNALS:
    void settingsChanged(bool hasChanges);

private:
    KSettingsOnlineQuotesPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KSettingsOnlineQuotes)
};

#endif
