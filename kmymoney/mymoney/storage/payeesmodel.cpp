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

#include "mymoneyfile.h"

struct PayeesModel::Private
{
  Private(PayeesModel* parent)
  : q(parent)
  , emptyPayeeModel(nullptr)
  {}

  PayeesModel*            q;
  PayeesModelEmptyPayee*  emptyPayeeModel;
};

PayeesModelEmptyPayee::PayeesModelEmptyPayee(QObject* parent)
  : PayeesModel(parent)
{
  setObjectName(QLatin1String("PayeesModelEmptyPayee"));
  QMap<QString, MyMoneyPayee> list;
  list[QString()] = MyMoneyPayee();
  PayeesModel::load(list);
}

PayeesModelEmptyPayee::~PayeesModelEmptyPayee()
{
}

QVariant PayeesModelEmptyPayee::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  // never show any data for the empty payee
  if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
    return QString();

  return PayeesModel::data(idx, role);
}

PayeesModel::PayeesModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<MyMoneyPayee>(parent, QStringLiteral("P"), PayeesModel::ID_SIZE, undoStack)
  , d(new Private(this))
{
  setObjectName(QLatin1String("PayeesModel"));
}

PayeesModel::~PayeesModel()
{
}

PayeesModelEmptyPayee* PayeesModel::emptyPayee()
{
  if (d->emptyPayeeModel == nullptr) {
    d->emptyPayeeModel = new PayeesModelEmptyPayee(this);
  }
  return d->emptyPayeeModel;
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

    case eMyMoney::Model::IdRole:
      rc = payee.id();
      break;
    case eMyMoney::Model::PayeeAddressRole:
      rc = payee.address();
      break;
    case eMyMoney::Model::PayeeCityRole:
      rc = payee.city();
      break;
    case eMyMoney::Model::PayeeStateRole:
      rc = payee.state();
      break;
    case eMyMoney::Model::PayeePostCodeRole:
      rc = payee.postcode();
      break;
    case eMyMoney::Model::PayeeTelephoneRole:
      rc = payee.telephone();
      break;
    case eMyMoney::Model::PayeeEmailRole:
      rc = payee.email();
      break;
    case eMyMoney::Model::PayeeNotesRole:
      rc = payee.notes();
      break;
    case eMyMoney::Model::PayeeReferenceRole:
      rc = payee.reference();
      break;
    case eMyMoney::Model::PayeeMatchTypeRole:
      break;
    case eMyMoney::Model::PayeeMatchKeyRole:
      break;
    case eMyMoney::Model::PayeeMatchCaseRole:
      break;
    case eMyMoney::Model::PayeeDefaultAccountRole:
      rc = payee.defaultAccountId();
      break;
    case eMyMoney::Model::ItemReferenceRole:
      rc = MyMoneyFile::instance()->isReferenced(payee);
      break;
  }
  return rc;
}

bool PayeesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid())
    return false;
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return false;

  MyMoneyPayee& payee = static_cast<TreeItem<MyMoneyPayee>*>(index.internalPointer())->dataRef();

  bool rc = true;
  switch(role) {
    case eMyMoney::Model::PayeeNameRole:
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

    case eMyMoney::Model::IdRole:
      rc = false;
      break;
    case eMyMoney::Model::PayeeAddressRole:
      payee.setAddress(value.toString());
      break;
    case eMyMoney::Model::PayeeCityRole:
      payee.setCity(value.toString());
      break;
    case eMyMoney::Model::PayeeStateRole:
      payee.setState(value.toString());
      break;
    case eMyMoney::Model::PayeePostCodeRole:
      payee.setPostcode(value.toString());
      break;
    case eMyMoney::Model::PayeeTelephoneRole:
      payee.setTelephone(value.toString());
      break;
    case eMyMoney::Model::PayeeEmailRole:
      payee.setEmail(value.toString());
      break;
    case eMyMoney::Model::PayeeNotesRole:
      payee.setNotes(value.toString());
      break;
    case eMyMoney::Model::PayeeReferenceRole:
      payee.setReference(value.toString());
      break;
    case eMyMoney::Model::PayeeMatchTypeRole:
      break;
    case eMyMoney::Model::PayeeMatchKeyRole:
      break;
    case eMyMoney::Model::PayeeMatchCaseRole:
      break;
    case eMyMoney::Model::PayeeDefaultAccountRole:
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
