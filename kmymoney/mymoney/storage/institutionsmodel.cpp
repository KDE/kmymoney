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

#include "institutionsmodel.h"

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

struct InstitutionsModel::Private
{
  Private(InstitutionsModel* qq, QObject* parent)
    : q_ptr(qq)
    , parentObject(parent)
  {
  }


  InstitutionsModel*    q_ptr;
  QObject*            parentObject;
};

InstitutionsModel::InstitutionsModel(QObject* parent)
  : MyMoneyModel<MyMoneyInstitution>(parent, QStringLiteral("I"), InstitutionsModel::ID_SIZE)
  , d(new Private(this, parent))
{
  setObjectName(QLatin1String("InstitutionsModel"));
}

InstitutionsModel::~InstitutionsModel()
{
}

int InstitutionsModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return Column::MaxColumns;
}

QVariant InstitutionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case Column::AccountName:
        return i18n("Name");
      case Column::Type:
        return i18n("Type");
      case Column::Tax:
        return i18n("Tax");
      case Column::Vat:
        return i18n("VAT");
      case Column::CostCenter:
        return i18nc("CostCenter", "CC");
      case Column::TotalBalance:
        return i18n("Total Balance");
      case Column::PostedValue:
        return i18n("Posted Value");
      case Column::TotalValue:
        return i18n("Total Value");
      case Column::Number:
        return i18n("Number");
      case Column::SortCode:
        return i18n("SortCode");
      default:
        return QVariant();
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant InstitutionsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  QVariant rc;
  const MyMoneyInstitution& institution = static_cast<TreeItem<MyMoneyInstitution>*>(index.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(index.column()) {
        case Column::AccountName:
          // make sure to never return any displayable text for the dummy entry
          if (!institution.id().isEmpty()) {
            rc = institution.name();
          } else {
            rc = QString();
          }
          break;
      case Column::Type:
        return i18n("Type");
      case Column::Tax:
        return i18n("Tax");
      case Column::Vat:
        return i18n("VAT");
      case Column::CostCenter:
        return i18nc("CostCenter", "CC");
      case Column::TotalBalance:
        return i18n("Total Balance");
      case Column::PostedValue:
        return i18n("Posted Value");
      case Column::TotalValue:
        return i18n("Total Value");
      case Column::Number:
        return i18n("Number");
      case Column::SortCode:
        return i18n("SortCode");
        default:
          break;
      }
      break;

    case Qt::TextAlignmentRole:
      rc = QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = institution.id();
      break;

  }
  return rc;
}

bool InstitutionsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }

  qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  return QAbstractItemModel::setData(index, value, role);
}

void InstitutionsModel::load(const QMap<QString, MyMoneyInstitution>& list)
{
  beginResetModel();
  // first get rid of any existing entries
  clearModelItems();


  // and don't count loading as a modification
  setDirty(false);

  /// @todo here implement institution loading
  int row = 0;
  foreach (const auto item, list) {
    static_cast<TreeItem<MyMoneyInstitution>*>(index(row, 0).internalPointer())->dataRef() = item;
    ++row;
  }
  endResetModel();

  qDebug() << "Model for 'I' loaded with" << rowCount() << "items";
}
