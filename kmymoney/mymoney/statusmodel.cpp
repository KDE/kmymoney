/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "statusmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItem>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"


StatusModel::StatusModel(QObject* parent)
  : MyMoneyModel<StatusEntry>(parent, QStringLiteral("ST"), 2, nullptr)
{
  QMap<QString, StatusEntry> states = {
    { QStringLiteral("ST01"), StatusEntry(QString(), eMyMoney::Split::State::NotReconciled, QString(), i18nc("Reconciliation state 'Not reconciled'", "Not reconciled")) },
    { QStringLiteral("ST02"), StatusEntry(QString(), eMyMoney::Split::State::Cleared, i18nc("Reconciliation flag C", "C"), i18nc("Reconciliation state 'Cleared'", "Cleared")) },
    { QStringLiteral("ST03"), StatusEntry(QString(), eMyMoney::Split::State::Reconciled, i18nc("Reconciliation flag R", "R"), i18nc("Reconciliation state 'Reconciled'", "Reconciled")) },
    { QStringLiteral("ST04"), StatusEntry(QString(), eMyMoney::Split::State::Frozen, i18nc("Reconciliation flag F", "F"), i18nc("Reconciliation state 'Frozen'", "Frozen")) }
  };
  load(states);
}

StatusModel::~StatusModel()
{
}

int StatusModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)

  return 1;
}

QVariant StatusModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_UNUSED(orientation);

  if ((role == Qt::DisplayRole) && (section == 0))
    return i18nc("Reconciliation state", "Status");
  return {};
}

Qt::ItemFlags StatusModel::flags(const QModelIndex& index) const
{
  Q_UNUSED(index);
  return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

QVariant StatusModel::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  const StatusEntry& statusEntry = static_cast<TreeItem<StatusEntry>*>(idx.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        case 0:
          return statusEntry.longName();

        default:
          break;
      }
      break;

    case eMyMoney::Model::SplitReconcileFlagRole:
      return statusEntry.shortName();

    case eMyMoney::Model::SplitReconcileStatusRole:
      return statusEntry.longName();

  }
  return {};
}
