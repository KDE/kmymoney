/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "transactioneditorbase.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractButton>
#include <QCompleter>
#include <QKeyEvent>
#include <QLineEdit>
#include <QModelIndex>
#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountcreator.h"
#include "accountsmodel.h"
#include "creditdebitedit.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneyutils.h"
#include "mymoneyfile.h"
#include "payeecreator.h"
#include "widgethintframe.h"

class TransactionEditorBase::Private
{
public:
    Private()
        : cancelButton(nullptr)
        , enterButton(nullptr)
        , focusFrame(nullptr)
        , readOnly(false)
        , accepted(false)
    {
    }

    QAbstractButton* cancelButton;
    QAbstractButton* enterButton;
    WidgetHintFrame* focusFrame;
    bool readOnly;
    bool accepted;
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

void TransactionEditorBase::acceptEdit()
{
    if (isTransactionDataValid()) {
        d->accepted = true;
        Q_EMIT done();
    }
}

bool TransactionEditorBase::accepted() const
{
    return d->accepted;
}

bool TransactionEditorBase::needCreateCategory(KMyMoneyAccountCombo* comboBox) const
{
    if (comboBox != nullptr) {
        if (!comboBox->popup()->isVisible() && !comboBox->currentText().isEmpty() && !comboBox->lineEdit()->isReadOnly()) {
            const auto accountId = comboBox->getSelected();
            const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
            if (!accountIdx.isValid() || accountIdx.data(eMyMoney::Model::AccountFullNameRole).toString().compare(comboBox->currentText())) {
                return true;
            }
        }
    }
    return false;
}

void TransactionEditorBase::createCategory(KMyMoneyAccountCombo* comboBox, eMyMoney::Account::Type type)
{
    auto creator = new AccountCreator(this);
    creator->setComboBox(comboBox);
    creator->addButton(d->cancelButton);
    creator->addButton(d->enterButton);
    creator->setAccountType(type);
    creator->createAccount();
}

eMyMoney::Account::Type TransactionEditorBase::defaultCategoryType(CreditDebitEdit* valueWidget) const
{
    eMyMoney::Account::Type type = eMyMoney::Account::Type::Unknown;
    if (valueWidget->haveValue()) {
        if (valueWidget->value().isPositive()) {
            type = eMyMoney::Account::Type::Income;
        } else {
            type = eMyMoney::Account::Type::Expense;
        }
    }
    return type;
}

bool TransactionEditorBase::needCreatePayee(QComboBox* comboBox) const
{
    if (comboBox != nullptr) {
        // set case sensitivity so that a payee with the same spelling
        // but different case can be created and is not found by accident
        // inside the Qt logic (see QComboBoxPrivate::_q_editingFinished())
        comboBox->completer()->setCaseSensitivity(Qt::CaseSensitive);
        if (!comboBox->currentText().isEmpty()) {
            const auto index(comboBox->findText(comboBox->currentText()));
            if (index == -1) {
                return true;
            }
        }
    }
    return false;
}

void TransactionEditorBase::createPayee(QComboBox* comboBox)
{
    auto creator = new PayeeCreator(this);
    creator->setComboBox(comboBox);
    creator->addButton(d->cancelButton);
    creator->addButton(d->enterButton);
    creator->createPayee();
}
