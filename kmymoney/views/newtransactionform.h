/*
    SPDX-FileCopyrightText: 2015 Thomas Baumgart <Thomas Baumgart <tbaumgart@kde.org>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NEWTRANSACTIONFORM_H
#define NEWTRANSACTIONFORM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QModelIndex;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


class NewTransactionForm : public QFrame
{
    Q_OBJECT
public:
    explicit NewTransactionForm(QWidget* parent = 0);
    virtual ~NewTransactionForm();

public Q_SLOTS:
    void showTransaction(const QString& transactionSplitId);
    void modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

private:
    class Private;
    Private * const d;
};

#endif // NEWTRANSACTIONFORM_H

