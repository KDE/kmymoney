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
class QAbstractItemModel;

class TransactionEditorBase : public QFrame
{
    Q_OBJECT

public:
    explicit TransactionEditorBase(QWidget* parent = 0, const QString& accountId = QString());
    virtual ~TransactionEditorBase();

    virtual bool accepted() const = 0;
    virtual void loadTransaction(const QModelIndex& index) = 0;
    virtual void saveTransaction() = 0;
    virtual void setAmountPlaceHolderText(const QAbstractItemModel* model)
    {
        Q_UNUSED(model)
    }

protected:
    void addSplitsFromModel(MyMoneyTransaction& t, const SplitModel* model) const;
    void addSplitsFromModel(QList<MyMoneySplit>& splits, const SplitModel* model) const;

Q_SIGNALS:
    void done();
    void editorLayoutChanged();

};

#endif // TRANSACTIONEDITORBASE_H

