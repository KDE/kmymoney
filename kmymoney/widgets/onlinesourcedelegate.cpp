/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinesourcedelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmmonlinequotesprofilemanager.h"
#include <alkimia/alkonlinequotesource.h>

OnlineSourceDelegate::OnlineSourceDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

OnlineSourceDelegate::~OnlineSourceDelegate()
{
}

QWidget* OnlineSourceDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    auto* editor = new KComboBox(parent);

    KMMOnlineQuotesProfileManager& manager(KMMOnlineQuotesProfileManager::instance());
    const auto quoteProfile(manager.profile(QLatin1String("kmymoney5")));
    if (quoteProfile) {
        const auto quoteSourceNames = quoteProfile->quoteSources();
        // add only quote sources that are eligible for currency conversions
        for (const auto& quoteSourceName : quoteSourceNames) {
            const auto quoteSource = AlkOnlineQuoteSource(quoteSourceName, quoteProfile);
            if (quoteSource.requiresTwoIdentifier()) {
                editor->addItem(quoteSourceName);
            }
        }
    }
    editor->setEnabled(quoteProfile != nullptr);
    editor->model()->sort(0);

    return editor;
}

void OnlineSourceDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    QString currentText = index.data(Qt::EditRole).toString();
    comboBox->setCurrentText(currentText);
}

void OnlineSourceDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}
