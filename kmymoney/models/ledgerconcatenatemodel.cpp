/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgerconcatenatemodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"

class LedgerConcatenateModelPrivate
{
public:
    LedgerConcatenateModelPrivate()
    {
    }
};

LedgerConcatenateModel::LedgerConcatenateModel(QObject* parent)
    : QConcatenateTablesProxyModel(parent)
    , d_ptr(new LedgerConcatenateModelPrivate)
{
    connect(MyMoneyFile::instance(), &MyMoneyFile::storageTransactionStarted, this, [&](bool journalBlocking) {
        if (journalBlocking) {
            blockSignals(true);
        }
    });

    connect(MyMoneyFile::instance(), &MyMoneyFile::storageTransactionEnded, this, [&](bool journalBlocking) {
        if (journalBlocking) {
            blockSignals(false);
            beginResetModel();
            endResetModel();
        }
    });
}

LedgerConcatenateModel::~LedgerConcatenateModel()
{
    Q_D(LedgerConcatenateModel);
    delete d;
}
