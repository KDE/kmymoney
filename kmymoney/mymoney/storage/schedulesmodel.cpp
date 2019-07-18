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

#include "schedulesmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneyaccount.h"

struct SchedulesModel::Private
{
  Private(SchedulesModel* qq)
  : q(qq)
  {
  }

  QModelIndex indexByType(eMyMoney::Schedule::Type type)
  {
    const QModelIndexList indexes = q->match(q->index(0, 0), eMyMoney::Model::Roles::ScheduleTypeRole, static_cast<int>(type));
    if (indexes.isEmpty())
      return QModelIndex();
    return indexes.first();
  }

  SchedulesModel* q;
};

SchedulesModel::SchedulesModel(QObject* parent)
  : MyMoneyModel<MyMoneySchedule>(parent, QStringLiteral("SCH"), SchedulesModel::ID_SIZE)
  , d(new Private(this))
{
  setObjectName(QLatin1String("SchedulesModel"));
}

SchedulesModel::~SchedulesModel()
{
}

int SchedulesModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  // The group entries have only a single column
  if (!parent.isValid()) {
    return 1;
  }
  return static_cast<int>(Column::MaxColumns);
}


QVariant SchedulesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  const QVector<const char *> headers = {
    I18N_NOOP2("Schedule header", "Type/Name"),
    I18N_NOOP2("Schedule header", "Account"),
    I18N_NOOP2("Schedule header", "Payee"),
    I18N_NOOP2("Schedule header", "Amount"),
    I18N_NOOP2("Schedule header", "Next Due Date"),
    I18N_NOOP2("Schedule header", "Frequency"),
    I18N_NOOP2("Schedule header", "Payment Method")
  };

  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section < headers.count())
      return i18n(headers.at(section));
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant SchedulesModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return QVariant();

  QVariant rc;
  const MyMoneySchedule& schedule = static_cast<TreeItem<MyMoneySchedule>*>(index.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      // make sure to never return any displayable text for the dummy entries
      // other than the type/name column
      switch(index.column()) {
        case Column::Name:
          rc = schedule.name();
          break;
      }

      if (schedule.id().isEmpty() && index.column() != Column::Name) {
        rc.clear();
      }
      break;

    case Qt::TextAlignmentRole:
      rc = QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = schedule.id();
      break;

    case eMyMoney::Model::Roles::ScheduleTypeRole:
      rc = static_cast<int>(schedule.type());
      break;
  }
  return rc;
}

bool SchedulesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }
  bool rc = false;

  // qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
  MyMoneySchedule& schedule = static_cast<TreeItem<MyMoneySchedule>*>(index.internalPointer())->dataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(index.column()) {
        case Column::Name:
          schedule.setName(value.toString());
          rc = true;
          break;
        default:
          break;
      }
      break;

    case eMyMoney::Model::Roles::ScheduleTypeRole:
      schedule.setType(static_cast<eMyMoney::Schedule::Type>(value.toInt()));
      rc = true;
      break;

    default:
      break;
  }
  m_dirty |= rc;
  return rc;
}

void SchedulesModel::load(const QMap<QString, MyMoneySchedule>& list)
{
  const QVector<QPair<eMyMoney::Schedule::Type, QString>> types = {
    { eMyMoney::Schedule::Type::Bill, i18n("Bills") },
    { eMyMoney::Schedule::Type::Deposit, i18n("Deposits") },
    { eMyMoney::Schedule::Type::Transfer, i18n("Transfers") },
    { eMyMoney::Schedule::Type::LoanPayment, i18n("Loans") }
  };

  beginResetModel();
  clearModelItems();

  // create the type entries
  insertRows(0, types.count());
  int row = 0;
  foreach(auto type, types) {
    auto idx = index(row, 0);
    setData(idx, static_cast<int>(type.first), eMyMoney::Model::Roles::ScheduleTypeRole);
    setData(idx, type.second);
    ++row;
  }
  m_nextId = 0;

  QRegularExpression exp(QStringLiteral("^%1(\\d+)$").arg(m_idLeadin));
  foreach (auto item, list) {
    QRegularExpressionMatch m = exp.match(item.id());
    if (m.hasMatch()) {
      const quint64 id = m.captured(1).toUInt();
      if (id > m_nextId) {
        m_nextId = id;
      }
    }
    auto groupIdx = d->indexByType(item.type());
    if (groupIdx.isValid()) {
      const int r = rowCount(groupIdx);
      insertRows(r, 1, groupIdx);
      static_cast<TreeItem<MyMoneySchedule>*>(index(r, 0, groupIdx).internalPointer())->dataRef() = item;
    } else {
      qDebug() << "Invalid schedule item of type" << static_cast<int>(item.type()) << "- Skipped";
    }
  }
  // and don't count loading as a modification
  setDirty(false);

  // inform that the whole model has changed
  endResetModel();
}

