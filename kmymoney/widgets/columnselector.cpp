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

#include "columnselector.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QHeaderView>
#include <QMenu>
#include <QPoint>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

/// @todo add feature to group multiple column selectors for the same view
/// in different instances by group name.

class ColumnSelectorPrivate
{
  Q_DECLARE_PUBLIC(ColumnSelector)

public:
  ColumnSelectorPrivate(ColumnSelector* qq)
    : q_ptr(qq)
    , treeView(nullptr)
    , tableView(nullptr)
    , headerView(nullptr)
    , model(nullptr)
    // default is, that column 0 is always visible
    , alwaysVisibleColumns(QVector<int>({ 0 }))
    , storageOffset(0)
    , isInit(false)
    {
  }

  void setColumnHidden(int col, bool state)
  {
    if (treeView) {
      treeView->setColumnHidden(col, state);
    }
    else if (tableView) {
      tableView->setColumnHidden(col, state);
    }
  }

  bool isColumnHidden(int col)
  {
    if (treeView) {
      return treeView->isColumnHidden(col);
    }
    else if (tableView) {
      return tableView->isColumnHidden(col);
    }
    return false;
  }

  void init(const QString& _configGroupName)
  {
    Q_Q(ColumnSelector);

    configGroupName = _configGroupName;

    if ((treeView || tableView) && model) {
      const auto maxColumn = model->columnCount();
      QList<int> visibleColumns;
      if (!configGroupName.isEmpty()) {
        // restore the headers when we have a group name
        const auto grp = KSharedConfig::openConfig()->group(configGroupName);
        const auto columnNames = grp.readEntry("HeaderState", QByteArray());
        headerView->restoreState(columnNames);

        visibleColumns = grp.readEntry<int>("ColumnsSelection", QList<int>());
        // add the storage offset during loading operation
        for (auto& column : visibleColumns) {
          if (applyStorageOffsetColumns.contains(column+storageOffset)) {
            column += storageOffset;
          }
        }
      } else {
        // in case no group name is given, all columns are visible
        for (int col = 0; col < maxColumn; ++col) {
          visibleColumns += col;
        }
      }

      // now setup the initial visibility of the columns
      for (int col = 0; col < maxColumn; ++col) {
        if (alwaysHiddenColumns.contains(col)) {
          setColumnHidden(col, true);
        } else if (alwaysVisibleColumns.contains(col) || visibleColumns.contains(col)) {
          setColumnHidden(col, false);
        } else {
          setColumnHidden(col, true);
        }
      }

      // allow context menu to be opened on tree header for columns selection
      headerView->setContextMenuPolicy(Qt::CustomContextMenu);

      if (!isInit) {
        q->connect(headerView, &QWidget::customContextMenuRequested, q, &ColumnSelector::slotColumnsMenu);
        q->connect(headerView, &QHeaderView::sectionResized, q, &ColumnSelector::slotUpdateHeaderState);
        isInit = true;
      }

    } else if (!(treeView || tableView)) {
      qDebug() << "WARNING: You must not create a ColumnSelector without a view";
    }
  }

  ColumnSelector*       q_ptr;

  QTreeView*            treeView;
  QTableView*           tableView;
  QHeaderView*          headerView;
  QAbstractItemModel*   model;

  QVector<int>          selectableColumns;
  QVector<int>          alwaysHiddenColumns;
  QVector<int>          alwaysVisibleColumns;
  QVector<int>          applyStorageOffsetColumns;
  QString               configGroupName;

  int                   storageOffset;
  bool                  isInit;
};


ColumnSelector::ColumnSelector(QTableView* view, const QString& configGroupName, int offset, const QVector<int>& columns)
: d_ptr(new ColumnSelectorPrivate(this))
{
  Q_D(ColumnSelector);
  d->tableView = view;
  d->headerView = view->horizontalHeader();
  d->model = view->model();
  d->storageOffset = offset;
  d->applyStorageOffsetColumns = columns;
  d->init(configGroupName);
}

