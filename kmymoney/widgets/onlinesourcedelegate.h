/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINESOURCEDELEGATE_H
#define ONLINESOURCEDELEGATE_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QComboBox>
#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMM_BASE_WIDGETS_EXPORT OnlineSourceDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit OnlineSourceDelegate(QObject* parent = nullptr);
    ~OnlineSourceDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};

#endif // ONLINESOURCEDELEGATE_H