void SchedulesModel::addItem(MyMoneySchedule& schedule)
{
  if (schedule.type() == eMyMoney::Schedule::Type::Any) {
    qDebug() << "Schedule to be added has no type. Rejected.";
    return;
  }

  QModelIndex group = d->indexByType(schedule.type());

  const int row = rowCount(group);
  insertRows(row, 1, group);
  const QModelIndex idx = index(row, 0, group);
  // assign an ID and store the object and
  // make sure the caller receives the assigned ID
  schedule = MyMoneySchedule(nextId(), schedule);
  static_cast<TreeItem<MyMoneySchedule>*>(idx.internalPointer())->dataRef() = schedule;

  setDirty();
  emit dataChanged(idx, index(row, columnCount(group)-1, group));
}

QList<MyMoneySchedule> SchedulesModel::scheduleList(const QString& accountId,
                                                       eMyMoney::Schedule::Type type,
                                                       eMyMoney::Schedule::Occurrence occurrence,
                                                       eMyMoney::Schedule::PaymentType paymentType,
                                                       const QDate& startDate,
                                                       const QDate& endDate,
                                                       bool overdue) const

{
  // in case the type is set, we take the group item as start point.
  // in other cases, we use the root item
  QModelIndex group;
  if (type != eMyMoney::Schedule::Type::Any) {
    group = d->indexByType(type);
  }
  const auto indexes = match(index(0, 0, group), eMyMoney::Model::Roles::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive);

  QList<MyMoneySchedule> list;
  for (int row = 0; row < indexes.count(); ++row) {
    const QModelIndex& idx = indexes.at(row);
    const auto& schedule = static_cast<TreeItem<MyMoneySchedule>*>(idx.internalPointer())->constDataRef();

    if (occurrence != eMyMoney::Schedule::Occurrence::Any) {
      if (occurrence != schedule.baseOccurrence()) {
        continue;
      }
    }

    if (paymentType != eMyMoney::Schedule::PaymentType::Any) {
      if (paymentType != schedule.paymentType()) {
        continue;
      }
    }

    if (!accountId.isEmpty()) {
      bool found = false;
      const MyMoneyTransaction& t = schedule.transaction();
      foreach(const auto& split, t.splits()) {
        if (split.accountId() == accountId) {
          found = true;
          break;
        }
      }
      if (!found) {
        continue;
      }
    }

    if (startDate.isValid() && endDate.isValid()) {
      if (schedule.paymentDates(startDate, endDate).count() == 0) {
        continue;
      }
    }

    if (startDate.isValid() && !endDate.isValid()) {
      if (!schedule.nextPayment(startDate.addDays(-1)).isValid()) {
        continue;
      }
    }

    if (!startDate.isValid() && endDate.isValid()) {
      if (schedule.startDate() > endDate) {
        continue;
      }
    }

    if (overdue) {
      if (!schedule.isOverdue())
        continue;
    }

    // qDebug("Adding '%s'", (*pos).name().toLatin1());
    list << schedule;
  }
  return list;
}

#if 0
QList<MyMoneySchedule> SchedulesModel::scheduleListEx(int scheduleTypes,
                                                      int scheduleOcurrences,
                                                      int schedulePaymentTypes,
                                                      QDate date,
                                                      const QStringList& accounts) const
{
  const auto indexes = match(index(0, 0), eMyMoney::ModelRoles::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive);

  QList<MyMoneySchedule> list;
  if (!date.isValid())
    return list;

  for (int row = 0; row < indexes.count(); ++row) {
    const QModelIndex& idx = indexes.at(row);
    const auto& schedule = static_cast<TreeItem<MyMoneySchedule>*>(idx.internalPointer())->constDataRef();

    if (scheduleTypes && !(scheduleTypes & (int)schedule.type()))
      continue;

    if (scheduleOcurrences && !(scheduleOcurrences & (int)schedule.baseOccurrence()))
      continue;

    if (schedulePaymentTypes && !(schedulePaymentTypes & (int)schedule.paymentType()))
      continue;

    if (schedule.paymentDates(date, date).count() == 0)
      continue;

    if (schedule.isFinished())
      continue;

    if (schedule.hasRecordedPayment(date))
      continue;

    if (accounts.count() > 0) {
      if (accounts.contains(schedule.account().id()))
        continue;
    }

    //    qDebug("\tAdding '%s'", (*pos).name().toLatin1());
    list << schedule;
  }

  return list;
}
#endif
