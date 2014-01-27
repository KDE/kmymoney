/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
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

#include <QDebug>
#include <QTimer>

#include <KIcon>
#include <KLocalizedString>

#include "mymoneyutils.h"
#include "mymoney/onlinetransfer.h"
#include "mymoney/onlinejobtyped.h"


onlineJobModel::onlineJobModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_jobIdList( QStringList() )
{
  MyMoneyFile *file = MyMoneyFile::instance();
  connect(file, SIGNAL( objectAdded(MyMoneyFile::notificationObjectT,MyMoneyObject*const) ),
          this, SLOT(slotObjectAdded(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(file, SIGNAL(objectModified(MyMoneyFile::notificationObjectT,MyMoneyObject*const)),
          this, SLOT(slotObjectModified(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(file, SIGNAL(objectRemoved(MyMoneyFile::notificationObjectT,QString)),
          this, SLOT(slotObjectRemoved(MyMoneyFile::notificationObjectT,QString)));
}

void onlineJobModel::load()
{ 
  unload();
  beginInsertRows(QModelIndex(), 0, 0);
  foreach(const onlineJob job, MyMoneyFile::instance()->onlineJobList()) {
    m_jobIdList.append(job.id());
  }
  endInsertRows();
  emit dataChanged(index(0, 0, QModelIndex()), index(rowCount()-1, columnCount()-1, QModelIndex()));
}

void onlineJobModel::unload()
{
  if (rowCount() != 0) {
    unsigned int oldRowCount = rowCount();
    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_jobIdList.clear();
    endRemoveRows();
    emit dataChanged(index(0, 0, QModelIndex()), index(oldRowCount-1, columnCount()-1, QModelIndex()));
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
      switch(section) {
      case ColAccount: return i18n("Account");
      case ColAction: return i18n("Action");
      case ColDestination: return i18n("Destination");
      case ColValue: return i18n("Value");
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

  Q_ASSERT( m_jobIdList.length() > index.row() );
  onlineJob job;
  
  try {
    job = MyMoneyFile::instance()->getOnlineJob( m_jobIdList[index.row()] );
  } catch (MyMoneyException* e) {
    delete e;
    return QVariant();
  }
  
  Q_ASSERT( !job.isNull() );

  const onlineTransfer *transfer = dynamic_cast<const onlineTransfer*>(job.task());

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case ColAccount: return QVariant::fromValue(job.responsibleMyMoneyAccount().name());
    case ColAction: return QVariant::fromValue(job.task()->jobTypeName());
    case ColDestination: return ( (transfer != 0) ? QVariant::fromValue(transfer->getRecipient().ownerName()) : QVariant());
    case ColValue: return ( (transfer != 0) ? QVariant::fromValue(MyMoneyUtils::formatMoney(transfer->value(), transfer->currency())) : QVariant() );
    default: return QVariant();
    }
  } else if (role == Qt::DecorationRole && index.column() == 0 && transfer != 0) {
    if (job.isLocked())
      return KIcon("task-ongoing");

    switch (job.bankAnswerState()) {
    case onlineJob::acceptedByBank: return KIcon("task-complete");
    case onlineJob::sendingError:
    case onlineJob::abortedByUser: 
    case onlineJob::rejectedByBank: return KIcon("task-reject");
    case onlineJob::noBankAnswer: break;
    }
    if (job.sendDate().isValid()) {
      return KIcon("task-accepted");
    } else if ( !job.isValid() ) {
      return KIcon("task-attention");
    }
  } else if (role == OnlineJobId || role == Qt::ToolTipRole) {
    return job.id();
  }
  return QVariant();
}

void onlineJobModel::reloadAll()
{
  emit dataChanged(index(rowCount()-1, 0), index(rowCount()-1, columnCount()-1));
}

/**
 * This method removes the rows from MyMoneyFile.
 */
bool onlineJobModel::removeRow(int row, const QModelIndex& parent)
{
  if (parent.isValid())
    return false;

  Q_ASSERT( m_jobIdList.count() < row );  
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyFileTransaction transaction;
  const onlineJob job = file->getOnlineJob( m_jobIdList[row] );
  file->removeOnlineJob(job);
  transaction.commit();
  return true;
}

/**
 * This method removes the rows from MyMoneyFile.
 */
bool onlineJobModel::removeRows( int row, int count, const QModelIndex & parent )
{ 
  if (parent.isValid())
    return false;

  Q_ASSERT( m_jobIdList.count() > row );
  Q_ASSERT( m_jobIdList.count() >= (row+count) );
  
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyFileTransaction transaction;
  for( int i=0; i < count; ++i ) {
    const onlineJob job = file->getOnlineJob( m_jobIdList[row+i] );
    file->removeOnlineJob(job);
  }
  transaction.commit();
  return true;
}

void onlineJobModel::slotObjectAdded(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj)
{
  if ( Q_LIKELY(objType != MyMoneyFile::notifyOnlineJob) )
    return;
  beginInsertRows(QModelIndex(), rowCount(), rowCount());
  m_jobIdList.append(obj->id());
  endInsertRows();
  emit dataChanged(index(rowCount()-1, 0, QModelIndex()), index(rowCount()-1, columnCount()-1, QModelIndex()));
}

void onlineJobModel::slotObjectModified(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj)
{
  if ( Q_LIKELY(objType != MyMoneyFile::notifyOnlineJob) )
    return;

  int row = m_jobIdList.indexOf(obj->id());
  if (row != -1)
    emit dataChanged(index(row, 0), index(row, columnCount()-1));
}

void onlineJobModel::slotObjectRemoved(MyMoneyFile::notificationObjectT objType, const QString& id)
{
  if ( Q_LIKELY(objType != MyMoneyFile::notifyOnlineJob) )
    return;

  int row = m_jobIdList.indexOf(id);
  if (row != -1) {
    m_jobIdList.removeAll(id);
    beginRemoveRows(QModelIndex(), row, row);
    endRemoveRows();
    emit dataChanged(index(row, 0), index(row, columnCount()-1));
  }
}
