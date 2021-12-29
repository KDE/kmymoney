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

#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"

class TransactionEditorBase::Private
{
public:
    Private()
        : cancelButton(nullptr)
        , enterButton(nullptr)
        , readOnly(false)
    {
    }

    QAbstractButton* cancelButton;
    QAbstractButton* enterButton;
    bool readOnly;
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
                        if (d->enterButton->isEnabled() && !d->readOnly) {
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

void TransactionEditorBase::setReadOnly(bool readOnly)
{
    d->readOnly = readOnly;
}

bool TransactionEditorBase::isReadOnly() const
{
    return d->readOnly;
}

void TransactionEditorBase::setupTabOrder(const QString& name, const QStringList& defaultTabOrder)
{
    KMyMoneyUtils::setupTabOrder(this, name, defaultTabOrder);
}
