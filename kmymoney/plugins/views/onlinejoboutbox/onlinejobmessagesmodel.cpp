/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
 *
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
 */

#include "onlinejobmessagesmodel.h"

#include <QIcon>
#include <QDateTime>

#include <KLocalizedString>

#include "mymoneyenums.h"
#include "icons/icons.h"

using namespace Icons;

onlineJobMessagesModel::onlineJobMessagesModel(QObject* parent)
    : QAbstractTableModel(parent),
    m_job()
{

}

QVariant onlineJobMessagesModel::data(const QModelIndex& index, int role) const
{
  const QList<onlineJobMessage> messages = m_job.jobMessageList();
  if (index.row() >= messages.count())
    return QVariant();

  switch (index.column()) {
    case 0: switch (role) { // Status/Date column
        case Qt::DisplayRole: return messages[index.row()].timestamp();
        case Qt::DecorationRole: switch (messages[index.row()].type()) {
            case eMyMoney::OnlineJob::MessageType::Debug:
            case eMyMoney::OnlineJob::MessageType::Log:
            case eMyMoney::OnlineJob::MessageType::Information: return Icons::get(Icon::DialogInformation);
            case eMyMoney::OnlineJob::MessageType::Warning: return Icons::get(Icon::DialogWarning);
            case eMyMoney::OnlineJob::MessageType::Error: return Icons::get(Icon::DialogError);
            break;
          }
          break;
        case Qt::ToolTipRole: switch (messages[index.row()].type()) {
            case eMyMoney::OnlineJob::MessageType::Debug: return i18n("Information to find issues.");
            case eMyMoney::OnlineJob::MessageType::Log: return i18n("Information stored for provability.");
            case eMyMoney::OnlineJob::MessageType::Information: return i18n("Informative message without certain significance.");
            case eMyMoney::OnlineJob::MessageType::Warning: return i18n("Warning message.");
            case eMyMoney::OnlineJob::MessageType::Error: return i18n("Error");
            break;
          }
          break;
        default: return QVariant();
      }
      break;
    case 1: switch (role) { // Origin column
        case Qt::DisplayRole: return messages[index.row()].sender();
        default: return QVariant();
      }
      break;
    case 2: switch (role) { // Message column
        case Qt::DisplayRole: return messages[index.row()].message();
        default: return QVariant();
      }
      break;
    default:
      break;
  }

  // Actually we should never get here. But let's make this model bullet proof.
  return QVariant();
}

int onlineJobMessagesModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return 3;
}

int onlineJobMessagesModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return m_job.jobMessageList().count();
}

QModelIndex onlineJobMessagesModel::parent(const QModelIndex&) const
{
  return QModelIndex();
}

QVariant onlineJobMessagesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    switch (section) {
      case 0: switch (role) {
          case Qt::DisplayRole: return i18n("Date");
          default: return QVariant();
        }
      case 1: switch (role) {
          case Qt::DisplayRole: return i18n("Origin");
          default: return QVariant();
        }
      case 2: switch (role) {
          case Qt::DisplayRole: return i18n("Description");
          default: return QVariant();
        }
    }
  }

  return QVariant();
}

void onlineJobMessagesModel::setOnlineJob(const onlineJob& job)
{
  beginResetModel();
  m_job = job;
  endResetModel();
}
