/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
