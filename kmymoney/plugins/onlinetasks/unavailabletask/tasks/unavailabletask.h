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

#ifndef UNAVAILABLETASK_H
#define UNAVAILABLETASK_H

#include "onlinetasks/interfaces/tasks/onlinetask.h"

/**
 * @brief Task which can be used if original task is unavailable
 *
 * This task simply stores the XML data given to it and can write it back.
 *
 * The XML storage backend needs to load all tasks into memory. To prevent
 * data corruption if the original task cannot be loaded, this task can be used.
 */
class unavailableTask : public onlineTask
{
public:
  ONLINETASK_META(unavailableTask, "org.kmymoney.onlineTask.unavailableTask");
  virtual bool isValid() const;
  virtual QString jobTypeName() const;

  /**
   * @name SqlMethods
   * @{
   * For sql databases this plugin is not needed nor used. So these functions
   * do not have a real implementation.
   */
  virtual QString storagePluginIid() const;
  virtual bool sqlSave(QSqlDatabase databaseConnection, const QString& onlineJobId) const;
  virtual bool sqlModify(QSqlDatabase databaseConnection, const QString& onlineJobId) const;
  virtual bool sqlRemove(QSqlDatabase databaseConnection, const QString& onlineJobId) const;
  virtual onlineTask* createFromSqlDatabase(QSqlDatabase connection, const QString& onlineJobId) const;
  /** @} */

protected:
  virtual QString responsibleAccount() const;
  virtual unavailableTask* createFromXml(const QDomElement& element) const;
  virtual void writeXML(QDomDocument& document, QDomElement& parent) const;
  virtual bool hasReferenceTo(const QString& id) const;
  virtual unavailableTask* clone() const;

private:
  explicit unavailableTask(const QDomElement& element);

  /**
   * The data received by createFromXml(). Written back by writeXML().
   */
  QDomElement m_data;
};

#endif // UNAVAILABLETASK_H
