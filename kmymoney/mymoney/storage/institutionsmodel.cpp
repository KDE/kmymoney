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
#include "accountsmodel.h"

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
  Q_DECLARE_PUBLIC(InstitutionsModel);

  Private(InstitutionsModel* qq, QObject* parent)
    : q_ptr(qq)
    , parentObject(parent)
  {
  }

  void loadAccounts(const QModelIndex& idx, const QList<QString>& idList)
  {
    Q_Q(InstitutionsModel);
    const int accounts = idList.count();
    q->insertRows(0, accounts, idx);
    // we create institution subentries here with the id of the account. These will never
    // be used as institutions and also not found by indexById for a different m_leadIn
    // The are only used in data() to proxy data from the accountsModel.
    for (int row = 0; row < accounts; ++row) {
      const auto subIdx = q->index(row, 0, idx);
      MyMoneyInstitution account(idList.at(row), MyMoneyInstitution());
      static_cast<TreeItem<MyMoneyInstitution>*>(subIdx.internalPointer())->dataRef() = account;
    }
  }


  InstitutionsModel*  q_ptr;
  QObject*            parentObject;
  AccountsModel*      accountsModel;
};

InstitutionsModel::InstitutionsModel(AccountsModel* accountsModel, QObject* parent)
  : MyMoneyModel<MyMoneyInstitution>(parent, QStringLiteral("I"), InstitutionsModel::ID_SIZE)
  , d(new Private(this, parent))
{
  d->accountsModel = accountsModel;
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
  if (Q_UNLIKELY(!d->accountsModel))
    return QVariant();
  return d->accountsModel->headerData(section, orientation, role);
}

QVariant InstitutionsModel::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  QVariant rc;
  const MyMoneyInstitution& institution = static_cast<TreeItem<MyMoneyInstitution>*>(idx.internalPointer())->constDataRef();

  // check for a sub-entry which is actually a proxy to the corresponding account
  if (idx.isValid()) {
    const auto accountIdx = d->accountsModel->indexById(institution.id());
    const auto subIdx = d->accountsModel->index(accountIdx.row(), idx.column(), accountIdx.parent());
    return d->accountsModel->data(subIdx, role);
  }

  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        case Column::AccountName:
          // make sure to never return any displayable text for the dummy entry
          if (!institution.id().isEmpty()) {
            rc = institution.name();
          } else {
            rc = QString();
          }
          break;
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

  int row = 0;
  // insert one more used for the no institution item
  insertRows(0, list.count()+1);
  MyMoneyInstitution noBank((QString()), MyMoneyInstitution());
  noBank.setName(i18n("Accounts with no institution assigned"));
  static_cast<TreeItem<MyMoneyInstitution>*>(index(0, 0).internalPointer())->dataRef() = noBank;
  ++row;
  foreach(const auto institution, list) {
    const auto idx = index(row, 0);
    static_cast<TreeItem<MyMoneyInstitution>*>(idx.internalPointer())->dataRef() = institution;
    d->loadAccounts(idx, institution.accountList());
    ++row;
  }

  endResetModel();

  emit modelLoaded();

  qDebug() << "Model for \"I\" loaded with" << rowCount() << "items";
}
