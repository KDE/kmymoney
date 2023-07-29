/*
 *    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractButton>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountcreator.h"
#include "kmymoneyaccountcombo.h"
#include "knewaccountdlg.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"

AccountCreator::AccountCreator(QObject* parent)
    : QObject(parent)
    , m_comboBox(nullptr)
    , m_accountType(eMyMoney::Account::Type::Unknown)
{
}

void AccountCreator::addButton(QAbstractButton* button)
{
    m_buttons.append(button);
}

void AccountCreator::setAccountType(eMyMoney::Account::Type type)
{
    m_accountType = type;
}

void AccountCreator::setComboBox(KMyMoneyAccountCombo* cb)
{
    m_comboBox = cb;
}

void AccountCreator::createAccount()
{
    QTimer::singleShot(150, this, [&]() {
        // wait another round if any of the buttons is pressed
        for (const auto& button : m_buttons) {
            if (button->isDown()) {
                createAccount();
                return;
            }
        }

        MyMoneyAccount parent;
        MyMoneyAccount account;

        account.setName(m_comboBox->currentText());

        if (m_accountType == eMyMoney::Account::Type::Asset) {
            parent = MyMoneyFile::instance()->asset();
        } else if (m_accountType == eMyMoney::Account::Type::Liability) {
            parent = MyMoneyFile::instance()->liability();
        } else if (m_accountType == eMyMoney::Account::Type::Expense) {
            parent = MyMoneyFile::instance()->expense();
        } else if (m_accountType == eMyMoney::Account::Type::Income) {
            parent = MyMoneyFile::instance()->income();
        }

        const bool isAccount = (m_accountType == eMyMoney::Account::Type::Asset) || (m_accountType == eMyMoney::Account::Type::Liability);
        const auto creator = isAccount ? &KNewAccountDlg::newAccount : &KNewAccountDlg::newCategory;
        const QString undoAction = isAccount ? i18nc("Create undo action", "Create account") : i18nc("Create undo action", "Create category");

        MyMoneyFileTransaction ft(undoAction, false);
        creator(account, parent);

        if (account.id().isEmpty()) {
            m_comboBox->setSelected(QString());
            m_comboBox->clearSelection();
            m_comboBox->setFocus();
        } else {
            ft.commit();
            m_comboBox->setSelected(account.id());
            auto widget = m_comboBox->nextInFocusChain();
            widget->setFocus();
        }

        // suicide, we're done
        deleteLater();
    });
}
