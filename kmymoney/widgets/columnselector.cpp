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

class ColumnSelectorPrivate
{
public:
  QTreeView*            view;
  QString               configGroupName;
};


ColumnSelector::ColumnSelector(QTreeView *view, const QString& configGroupName)
  : d_ptr(new ColumnSelectorPrivate)
{
  Q_D(ColumnSelector);
  d->view = view;
  d->configGroupName = configGroupName;

  if (view && view->model()) {
    // restore the headers when we have a view and a model
    const auto grp = KSharedConfig::openConfig()->group(configGroupName);
    const auto columnNames = grp.readEntry("HeaderState", QByteArray());
    view->header()->restoreState(columnNames);

    QList<int> visibleColumns = grp.readEntry<int>("ColumnsSelection", QList<int>());
    if (!visibleColumns.isEmpty()) {
      const auto maxColumn = view->model()->columnCount();
      for (int col = 1; col < maxColumn; ++col) {
        view->setColumnHidden(col, !visibleColumns.contains(col));
      }
    }
    // allow context menu to be opened on tree header for columns selection
    view->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(view->header(), &QWidget::customContextMenuRequested, this, &ColumnSelector::slotColumnsMenu);
    connect(view->header(), &QHeaderView::sectionResized, this, &ColumnSelector::slotUpdateHeaderState);
  } else {
    qDebug() << "WARNING: You must not create a ColumnSelector without a view or view without model";
  }

}

ColumnSelector::~ColumnSelector()
{
  Q_D(ColumnSelector);
  delete d;
}

void ColumnSelector::slotUpdateHeaderState()
{
  Q_D(ColumnSelector);
  auto grp = KSharedConfig::openConfig()->group(d->configGroupName);
  grp.writeEntry("HeaderState", d->view->header()->saveState());
  grp.sync();
}

void ColumnSelector::slotColumnsMenu(const QPoint)
{
  Q_D(ColumnSelector);

  // create menu
  QMenu menu(i18n("Displayed columns"));
  QList<QAction *> actions;
  const auto maxColumn = d->view->header()->count();

  for (int col = 1; col < maxColumn; ++col) {
    auto a = new QAction(nullptr);
    a->setObjectName(QString::number(col));
    a->setText(d->view->model()->headerData(col, Qt::Horizontal).toString());
    a->setCheckable(true);
    a->setChecked(!d->view->isColumnHidden(col));
    actions.append(a);
  }
  menu.addActions(actions);

  // execute menu and get result
  const auto retAction = menu.exec(QCursor::pos());
  if (retAction) {
    d->view->setColumnHidden(retAction->objectName().toInt(), !retAction->isChecked());

    auto grp = KSharedConfig::openConfig()->group(d->configGroupName);
    // column zero is always visible
    QList<int> visibleColumns { 0 };

    for (int col = 1; col < maxColumn; ++col) {
      if (!d->view->isColumnHidden(col)) {
        visibleColumns.append(col);
      }
    }
    grp.writeEntry("ColumnsSelection", visibleColumns);

    // do this as last statement as it contains the sync of the grp
    slotUpdateHeaderState();
  }
}
