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
#include <QPlainTextEdit>
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
#include "kcurrencyconverter.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "mymoneyfile.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "payeecreator.h"
#include "tagcreator.h"
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
        , enterMovesBetweenFields(false)
    {
    }

    QAbstractButton* cancelButton;
    QAbstractButton* enterButton;
    WidgetHintFrame* focusFrame;
    bool readOnly;
    bool accepted;
    bool enterMovesBetweenFields;
    KCurrencyConverter currencyConverter;
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
        if (!e->modifiers() || ((e->modifiers() & Qt::KeypadModifier) && (e->key() == Qt::Key_Enter))) {
            if (d->enterButton->isVisible()) {
                switch (e->key()) {
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    if (d->enterMovesBetweenFields) {
                        focusNextPrevChild(true);
                    } else {
                        processReturnKey();
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

bool TransactionEditorBase::needCreateObject(QComboBox* comboBox) const
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

void TransactionEditorBase::createTag(KTagContainer* tagContainer)
{
    auto creator = new TagCreator(this);
    creator->setTagContainer(tagContainer);
    creator->addButton(d->cancelButton);
    creator->addButton(d->enterButton);
    creator->createTag();
}

void TransactionEditorBase::processReturnKey()
{
    // delay the calls to reject, setFocus and click because this method
    // may be called from an event handler (see InvestTransactionEditor)
    if (focusWidget() == d->cancelButton) {
        QMetaObject::invokeMethod(this, &TransactionEditorBase::reject, Qt::QueuedConnection);
    } else {
        if (d->enterButton->isEnabled() && !d->readOnly) {
            // move focus to enter button which
            // triggers update of widgets
            QMetaObject::invokeMethod(d->enterButton, "setFocus", Qt::QueuedConnection);
            QMetaObject::invokeMethod(d->enterButton, "click", Qt::QueuedConnection);
        }
    }
}

KCurrencyConverter* TransactionEditorBase::currencyConverter() const
{
    return &d->currencyConverter;
}

void TransactionEditorBase::updateConversionRate(MultiCurrencyEdit* amountEdit) const
{
    const auto rate = d->currencyConverter.updateRate(amountEdit, postDate());
    const auto state = amountEdit->displayState();
    switch (state) {
    case MultiCurrencyEdit::DisplayShares:
        amountEdit->setValue(amountEdit->shares() * rate);
        amountEdit->setShares(amountEdit->shares());
        break;
    case MultiCurrencyEdit::DisplayValue:
        amountEdit->setValue(amountEdit->value());
        amountEdit->setShares(amountEdit->value() / rate);
        break;
    }
}

void TransactionEditorBase::slotSettingsChanged()
{
    d->enterMovesBetweenFields = KMyMoneySettings::enterMovesBetweenFields();
}

bool TransactionEditorBase::enterMovesBetweenFields() const
{
    return d->enterMovesBetweenFields;
}

bool TransactionEditorBase::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::KeyPress) {
        auto combobox = qobject_cast<QComboBox*>(o);
        auto textedit = qobject_cast<QPlainTextEdit*>(o);

        if (combobox != nullptr) {
            // filter out wheel events for combo boxes if the popup view is not visible
            if (combobox->view()) {
                if ((e->type() == QEvent::Wheel) && !combobox->view()->isVisible()) {
                    return true;
                }
            }

            // if it is a key press on a combobox
            const auto kev = static_cast<QKeyEvent*>(e);
            if (kev->modifiers() == Qt::NoModifier) {
                // no shift, ctrl, meta or alt
                if ((kev->key() == Qt::Key_Enter) || (kev->key() == Qt::Key_Return)) {
                    // and the return key and we use it to move between fields
                    if (d->enterMovesBetweenFields) {
                        focusNextChild();
                        return true;
                    }
                }
            }

        } else if (textedit != nullptr) {
            auto kev = static_cast<QKeyEvent*>(e);
            if ((kev->key() == Qt::Key_Enter) || (kev->key() == Qt::Key_Return)) {
                // and the return key and we use it to move between fields
                if (d->enterMovesBetweenFields) {
                    if (kev->modifiers() == Qt::AltModifier) {
                        textedit->insertPlainText(QLatin1String("\n"));
                        textedit->textCursor().movePosition(QTextCursor::Right);
                    } else {
                        focusNextChild();
                    }
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(o, e);
}

void TransactionEditorBase::changeEvent(QEvent* ev)
{
    if (ev->type() == QEvent::FontChange) {
        // since AmountEdit widgets internally use a stylesheet, they don't receive
        // the font update via the Qt event system. We simply update them here.
        const auto children = findChildren<AmountEdit*>();
        const auto newFont = font();
        std::for_each(children.cbegin(), children.cend(), [&](AmountEdit* w) {
            w->setFont(newFont);
        });
    }
    QWidget::changeEvent(ev);
}
