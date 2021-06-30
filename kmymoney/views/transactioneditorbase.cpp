/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "transactioneditorbase.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractButton>
#include <QKeyEvent>
#include <QModelIndex>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"
#include "splitmodel.h"

class TransactionEditorBase::Private
{
public:
    Private()
        : cancelButton(nullptr)
        , enterButton(nullptr)
    {
    }

    QAbstractButton* cancelButton;
    QAbstractButton* enterButton;
};

TransactionEditorBase::TransactionEditorBase(QWidget* parent, const QString& accountId)
    : QFrame(parent, Qt::FramelessWindowHint /* | Qt::X11BypassWindowManagerHint */)
    , d(new TransactionEditorBase::Private)
{
    Q_UNUSED(accountId)
}

TransactionEditorBase::~TransactionEditorBase()
{
}

void TransactionEditorBase::addSplitsFromModel(QList<MyMoneySplit>& splits, const SplitModel* model) const
{
    const auto rows = model->rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = model->index(row, 0);
        MyMoneySplit s;
        s.setNumber(idx.data(eMyMoney::Model::SplitNumberRole).toString());
        s.setMemo(idx.data(eMyMoney::Model::SplitMemoRole).toString());
        s.setAccountId(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
        s.setShares(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
        s.setValue(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>());
        s.setCostCenterId(idx.data(eMyMoney::Model::SplitCostCenterIdRole).toString());
        s.setPayeeId(idx.data(eMyMoney::Model::SplitPayeeIdRole).toString());
        s.setTagIdList(idx.data(eMyMoney::Model::SplitTagIdRole).toStringList());
        splits.append(s);
    }
}

void TransactionEditorBase::addSplitsFromModel(MyMoneyTransaction& t, const SplitModel* model) const
{
    // now update and add what we have in the model
    const auto rows = model->rowCount();
    for (int row = 0; row < rows; ++row) {
        const auto idx = model->index(row, 0);
        MyMoneySplit s;
        const QString splitId = idx.data(eMyMoney::Model::IdRole).toString();
        // Extract the split from the transaction if
        // it already exists. Otherwise it remains
        // an empty split and will be added later.
        try {
            s = t.splitById(splitId);
        } catch(const MyMoneyException&) {
        }
        s.setNumber(idx.data(eMyMoney::Model::SplitNumberRole).toString());
        s.setMemo(idx.data(eMyMoney::Model::SplitMemoRole).toString());
        s.setAccountId(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
        s.setShares(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
        s.setValue(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>());
        s.setCostCenterId(idx.data(eMyMoney::Model::SplitCostCenterIdRole).toString());
        s.setPayeeId(idx.data(eMyMoney::Model::SplitPayeeIdRole).toString());
        s.setTagIdList(idx.data(eMyMoney::Model::SplitTagIdRole).toStringList());

        if (s.id().isEmpty()) {
            t.addSplit(s);
        } else {
            t.modifySplit(s);
        }
    }
}

void TransactionEditorBase::keyPressEvent(QKeyEvent* e)
{
    if (d->cancelButton && d->enterButton) {
        if (!e->modifiers() || (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter)) {
            if (d->enterButton->isVisible()) {
                switch (e->key()) {
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    if (focusWidget() == d->cancelButton) {
                        reject();
                    } else {
                        if (d->enterButton->isEnabled()) {
                            // move focus to enter button which
                            // triggers update of widgets
                            d->enterButton->setFocus();
                            d->enterButton->click();
                        }
                        return;
                    }
                    break;

                case Qt::Key_Escape:
                    reject();
                    break;

                default:
                    e->ignore();
                    break;
                }
            } else {
                e->ignore();
            }
        } else {
            e->ignore();
        }
    }
}

void TransactionEditorBase::reject()
{
    emit done();
}

void TransactionEditorBase::setCancelButton(QAbstractButton* button)
{
    d->cancelButton = button;
}

void TransactionEditorBase::setEnterButton(QAbstractButton* button)
{
    d->enterButton = button;
}