ColumnSelector::ColumnSelector(QTreeView* view, const QString& configGroupName, int offset, const QVector<int>& columns)
  : d_ptr(new ColumnSelectorPrivate(this))
{
  Q_D(ColumnSelector);
  d->treeView = view;
  d->headerView = view->header();
  d->model = view->model();
  d->storageOffset = offset;
  d->applyStorageOffsetColumns = columns;
  d->init(configGroupName);
}

ColumnSelector::~ColumnSelector()
{
  Q_D(ColumnSelector);
  delete d;
}

void ColumnSelector::slotUpdateHeaderState()
{
  Q_D(ColumnSelector);
  if (!d->configGroupName.isEmpty()) {
    auto grp = KSharedConfig::openConfig()->group(d->configGroupName);
    grp.writeEntry("HeaderState", d->headerView->saveState());
    grp.sync();
  }
}

void ColumnSelector::slotColumnsMenu(const QPoint)
{
  Q_D(ColumnSelector);

  // create actions menu
  QList<QAction *> actions;
  const auto maxColumn = d->headerView->count();

  for (int col = 0; col < maxColumn; ++col) {
    if (d->alwaysHiddenColumns.contains(col))
      continue;
    if (d->alwaysVisibleColumns.contains(col))
      continue;
    if (!d->selectableColumns.isEmpty()) {
      if (!d->selectableColumns.contains(col)) {
        continue;
      }
    }
    auto a = new QAction(nullptr);
    a->setObjectName(QString::number(col));
    a->setText(d->model->headerData(col, Qt::Horizontal).toString());
    a->setCheckable(true);
    a->setChecked(!d->isColumnHidden(col));
    actions.append(a);
  }

  // if there are any actions present menu
  if (!actions.isEmpty()) {
    QMenu menu(i18n("Displayed columns"));
    menu.addActions(actions);

    // execute menu and get result
    const auto retAction = menu.exec(QCursor::pos());
    if (retAction) {
      d->setColumnHidden(retAction->objectName().toInt(), !retAction->isChecked());

      QList<int> visibleColumns;

      for (int col = 0; col < maxColumn; ++col) {
        if (d->alwaysHiddenColumns.contains(col))
          continue;
        // in case a column is always visible, we
        // add it to the list for backward compatibility
        if (!d->alwaysVisibleColumns.contains(col)) {
          if (!d->selectableColumns.isEmpty() && !d->selectableColumns.contains(col))
            continue;
        }
        if (!d->isColumnHidden(col)) {
          visibleColumns.append(col);
        }
      }
      if (!d->configGroupName.isEmpty()) {
        auto grp = KSharedConfig::openConfig()->group(d->configGroupName);
        // subtract offset during storage operation
        for (auto &column : visibleColumns) {
          if (d->applyStorageOffsetColumns.contains(column)) {
            column -= d->storageOffset;
          }
        }
        grp.writeEntry("ColumnsSelection", visibleColumns);
      }

      // do this as last statement as it contains the sync of the grp
      slotUpdateHeaderState();
    }
  }
}

void ColumnSelector::setAlwaysHidden(QVector<int> columns)
{
  Q_D(ColumnSelector);

  d->alwaysHiddenColumns = columns;
  for (int i = 0; i < columns.count(); ++i) {
    auto col = columns.at(i);
    d->setColumnHidden(col, true);
    d->alwaysVisibleColumns.removeAll(col);
  }
}

void ColumnSelector::setAlwaysVisible(QVector<int> columns)
{
  Q_D(ColumnSelector);

  d->alwaysVisibleColumns = columns;
  for (int i = 0; i < columns.count(); ++i) {
    auto col = columns.at(i);
    d->setColumnHidden(col, false);
    d->alwaysHiddenColumns.removeAll(col);
  }
}

void ColumnSelector::setSelectable(QVector<int> columns)
{
  Q_D(ColumnSelector);

  d->selectableColumns = columns;
}

void ColumnSelector::setModel(QAbstractItemModel* model)
{
  Q_D(ColumnSelector);
  d->model = model;
  d->init(d->configGroupName);
}

QVector<int> ColumnSelector::columns() const
{
  Q_D(const ColumnSelector);

  QVector<int>    columns;
  const auto maxColumn = d->headerView->count();

  for (int col = 0; col < maxColumn; ++col) {
    columns.append(col);
  }
  return columns;
}
