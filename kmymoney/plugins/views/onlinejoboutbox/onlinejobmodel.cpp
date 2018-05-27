/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2013-2015 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "onlinejobmodel.h"

#include <QIcon>
#include <QDateTime>

#include <KLocalizedString>

#include "mymoneyobject.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyutils.h"
#include "onlinetasks/interfaces/tasks/onlinetask.h"
#include "onlinetasks/interfaces/tasks/credittransfer.h"
#include "mymoney/onlinejobtyped.h"
#include "payeeidentifier.h"
#include "payeeidentifiertyped.h"
#include "payeeidentifier/ibanbic/ibanbic.h"
#include "icons/icons.h"
#include "mymoneyenums.h"

using namespace Icons;

onlineJobModel::onlineJobModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_jobIdList(QStringList())
{
  MyMoneyFile *const file = MyMoneyFile::instance();
  connect(file, &MyMoneyFile::objectAdded,
          this, &onlineJobModel::slotObjectAdded);
  connect(file, &MyMoneyFile::objectModified,
          this, &onlineJobModel::slotObjectModified);
  connect(file, &MyMoneyFile::objectRemoved,
          this, &onlineJobModel::slotObjectRemoved);
}

void onlineJobModel::load()
{
  unload();
  beginInsertRows(QModelIndex(), 0, 0);
  foreach (const onlineJob job, MyMoneyFile::instance()->onlineJobList()) {
    m_jobIdList.append(job.id());
  }
  endInsertRows();
}

void onlineJobModel::unload()
{
  if (!m_jobIdList.isEmpty()) {
    beginResetModel();
    m_jobIdList.clear();
    endResetModel();
  }
}

int onlineJobModel::rowCount(const QModelIndex & parent) const
{
  if (parent.isValid())
    return 0;
  return m_jobIdList.count();
}

int onlineJobModel::columnCount(const QModelIndex & parent) const
{
  if (parent.isValid())
    return 0;
  return 4;
}

QVariant onlineJobModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal) {
    switch (section) {
      case columns::ColAccount: return i18n("Account");
      case columns::ColAction: return i18n("Action");
      case columns::ColDestination: return i18n("Destination");
      case columns::ColValue: return i18n("Value");
    }
  }
  return QVariant();
}

/**
 * @todo LOW improve speed
 * @todo use now onlineJob system
 */
