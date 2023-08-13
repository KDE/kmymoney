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
#include "widgethintframe.h"

class TransactionEditorBase::Private
{
public:
    Private()
        : cancelButton(nullptr)
        , enterButton(nullptr)
        , focusFrame(nullptr)
        , readOnly(false)
    {
    }

    QAbstractButton* cancelButton;
    QAbstractButton* enterButton;
    WidgetHintFrame* focusFrame;
    bool readOnly;
};

TransactionEditorBase::TransactionEditorBase(QWidget* parent, const QString& accountId)
    : QWidget(parent)
    , d(new TransactionEditorBase::Private)
{
    Q_UNUSED(accountId)
    d->focusFrame = new WidgetHintFrame(this, WidgetHintFrame::Focus);
    WidgetHintFrame::show(this);
    connect(d->focusFrame, &QObject::destroyed, this, [&]() {
        d->focusFrame = nullptr;
    });
}

TransactionEditorBase::~TransactionEditorBase()
{
    if (d->focusFrame) {
        d->focusFrame->deleteLater();
    }
}

TransactionEditorBase::QWidget* TransactionEditorBase::focusFrame() const
{
    return d->focusFrame;
}

bool TransactionEditorBase::focusNextPrevChild(bool next)
{
    auto rc = KMyMoneyUtils::tabFocusHelper(this, next);

    if (rc == false) {
        rc = QWidget::focusNextPrevChild(next);
    }
    return rc;
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
    Q_EMIT done();
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

QStringList TransactionEditorBase::tabOrder(const QString& name, const QStringList& defaultTabOrder) const
{
    return KMyMoneyUtils::tabOrder(name, defaultTabOrder);
}

void TransactionEditorBase::setupTabOrder(const QStringList& tabOrder)
{
    KMyMoneyUtils::setupTabOrder(this, tabOrder);
}

void TransactionEditorBase::storeTabOrder(const QString& name, const QStringList& tabOrder)
{
    KMyMoneyUtils::storeTabOrder(name, tabOrder);
}

WidgetHintFrameCollection* TransactionEditorBase::widgetHintFrameCollection() const
{
    return nullptr;
}

void TransactionEditorBase::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    if (d->focusFrame) {
        d->focusFrame->setVisible(visible);
    }
}

void TransactionEditorBase::setAmountPlaceHolderText(const QAbstractItemModel* model)
{
    Q_UNUSED(model)
}

bool TransactionEditorBase::setSelectedJournalEntryIds(const QStringList& selectedJournalEntryIds)
{
    Q_UNUSED(selectedJournalEntryIds)
    return true;
}

QString TransactionEditorBase::errorMessage() const
{
    return {};
}
