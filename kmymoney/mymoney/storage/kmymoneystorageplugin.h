/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
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

#include <QObject>
#include <QVariantList>
#include <QtSql/QSqlDatabase>

namespace KMyMoneyPlugin
{

/**
 * @brief Interface for plugins which store data in an sql database
 *
 */
class storagePlugin : public QObject
{
  Q_OBJECT

public:
  /**
   * This is the constructor needed by KService
   */
  explicit storagePlugin(QObject* parent = 0, const QVariantList& options = QVariantList());

  /**
   * @brief Setup database to make it usable by the plugins
   *
   * This method is also called on the first usage in a session. You must check if your changes
   * were applied before. This also enables the plugin to upgrade the schema if needed.
   *
   * You must fill the kmmPluginInfo table with your plugins data. The version column is for your
   * usage. A higher number means a more recent version.
   * Use a transaction!
   */
  virtual bool setupDatabase(QSqlDatabase connection) = 0;

  /**
   * @brief Remove all data belonging to the plugin from the database
   */
  virtual bool removePluginData(QSqlDatabase connection) = 0;

  virtual ~storagePlugin();
};

} // end namespace KMyMoneyPlugin

Q_DECLARE_INTERFACE(KMyMoneyPlugin::storagePlugin, "org.kmymoney.plugin.storageplugin")

#endif // KMYMONEYPLUGIN_STORAGEPLUGIN_H