QVariant onlineJobModel::data(const QModelIndex & index, int role) const
{
  if (index.parent().isValid())
    return QVariant();

  Q_ASSERT(m_jobIdList.length() > index.row());
  onlineJob job;

  try {
    job = MyMoneyFile::instance()->getOnlineJob(m_jobIdList[index.row()]);
  } catch (const MyMoneyException &) {
    return QVariant();
  }

  // id of MyMoneyObject
  if (role == roles::OnlineJobId)
    return QVariant::fromValue(job.id());
  else if (role == roles::OnlineJobRole)
    return QVariant::fromValue(job);

  // If job is null, display an error message and exit
  if (job.isNull()) {
    if (index.column() == columns::ColAction) {
      switch (role) {
        case Qt::DisplayRole: return i18n("Not able to display this job.");
        case Qt::ToolTipRole: return i18n("Could not find a plugin to display this job or it does not contain any data.");
      }
    }
    return QVariant();
  }

  // Show general information
  if (index.column() == columns::ColAccount) {
    // Account column
    if (role == Qt::DisplayRole) {
      return QVariant::fromValue(job.responsibleMyMoneyAccount().name());
    } else if (role == Qt::DecorationRole) {
      if (job.isLocked())
        return Icons::get(Icon::TaskOngoing);

      switch (job.bankAnswerState()) {
        case onlineJob::acceptedByBank: return Icons::get(Icon::TaskComplete);
        case onlineJob::sendingError:
        case onlineJob::abortedByUser:
        case onlineJob::rejectedByBank: return Icons::get(Icon::TaskReject);
        case onlineJob::noBankAnswer: break;
      }
      if (job.sendDate().isValid()) {
        return Icons::get(Icon::TaskAccepted);
      } else if (!job.isValid()) {
        return Icons::get(Icon::TaskAttention);
      }
    } else if (role == Qt::ToolTipRole) {
      if (job.isLocked())
        return i18n("Job is being processed at the moment.");

      switch (job.bankAnswerState()) {
        case onlineJob::acceptedByBank: return i18nc("Arg 1 is a date/time", "This job was accepted by the bank on %1.", job.bankAnswerDate().toString(Qt::DefaultLocaleShortDate));
        case onlineJob::sendingError: return i18nc("Arg 1 is a date/time", "Sending this job failed (tried on %1).", job.sendDate().toString(Qt::DefaultLocaleShortDate));
        case onlineJob::abortedByUser: return i18n("Sending this job was manually aborted.");
        case onlineJob::rejectedByBank: return i18nc("Arg 1 is a date/time", "The bank rejected this job on %1.", job.bankAnswerDate().toString(Qt::DefaultLocaleShortDate));
        case onlineJob::noBankAnswer:
          if (job.sendDate().isValid())
            return i18nc("Arg 1 is a date/time", "The bank accepted this job on %1.", job.sendDate().toString(Qt::DefaultLocaleShortDate));
          else if (!job.isValid())
            return i18n("This job needs further editing and cannot be sent therefore.");
          else
            return i18n("This job is ready for sending.");
      }
    }

    return QVariant();
  } else if (index.column() == columns::ColAction) {
    if (role == Qt::DisplayRole)
      return QVariant::fromValue(job.task()->jobTypeName());
    return QVariant();
  }

  // Show credit transfer data
  try {
    onlineJobTyped<creditTransfer> transfer(job);

    if (index.column() == columns::ColValue) {
      if (role == Qt::DisplayRole)
        return QVariant::fromValue(MyMoneyUtils::formatMoney(transfer.task()->value(), transfer.task()->currency()));
      if (role == Qt::TextAlignmentRole)
        return int (Qt::AlignVCenter | Qt::AlignRight);
    } else if (index.column() == columns::ColDestination) {
      if (role == Qt::DisplayRole) {
        const payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBic(transfer.constTask()->beneficiary());
        return QVariant(ibanBic->ownerName());
      }
    }
  } catch (const MyMoneyException &) {
  }

  return QVariant();
}

void onlineJobModel::reloadAll()
{
  emit dataChanged(index(rowCount() - 1, 0), index(rowCount() - 1, columnCount() - 1));
}

/**
 * This method removes the rows from MyMoneyFile.
 */
bool onlineJobModel::removeRow(int row, const QModelIndex& parent)
{
  if (parent.isValid())
    return false;

  Q_ASSERT(m_jobIdList.count() < row);
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyFileTransaction transaction;
  const onlineJob job = file->getOnlineJob(m_jobIdList[row]);
  file->removeOnlineJob(job);
  transaction.commit();
  return true;
}

/**
 * This method removes the rows from MyMoneyFile.
 */
bool onlineJobModel::removeRows(int row, int count, const QModelIndex & parent)
{
  if (parent.isValid())
    return false;

  Q_ASSERT(m_jobIdList.count() > row);
  Q_ASSERT(m_jobIdList.count() >= (row + count));

  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyFileTransaction transaction;
  for (int i = 0; i < count; ++i) {
    const onlineJob job = file->getOnlineJob(m_jobIdList[row+i]);
    file->removeOnlineJob(job);
  }
  transaction.commit();
  return true;
}

void onlineJobModel::slotObjectAdded(eMyMoney::File::Object objType, const QString& id)
{
  if (Q_LIKELY(objType != eMyMoney::File::Object::OnlineJob))
    return;
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  m_jobIdList.append(id);
  endInsertRows();
}

void onlineJobModel::slotObjectModified(eMyMoney::File::Object objType, const QString& id)
{
  if (Q_LIKELY(objType != eMyMoney::File::Object::OnlineJob))
    return;

  int row = m_jobIdList.indexOf(id);
  if (row != -1)
    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}

void onlineJobModel::slotObjectRemoved(eMyMoney::File::Object objType, const QString& id)
{
  if (Q_LIKELY(objType != eMyMoney::File::Object::OnlineJob))
    return;

  int row = m_jobIdList.indexOf(id);
  if (row != -1) {
    m_jobIdList.removeAll(id);
    beginRemoveRows(QModelIndex(), row, row);
    endRemoveRows();
  }
}
