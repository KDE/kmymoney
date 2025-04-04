/*
 *    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractButton>
#include <QComboBox>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "investmentcreator.h"
#include "kmymoneyutils.h"
#include "knewinvestmentwizard.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"

InvestmentCreator::InvestmentCreator(QObject* parent)
    : QObject(parent)
    , m_comboBox(nullptr)
{
}

void InvestmentCreator::addButton(QAbstractButton* button)
{
    m_buttons.append(button);
}

void InvestmentCreator::setComboBox(QComboBox* cb)
{
    m_comboBox = cb;
}

void InvestmentCreator::setInvestmentAccount(const MyMoneyAccount& parentAccount)
{
    m_parentAccount = parentAccount;
}

void InvestmentCreator::createInvestment()
{
    // keep our own copy just in case it
    // gets overwritten in the combobox while we
    // are active
    m_name = m_comboBox->currentText();

    QTimer::singleShot(150, this, [&]() {
        // wait another round if any of the buttons is pressed
        if (std::any_of(m_buttons.cbegin(), m_buttons.cend(), [&](QAbstractButton* b) -> bool {
                return b->isDown();
            })) {
            createInvestment();
            return;
        }

        qDebug() << "createInvestment" << m_name;

        MyMoneyFileTransaction ft(i18nc("Undo action description", "Create investment"), false);

        const auto newAccountId = KNewInvestmentWizard::newInvestment(m_parentAccount, m_name);
        if (newAccountId.isEmpty()) {
            m_comboBox->clearEditText();
            m_comboBox->setCurrentIndex(-1);
            m_comboBox->setFocus();
        } else {
            ft.commit();
            const auto index = m_comboBox->findData(newAccountId, eMyMoney::Model::IdRole);
            if (index != -1) {
                m_comboBox->setCurrentIndex(index);
                auto widget = m_comboBox->nextInFocusChain();
                widget->setFocus();
            } else {
                m_comboBox->clearEditText();
                m_comboBox->setCurrentIndex(-1);
                m_comboBox->setFocus();
            }
        }

        // suicide, we're done
        deleteLater();
    });
}
