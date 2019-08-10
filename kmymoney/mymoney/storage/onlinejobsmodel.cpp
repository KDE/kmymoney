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

#include "onlinejobsmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "mymoneyaccount.h"

struct OnlineJobsModel::Private
{
  Private() {}
};

OnlineJobsModel::OnlineJobsModel(QObject* parent)
  : MyMoneyModel<onlineJob>(parent, QStringLiteral("O"), OnlineJobsModel::ID_SIZE)
  , d(new Private)
{
  setObjectName(QLatin1String("OnlineJobsModel"));
}

OnlineJobsModel::~OnlineJobsModel()
{
}

int OnlineJobsModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 1;
}

QVariant OnlineJobsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0:
        return i18nc("OnlineJob", "Name");
        break;
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant OnlineJobsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  const onlineJob& job = static_cast<TreeItem<onlineJob>*>(index.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(index.column()) {
        case AccountName:
          return MyMoneyFile::instance()->accountsModel()->itemById(job.responsibleAccount()).name();
        default:
          return QStringLiteral("not yet implemented");
      }
      break;

    case Qt::TextAlignmentRole:
      if (index.column() == Columns::Value) {
        return QVariant(Qt::AlignRight | Qt::AlignVCenter);
      }
      return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      break;

    case eMyMoney::Model::Roles::IdRole:
      return job.id();
      break;
  }
  return QVariant();
}

bool OnlineJobsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }

  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}
