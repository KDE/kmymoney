/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAPACCOUNTWIZARD_H
#define MAPACCOUNTWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizard>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class WoobInterface;

class MapAccountWizardPrivate;
class MapAccountWizard : public QWizard
{
    Q_OBJECT

public:
    explicit MapAccountWizard(QWidget *parent, WoobInterface *woob);
    ~MapAccountWizard();

    QString currentBackend() const;
    QString currentAccount() const;

private:
    Q_DECLARE_PRIVATE(MapAccountWizard)
    MapAccountWizardPrivate * const d_ptr;

private Q_SLOTS:
    void slotCheckNextButton(void);
    void slotNewPage(int id);
    void slotGotAccounts();
    void slotGotBackends();
};

#endif
