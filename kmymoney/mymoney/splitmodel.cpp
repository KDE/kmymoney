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

#include "splitmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItem>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

struct SplitModel::Private
{
  Private(SplitModel* qq)
  : q(qq)
  , headerData(QHash<Column, QString> ({
    { Category, i18nc("Split header", "Category") },
    { Memo, i18nc("Split header", "Memo") },
    { Amount, i18nc("Split header", "Amount") },
  }))
  {
  }

  SplitModel*   q;
  QHash<Column, QString>          headerData;
};

SplitModel::SplitModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<MyMoneySplit>(parent, QStringLiteral("S"), 4, undoStack)
  , d(new Private(this))
{
}

SplitModel::~SplitModel()
{
}

int SplitModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  Q_ASSERT(d->headerData.count() == MaxColumns);
  return MaxColumns;
}

QString SplitModel::newSplitId()
{
  return QStringLiteral("New-ID");
}

bool SplitModel::isNewSplitId(const QString& id)
{
  return id.compare(newSplitId()) == 0;
}

QVariant SplitModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    switch (role) {
      case Qt::DisplayRole:
        return d->headerData.value(static_cast<Column>(section));

      case Qt::SizeHintRole:
        return QSize(20, 20);;
    }
    return {};
  }
  if (orientation == Qt::Vertical && role == Qt::SizeHintRole) {
    return QSize(10, 10);
  }

  return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags SplitModel::flags(const QModelIndex& index) const
{
  Q_UNUSED(index);
  return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

QVariant SplitModel::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  const MyMoneySplit& split = static_cast<TreeItem<MyMoneySplit>*>(idx.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        default:
          break;
      }
      break;

  }
  return {};
}

void SplitModel::appendSplit(const MyMoneySplit& split)
{
  m_undoStack->push(new UndoCommand(this, MyMoneySplit(), split));
}
