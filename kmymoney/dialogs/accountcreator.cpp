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
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountcreator.h"
#include "accountsmodel.h"
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
        if (std::any_of(m_buttons.cbegin(), m_buttons.cend(), [&](QAbstractButton* b) -> bool {
                return b->isDown();
            })) {
            createAccount();
            return;
        }

        // determine the top parent account
        MyMoneyAccount parent;
        if (m_accountType == eMyMoney::Account::Type::Asset) {
            parent = MyMoneyFile::instance()->asset();
        } else if (m_accountType == eMyMoney::Account::Type::Liability) {
            parent = MyMoneyFile::instance()->liability();
        } else if (m_accountType == eMyMoney::Account::Type::Expense) {
            parent = MyMoneyFile::instance()->expense();
        } else if (m_accountType == eMyMoney::Account::Type::Income) {
            parent = MyMoneyFile::instance()->income();
        }

        MyMoneyAccount account;

        const auto fullName = m_comboBox->currentText();
        QString newAccountName;
        if (fullName.contains(MyMoneyAccount::accountSeparator())) {
            const auto file = MyMoneyFile::instance();
            const auto lastSeparatorPos = fullName.lastIndexOf(MyMoneyAccount::accountSeparator());
            newAccountName = fullName.mid(lastSeparatorPos + 1);
            const auto newParentName = fullName.left(lastSeparatorPos);
            const auto topLevelAccountName = parent.name();
            const auto idx = file->accountsModel()->indexById(parent.id());

            // search in the preferred sub-tree based on amount entered
            parent = file->accountsModel()->itemByName(newParentName, idx);
            // if not found in the preferred sub-tree, we search all
            if (parent.id().isEmpty()) {
                parent = file->accountsModel()->itemByName(newParentName, QModelIndex());
            }
            // if still not found, we inform the user
            if (parent.id().isEmpty()) {
                KMessageBox::error(m_comboBox,
                                   i18nc("@info %1 selected parent, %2 top level account",
                                         "The selected parent account <b>%1</b> does not exist in the <b>%2</b> hierarchy.",
                                         newParentName,
                                         topLevelAccountName),
                                   i18n("Account/category creation problem"));
            }
        } else {
            newAccountName = m_comboBox->currentText();
        }

        if (!parent.id().isEmpty()) {
            account.setName(newAccountName);

            const bool isAccount = (m_accountType == eMyMoney::Account::Type::Asset) || (m_accountType == eMyMoney::Account::Type::Liability);
            const auto creator = isAccount ? &KNewAccountDlg::newAccount : &KNewAccountDlg::newCategory;
            const QString undoAction = isAccount ? i18nc("Create undo action", "Create account") : i18nc("Create undo action", "Create category");

            MyMoneyFileTransaction ft(undoAction, false);
            creator(account, parent);

            // if the creation worked, we move on
            if (!account.id().isEmpty()) {
                ft.commit();
                m_comboBox->setSelected(account.id());
                auto widget = m_comboBox->nextInFocusChain();
                widget->setFocus();
            }
        }

        // in case the account/category was not created
        // we set the focus back on the widget we came from
        if (account.id().isEmpty()) {
            m_comboBox->setSelected(QString());
            m_comboBox->clearSelection();
            m_comboBox->setFocus();
        }

        // suicide, we're done
        deleteLater();
    });
}
