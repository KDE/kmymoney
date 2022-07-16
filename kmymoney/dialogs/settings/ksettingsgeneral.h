/*
    SPDX-FileCopyrightText: 2005-2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSETTINGSGENERAL_H
#define KSETTINGSGENERAL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KSettingsGeneralPrivate;
class KSettingsGeneral : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KSettingsGeneral)

public:
    explicit KSettingsGeneral(QWidget* parent = nullptr);
    ~KSettingsGeneral();

protected Q_SLOTS:
    void slotChooseLogPath();
    void slotUpdateLogTypes();

protected:
    void showEvent(QShowEvent* event) override;

Q_SIGNALS:
    void haveValidInput(bool valid);

private:
    KSettingsGeneralPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KSettingsGeneral)
};
#endif

