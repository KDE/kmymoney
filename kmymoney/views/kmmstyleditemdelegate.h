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

#ifndef KMMSTYLEDITEMDELEGATE_H
#define KMMSTYLEDITEMDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


class KMMStyledItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit KMMStyledItemDelegate(QWidget* parent = 0);
  virtual ~KMMStyledItemDelegate();

  /**
   * Make the editorEvent publically available
   */
  virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

  /**
   * Make the eventFilter publically available
   */
  bool eventFilter ( QObject * watched, QEvent * event ) override;
};

#endif // KMMSTYLEDITEMDELEGATE_H

