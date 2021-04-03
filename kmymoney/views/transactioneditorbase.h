/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRANSACTIONEDITORBASE_H
#define TRANSACTIONEDITORBASE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QWidget;

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"

class MyMoneyTransaction;
class SplitModel;

class TransactionEditorBase : public QFrame
{
    Q_OBJECT

public:
    explicit TransactionEditorBase(QWidget* parent = 0, const QString& accountId = QString());
    virtual ~TransactionEditorBase();

    virtual bool accepted() const = 0;
    virtual void loadTransaction(const QModelIndex& index) = 0;
    virtual void saveTransaction() = 0;

protected:
    void addSplitsFromModel(MyMoneyTransaction& t, const SplitModel* model);
    void addSplitsFromModel(QList<MyMoneySplit>& splits, const SplitModel* model);

Q_SIGNALS:
    void done();
    void editorLayoutChanged();

};

#endif // TRANSACTIONEDITORBASE_H

