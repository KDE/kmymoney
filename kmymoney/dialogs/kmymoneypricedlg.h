/*
    SPDX-FileCopyrightText: 2004-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYPRICEDLG_H
#define KMYMONEYPRICEDLG_H

#include "kmm_extended_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyPriceDlgPrivate;
class KMM_EXTENDED_DIALOGS_EXPORT KMyMoneyPriceDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KMyMoneyPriceDlg)

public:
    explicit KMyMoneyPriceDlg(QWidget* parent);
    ~KMyMoneyPriceDlg();

    void setSearchShortcut(const QKeySequence& shortcut);

protected Q_SLOTS:
    void slotNewPrice();
    void slotDeletePrice();
    void slotEditPrice();
    void slotOnlinePriceUpdate();
    void slotShowPriceMenu(const QPoint& p);

private:
    KMyMoneyPriceDlgPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KMyMoneyPriceDlg)
};

#endif // KMYMONEYPRICEDLG_H
