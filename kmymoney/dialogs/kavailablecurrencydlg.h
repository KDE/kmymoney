/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KAVAILABLECURRENCYDLG_H
#define KAVAILABLECURRENCYDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui
{
class KAvailableCurrencyDlg;
}

class KTreeWidgetSearchLineWidget;
class QTreeWidgetItem;
class KAvailableCurrencyDlg : public QDialog
{
    Q_OBJECT
public:
    explicit KAvailableCurrencyDlg(QWidget *parent = nullptr);
    ~KAvailableCurrencyDlg();

    QList<QTreeWidgetItem *> selectedItems() const;

protected Q_SLOTS:
    void slotLoadCurrencies();
    void slotItemSelectionChanged();

private:
    Ui::KAvailableCurrencyDlg*    ui;
    KTreeWidgetSearchLineWidget*  m_searchWidget;
};

#endif
