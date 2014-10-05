/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
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

#ifndef KMYMONEYPLUGIN_STORAGEPLUGIN_H
#define KMYMONEYPLUGIN_STORAGEPLUGIN_H

#include <QString>

namespace KMyMoneyPlugin
{

/**
 * @brief Interface for plugins which store data in the database
 *
 */
class storagePlugin
{
public:
  /**
   * @brief Setup database to make it usable by the plugins
   *
   * Use @c QSqlDatabase::database(connectionName); to get a database connection.
   */
  bool setupDatabase( const QString& connectionName ) = 0;

  /**
   * @brief Remove all data belonging to the plugin from the database
   *
   * Use @c QSqlDatabase::database(connectionName); to get a database connection.
   */
  bool removePluginData( const QString& connectionName ) = 0;
};

} // end namespace KMyMoneyPlugin

#endif // KMYMONEYPLUGIN_STORAGEPLUGIN_H
