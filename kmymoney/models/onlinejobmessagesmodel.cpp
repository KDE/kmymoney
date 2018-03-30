/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2015 Christian Dávid <christian-david@web.de>
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
    case 1: switch (role) { // Origin column
        case Qt::DisplayRole: return messages[index.row()].sender();
        default: return QVariant();
      }
    case 2: switch (role) { // Message column
        case Qt::DisplayRole: return messages[index.row()].message();
        default: return QVariant();
      }
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
