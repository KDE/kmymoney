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

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "mymoneyenums.h"
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
    QTimer::singleShot(150, this, [&]() {
        // wait another round if any of the buttons is pressed
        for (const auto& button : m_buttons) {
            if (button->isDown()) {
                createPayee();
                return;
            }
        }

        qDebug() << "createPayee" << m_comboBox->currentText();
        QString payeeId;
        bool ok;
        std::tie(ok, payeeId) = KMyMoneyUtils::newPayee(m_comboBox->currentText());
        if (!ok) {
            m_comboBox->clearEditText();
            m_comboBox->setCurrentIndex(-1);
            m_comboBox->setFocus();
        } else {
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
