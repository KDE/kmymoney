/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmmstyleditemdelegate.h"

KMMStyledItemDelegate::KMMStyledItemDelegate(QWidget* parent)
  : QStyledItemDelegate(parent)
{
}

KMMStyledItemDelegate::~KMMStyledItemDelegate()
{
}

bool KMMStyledItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool KMMStyledItemDelegate::eventFilter(QObject* watched, QEvent* event)
{
  return QStyledItemDelegate::eventFilter(watched, event);
}
