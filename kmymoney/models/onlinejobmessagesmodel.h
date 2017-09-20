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

#ifndef ONLINEJOBMESSAGESMODEL_H
#define ONLINEJOBMESSAGESMODEL_H

#include <QtCore/QAbstractTableModel>

#include "mymoney/onlinejob.h"

class onlineJobMessagesModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit onlineJobMessagesModel(QObject* parent = 0);
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual int columnCount(const QModelIndex& parent) const;
  virtual int rowCount(const QModelIndex& parent) const;
  virtual QModelIndex parent(const QModelIndex& child) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
  void setOnlineJob(const onlineJob& job);

protected:
  onlineJob m_job;
};

#endif // ONLINEJOBMESSAGESMODEL_H
