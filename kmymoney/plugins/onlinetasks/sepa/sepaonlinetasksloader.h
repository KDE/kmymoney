/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
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

#ifndef SEPAONLINETASKSLOADER_H
#define SEPAONLINETASKSLOADER_H

#include <kmymoney/plugins/onlinepluginextended.h>

class sepaOnlineTasksLoader : public QObject, public KMyMoneyPlugin::onlineTaskFactory
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::onlineTaskFactory)
public:
  sepaOnlineTasksLoader(QObject* parent = nullptr, const QVariantList& options = QVariantList{});
  virtual onlineTask* createOnlineTask(const QString& taskId) const;

};

#endif // SEPAONLINETASKSLOADER_H
