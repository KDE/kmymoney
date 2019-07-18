/*
 * Copyright 2016-2019  Thomas Baumgart <tbaumgart@kde.org>
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

#include "payeesmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

struct PayeesModel::Private
{
  Private()
  {}

};

PayeesModel::PayeesModel(QObject* parent)
  : MyMoneyModel<MyMoneyPayee>(parent, QStringLiteral("P"), PayeesModel::ID_SIZE)
  , d(new Private)
{
  setObjectName(QLatin1String("PayeesModel"));
}

PayeesModel::~PayeesModel()
{
}

int PayeesModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return 1;
}

QVariant PayeesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case 0:
        return i18n("Payee");
        break;
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant PayeesModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  QVariant rc;
  const MyMoneyPayee& payee = static_cast<TreeItem<MyMoneyPayee>*>(index.internalPointer())->constDataRef();
  switch (role) {
    case eMyMoney::Model::Roles::PayeeNameRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
      // make sure to never return any displayable text for the dummy entry
      if (!payee.id().isEmpty()) {
        rc = payee.name();
      } else {
        rc = QString();
      }
      break;

    case Qt::TextAlignmentRole:
      rc = QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = payee.id();
      break;
    case eMyMoney::Model::Roles::PayeeAddressRole:
      rc = payee.address();
      break;
    case eMyMoney::Model::Roles::PayeeCityRole:
      rc = payee.city();
      break;
    case eMyMoney::Model::Roles::PayeeStateRole:
      rc = payee.state();
      break;
    case eMyMoney::Model::Roles::PayeePostCodeRole:
      rc = payee.postcode();
      break;
    case eMyMoney::Model::Roles::PayeeTelephoneRole:
      rc = payee.telephone();
      break;
    case eMyMoney::Model::Roles::PayeeEmailRole:
      rc = payee.email();
      break;
    case eMyMoney::Model::Roles::PayeeNotesRole:
      rc = payee.notes();
      break;
    case eMyMoney::Model::Roles::PayeeReferenceRole:
      rc = payee.reference();
      break;
    case eMyMoney::Model::Roles::PayeeMatchTypeRole:
      break;
    case eMyMoney::Model::Roles::PayeeMatchKeyRole:
      break;
    case eMyMoney::Model::Roles::PayeeMatchCaseRole:
      break;
    case eMyMoney::Model::Roles::PayeeDefaultAccountRole:
      rc = payee.defaultAccountId();
      break;
  }
  return rc;
}

bool PayeesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid()) {
    return false;
  }

  if (!index.isValid())
    return false;
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return false;

  MyMoneyPayee& payee = static_cast<TreeItem<MyMoneyPayee>*>(index.internalPointer())->dataRef();

  bool rc = true;
  switch(role) {
    case eMyMoney::Model::Roles::PayeeNameRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
      // make sure to never return any displayable text for the dummy entry
      if (!payee.id().isEmpty()) {
        payee.setName(value.toString());
      } else {
        rc = false;
      }
      break;

    case Qt::TextAlignmentRole:
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = false;
      break;
    case eMyMoney::Model::Roles::PayeeAddressRole:
      payee.setAddress(value.toString());
      break;
    case eMyMoney::Model::Roles::PayeeCityRole:
      payee.setCity(value.toString());
      break;
    case eMyMoney::Model::Roles::PayeeStateRole:
      payee.setState(value.toString());
      break;
    case eMyMoney::Model::Roles::PayeePostCodeRole:
      payee.setPostcode(value.toString());
      break;
    case eMyMoney::Model::Roles::PayeeTelephoneRole:
      payee.setTelephone(value.toString());
      break;
    case eMyMoney::Model::Roles::PayeeEmailRole:
      payee.setEmail(value.toString());
      break;
    case eMyMoney::Model::Roles::PayeeNotesRole:
      payee.setNotes(value.toString());
      break;
    case eMyMoney::Model::Roles::PayeeReferenceRole:
      payee.setReference(value.toString());
      break;
    case eMyMoney::Model::Roles::PayeeMatchTypeRole:
      break;
    case eMyMoney::Model::Roles::PayeeMatchKeyRole:
      break;
    case eMyMoney::Model::Roles::PayeeMatchCaseRole:
      break;
    case eMyMoney::Model::Roles::PayeeDefaultAccountRole:
      payee.setDefaultAccountId(value.toString());
      break;
    default:
      rc = false;
      break;
  }

  if (rc) {
    setDirty();
    const auto topLeft = PayeesModel::index(index.row(), 0);
    const auto bottomRight = PayeesModel::index(index.row(), columnCount()-1);
    emit dataChanged(topLeft, bottomRight);
  }
  return rc;
}
