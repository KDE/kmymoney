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

#ifndef DATABASESTOREABLEOBJECT_H
#define DATABASESTOREABLEOBJECT_H

#include <qobject.h>
#include <QSqlDatabase>

/**
 * @brief Base class for plugin based objects which are stored in the database
 */
class databaseStoreableObject
{
public:

  /**
   * Iid of the KMyMoneyPlugin::storagePlugin which is needed to save this
   * object.
   *
   * Can be QString() to signal that no such plugin is needed.
   */
  virtual QString storagePluginIid() const = 0;

  virtual bool sqlSave(QSqlDatabase databaseConnection, const QString& objectId) const = 0;
  virtual bool sqlModify(QSqlDatabase databaseConnection, const QString& objectId) const = 0;
  virtual bool sqlRemove(QSqlDatabase databaseConnection, const QString& objectId) const = 0;

  virtual ~databaseStoreableObject() {}
};

Q_DECLARE_INTERFACE(databaseStoreableObject, "org.kmymoney.databaseStoreableObject")

#endif // DATABASESTOREABLEOBJECT_H
