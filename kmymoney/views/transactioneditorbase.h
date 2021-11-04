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
class QAbstractButton;
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

    virtual void setReadOnly(bool readOnly);
    bool isReadOnly() const;

protected:
    virtual void keyPressEvent(QKeyEvent* e) override;
    void setCancelButton(QAbstractButton* button);
    void setEnterButton(QAbstractButton* button);

protected Q_SLOTS:
    virtual void reject();

Q_SIGNALS:
    void done();
    void editorLayoutChanged();

private:
    class Private;
    QScopedPointer<Private> const d;
};

#endif // TRANSACTIONEDITORBASE_H

