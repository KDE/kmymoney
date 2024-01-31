/*
 *    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "ktagcontainer.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "tagcreator.h"

TagCreator::TagCreator(QObject* parent)
    : QObject(parent)
    , m_tagContainer(nullptr)
{
}

void TagCreator::addButton(QAbstractButton* button)
{
    m_buttons.append(button);
}

void TagCreator::setTagContainer(KTagContainer* tagContainer)
{
    m_tagContainer = tagContainer;
}

void TagCreator::createTag()
{
    // keep our own copy just in case it
    // gets overwritten in the combobox while we
    // are active
    m_name = m_tagContainer->tagCombo()->currentText();

    QTimer::singleShot(150, this, [&]() {
        // wait another round if any of the buttons is pressed
        if (std::any_of(m_buttons.constBegin(), m_buttons.constEnd(), [&](QAbstractButton* b) -> bool {
                return b->isDown();
            })) {
            createTag();
            return;
        }

        qDebug() << "createTag" << m_name;
        QString tagId;
        bool ok;

        MyMoneyFileTransaction ft(i18nc("Undo action description", "Create tag"), false);
        std::tie(ok, tagId) = KMyMoneyUtils::newTag(m_name);
        if (!ok) {
            m_tagContainer->tagCombo()->clearEditText();
            m_tagContainer->tagCombo()->setCurrentIndex(-1);
            m_tagContainer->setFocus();
        } else {
            ft.commit();
            const auto index = m_tagContainer->tagCombo()->findData(tagId, eMyMoney::Model::IdRole);
            if (index != -1) {
                m_tagContainer->tagCombo()->setCurrentIndex(-1);
                m_tagContainer->addTagWidget(tagId);
                auto widget = m_tagContainer->nextInFocusChain();
                while ((widget == m_tagContainer->tagCombo()) || (widget == m_tagContainer->tagCombo()->lineEdit())) {
                    widget = widget->nextInFocusChain();
                }
                widget->setFocus();
            } else {
                m_tagContainer->tagCombo()->clearEditText();
                m_tagContainer->tagCombo()->setCurrentIndex(-1);
                m_tagContainer->tagCombo()->setFocus();
            }
        }

        // suicide, we're done
        deleteLater();
    });
}
