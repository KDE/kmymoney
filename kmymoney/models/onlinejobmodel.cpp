#include "onlinejobmodel.h"

#include <QDebug>
#include <QTimer>

#include <KIcon>
#include <KLocalizedString>

#include "mymoneyutils.h"
#include "mymoney/onlinetransfer.h"
#include "mymoney/onlinejobknowntask.h"


onlineJobModel::onlineJobModel(QObject *parent) :
    QAbstractTableModel(parent)
{
  MyMoneyFile *file = MyMoneyFile::instance();
  connect(file, SIGNAL( objectAdded(MyMoneyFile::notificationObjectT,MyMoneyObject*const) ),
          this, SLOT(slotObjectAdded(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(file, SIGNAL(objectModified(MyMoneyFile::notificationObjectT,MyMoneyObject*const)),
          this, SLOT(slotObjectModified(MyMoneyFile::notificationObjectT,MyMoneyObject*const)));
  connect(file, SIGNAL(objectRemoved(MyMoneyFile::notificationObjectT,QString)),
          this, SLOT(slotObjectRemoved(MyMoneyFile::notificationObjectT,QString)));

  foreach(const onlineJob job, MyMoneyFile::instance()->onlineJobList()) {
    m_jobIdList.append(job.id());
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

  const onlineJob job = MyMoneyFile::instance()->getOnlineJob( m_jobIdList[index.row()] );
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
    case onlineJob::abortedByUser: return KIcon("task-reject");
    case onlineJob::rejectedByBank: return KIcon("task-attention");
    case onlineJob::noBankAnswer: break;
    }
    if (job.sendDate().isValid()) {
      return KIcon("task-accepted");
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
    beginRemoveRows(QModelIndex(), row, row);
    m_jobIdList.removeAll(id);
    endRemoveRows();
    emit dataChanged(index(row, 0), index(row, columnCount()-1));
  }
}
