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

#ifndef COLUMNSELECTOR_H
#define COLUMNSELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QTreeView>
#include <QString>
#include <QPoint>

class ColumnSelectorPrivate;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
 * This class takes care of selecting the columns of a tree view and
 * store the selection in the global application configuration.
 *
 * The @a parent tree view must have a model attached that allows to
 * extract the maximum number of columns. The header names found in
 * the model are displayed in a menu when the user clicks on the
 * header with the right mouse button.
 *
 * @author Thomas Baumgart
 */
class ColumnSelector : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(ColumnSelector)

public:
  explicit ColumnSelector(QTreeView* parent, const QString& configGroupName);
  ~ColumnSelector();

protected Q_SLOT:
  void slotColumnsMenu(const QPoint);

private:
  ColumnSelectorPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(ColumnSelector)
};

#endif
