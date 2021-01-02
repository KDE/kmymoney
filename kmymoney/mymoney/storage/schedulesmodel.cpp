/*
 * Copyright 2019-2020  Thomas Baumgart <tbaumgart@kde.org>
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
#include <QColor>
#include <QFont>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneyaccount.h"
#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneysecurity.h"

struct SchedulesModel::Private
{
  typedef struct {
    QString     name;
    QString     amount;
  } ScheduleInfo;

  Private(SchedulesModel* qq)
  : q(qq)
  , overdueScheme(QColor(Qt::red))
  , finishedScheme(QColor(Qt::darkGreen))
  {
  }

  QModelIndex indexByType(eMyMoney::Schedule::Type type)
  {
    const QModelIndexList indexes = q->match(q->index(0, 0), eMyMoney::Model::Roles::ScheduleTypeRole, static_cast<int>(type));
    if (indexes.isEmpty())
      return QModelIndex();
    return indexes.first();
  }

  ScheduleInfo scheduleInfo(const MyMoneySchedule& schedule) const
  {
    ScheduleInfo rc;
    const auto transaction = schedule.transaction();
    const auto s1 = (transaction.splits().size() < 1) ? MyMoneySplit() : transaction.splits()[0];
    const auto s2 = (transaction.splits().size() < 2) ? MyMoneySplit() : transaction.splits()[1];
    MyMoneySplit split;
    MyMoneyAccount acc;

    switch (schedule.type()) {
      case eMyMoney::Schedule::Type::Deposit:
        if (s1.value().isNegative())
          split = s2;
        else
          split = s1;
        break;

      case eMyMoney::Schedule::Type::LoanPayment:
        {
          auto found = false;
          QStringList list;
          for (const auto& s : transaction.splits()) {
            acc = MyMoneyFile::instance()->account(s.accountId());
            list.append(acc.id());
            if (acc.accountGroup() == eMyMoney::Account::Type::Asset
              || acc.accountGroup() == eMyMoney::Account::Type::Liability) {
              if (acc.accountType() != eMyMoney::Account::Type::Loan
              && acc.accountType() != eMyMoney::Account::Type::AssetLoan) {
                split = s;
                found = true;
                break;
              }
            }
          }
          if (!found) {
            qWarning() << "Split for payment account in" << schedule.id() << "not found in" << __FILE__ << __LINE__ ;
          }
        }
        break;

      default:
        if (!s1.value().isPositive())
          split = s1;
        else
          split = s2;
        break;
    }
    acc = MyMoneyFile::instance()->account(split.accountId());
    rc.name = acc.name();

    const auto currency = MyMoneyFile::instance()->currency(acc.currencyId());
    const auto amount = split.shares().abs();
    if (!acc.id().isEmpty()) {
      rc.amount = MyMoneyUtils::formatMoney(amount, acc, currency);

    } else {
      // there are some cases where the schedule does not have an account
      // in those cases the account will not have a fraction
      // use base currency instead
      rc.amount = MyMoneyUtils::formatMoney(amount, MyMoneyFile::instance()->baseCurrency());
    }
    return rc;
  }

  QString payee(const MyMoneySchedule& schedule) const
  {
    const auto transaction = schedule.transaction();
    const auto s1 = (transaction.splits().size() < 1) ? MyMoneySplit() : transaction.splits()[0];
    if (s1.payeeId().isEmpty()) {
      return {};
    }
    return MyMoneyFile::instance()->payee(s1.payeeId()).name();
  }

  SchedulesModel*   q;
  QColor            overdueScheme;
  QColor            finishedScheme;
};

SchedulesModel::SchedulesModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<MyMoneySchedule>(parent, QStringLiteral("SCH"), SchedulesModel::ID_SIZE, undoStack)
  , d(new Private(this))
{
  setObjectName(QLatin1String("SchedulesModel"));
  // force creation of group entries
  unload();
}

SchedulesModel::~SchedulesModel()
{
}

void SchedulesModel::clearModelItems()
{
  const QVector<QPair<eMyMoney::Schedule::Type, QString>> types = {
    { eMyMoney::Schedule::Type::Bill, i18nc("Schedule group", "Bills") },
    { eMyMoney::Schedule::Type::Deposit, i18nc("Schedule group", "Deposits") },
    { eMyMoney::Schedule::Type::Transfer, i18nc("Schedule group", "Transfers") },
    { eMyMoney::Schedule::Type::LoanPayment, i18nc("Schedule group", "Loans") }
  };
  MyMoneyModel<MyMoneySchedule>::clearModelItems();

  // create the type entries
  insertRows(0, types.count());
  int row = 0;
  foreach(auto type, types) {
    auto idx = index(row, 0);
    setData(idx, static_cast<int>(type.first), eMyMoney::Model::Roles::ScheduleTypeRole);
    setData(idx, type.second);
    ++row;
  }
}

int SchedulesModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
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

Qt::ItemFlags SchedulesModel::flags(const QModelIndex& index) const
{
  if (index.parent().isValid()) {
    return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  }
  return Qt::ItemIsEnabled;
}

QVariant SchedulesModel::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  // The group entries have only a single column
  if (!idx.parent().isValid() && idx.column() > 0) {
    return QVariant();
  }

  QVariant rc;
  const MyMoneySchedule& schedule = static_cast<TreeItem<MyMoneySchedule>*>(idx.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        case Column::Name:
          rc = schedule.name();
          break;

        case Column::Account:
          rc = d->scheduleInfo(schedule).name;
          break;

        case Column::Payee:
          rc = d->payee(schedule);
          break;

        case Column::Amount:
          rc = d->scheduleInfo(schedule).amount;
          break;

        case Column::NextDueDate:
          if (!schedule.isFinished()) {
            rc = MyMoneyUtils::formatDate(schedule.nextDueDate());
          } else {
            rc = i18nc("Finished schedule", "Finished");
          }
          break;

        case Column::Frequency:
          rc = i18n(schedule.occurrenceToString(schedule.occurrenceMultiplier(), schedule.occurrence()).toLatin1());
          break;

        case Column::PaymentMethod:
          rc = MyMoneyUtils::paymentMethodToString(schedule.paymentType());
          break;
      }

      // make sure to never return any displayable text for the dummy entries
      // other than the type/name column
      if (schedule.id().isEmpty() && idx.column() != Column::Name) {
        rc.clear();
      }
      break;

    case Qt::ForegroundRole:
      if (idx.parent().isValid()) {
        if (schedule.isFinished())
          return d->finishedScheme;
        if (schedule.isOverdue())
          return d->overdueScheme;
      }
      break;

    case Qt::FontRole:
      {
        QFont font;
        // display top level account groups in bold
        if (!idx.parent().isValid()) {
          font.setBold(true);
        }
        return font;
      }
      break;

    case Qt::TextAlignmentRole:
      if (idx.column() == Column::Amount) {
        rc = QVariant(Qt::AlignRight | Qt::AlignVCenter);
      } else {
        rc = QVariant(Qt::AlignLeft | Qt::AlignVCenter);
      }
      break;

    case eMyMoney::Model::Roles::IdRole:
      rc = schedule.id();
      break;

    case eMyMoney::Model::Roles::ScheduleTypeRole:
      rc = static_cast<int>(schedule.type());
      break;

    case eMyMoney::Model::ScheduleIsFinishedRole:
      rc = schedule.isFinished();
      break;

    case eMyMoney::Model::ScheduleIsOverdueRole:
      rc = schedule.isOverdue();
      break;

    case eMyMoney::Model::ScheduleAccountRole:
      rc = d->scheduleInfo(schedule).name;
      break;

    case eMyMoney::Model::SchedulePayeeRole:
      rc = d->payee(schedule);
      break;

    case eMyMoney::Model::ScheduleNextDueDateRole:
      rc = schedule.nextDueDate();
      break;

    case eMyMoney::Model::ScheduleFrequencyRole:
      rc = schedule.occurrenceMultiplier() * schedule.daysBetweenEvents(schedule.occurrence());
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

void SchedulesModel::setColorScheme(SchedulesModel::ColorScheme scheme, const QColor& color)
{
  switch(scheme) {
    case Overdue:
      d->overdueScheme= color;
      break;
    case Finished:
      d->finishedScheme = color;
      break;
  }
}


void SchedulesModel::load(const QMap<QString, MyMoneySchedule>& list)
{
  QElapsedTimer t;

  t.start();
  beginResetModel();
  clearModelItems();

  m_nextId = 0;

  QRegularExpression exp(QStringLiteral("^%1(\\d+)$").arg(m_idLeadin));
  int itemCount = 0;
  for (const auto& item : list) {
    ++itemCount;
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
  qDebug() << "Model for schedules loaded with" << itemCount << "items in" << t.elapsed() << "ms";
}

void SchedulesModel::doAddItem(const MyMoneySchedule& schedule, const QModelIndex& parentIdx)
{
  Q_UNUSED(parentIdx)

  if (schedule.type() == eMyMoney::Schedule::Type::Any) {
    qDebug() << "Schedule to be added has no type. Rejected.";
    return;
  }

  QModelIndex group = d->indexByType(schedule.type());

  if (group.isValid()) {
    MyMoneyModel::doAddItem(schedule, group);
  }
#if 0
  const int row = rowCount(group);
  insertRows(row, 1, group);
  const QModelIndex idx = index(row, 0, group);
  static_cast<TreeItem<MyMoneySchedule>*>(idx.internalPointer())->dataRef() = sch
  setDirty();
  emit dataChanged(idx, index(row, columnCount(group)-1, group));
#endif
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
