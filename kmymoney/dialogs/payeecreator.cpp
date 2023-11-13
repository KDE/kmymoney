/*
 *    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
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

#include "kmymoneyutils.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "payeecreator.h"

PayeeCreator::PayeeCreator(QObject* parent)
    : QObject(parent)
    , m_comboBox(nullptr)
{
}

void PayeeCreator::addButton(QAbstractButton* button)
{
    m_buttons.append(button);
}

void PayeeCreator::setComboBox(QComboBox* cb)
{
    m_comboBox = cb;
}

void PayeeCreator::createPayee()
{
    // keep our own copy just in case it
    // gets overwritten in the combobox while we
    // are active
    m_name = m_comboBox->currentText();

    QTimer::singleShot(150, this, [&]() {
        // wait another round if any of the buttons is pressed
        if (std::any_of(m_buttons.constBegin(), m_buttons.constEnd(), [&](QAbstractButton* b) -> bool {
                return b->isDown();
            })) {
            createPayee();
            return;
        }

        qDebug() << "createPayee" << m_name;
        QString payeeId;
        bool ok;

        MyMoneyFileTransaction ft(i18nc("Undo action description", "Create payee"), false);
        std::tie(ok, payeeId) = KMyMoneyUtils::newPayee(m_name);
        if (!ok) {
            m_comboBox->clearEditText();
            m_comboBox->setCurrentIndex(-1);
            m_comboBox->setFocus();
        } else {
            ft.commit();
            const auto index = m_comboBox->findData(payeeId, eMyMoney::Model::IdRole);
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
