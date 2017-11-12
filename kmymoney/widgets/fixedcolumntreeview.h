/*
 * Copyright 2014  Cristian Oneț <onet.cristian@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FIXEDCOLUMNTREEVIEW_H
#define FIXEDCOLUMNTREEVIEW_H

#include <QTreeView>

/**
  * This tree view should be used together with a source
  * tree view to obtain a view that has the first column
  * fixed just like in the "Frozen Column" Qt example.
  *
  * @usage Just create this view by passing the source view
  *        as a parameter. If you intend to change the model
  *        of the source view after this view was attached call
  *        sourceModelUpdated().
  */
class FixedColumnTreeView : public QTreeView
{
  Q_OBJECT

public:
  explicit FixedColumnTreeView(QTreeView *parent);
  ~FixedColumnTreeView();

public slots:
  /**
    * Call this if the model of the source view is changed
    * after the fixed column view was attached.
    */
  void sourceModelUpdated();

protected:
  bool viewportEvent(QEvent *event) override;
  bool eventFilter(QObject *object, QEvent *event) override;

protected slots:
  void onExpanded(const QModelIndex& index);
  void onCollapsed(const QModelIndex& index);
  void updateSectionWidth(int logicalIndex, int, int newSize);
  void updateSortIndicator(int logicalIndex, Qt::SortOrder order);

private:
  struct Private;
  Private * const d;
};

#endif // FIXEDCOLUMNTREEVIEW_H
