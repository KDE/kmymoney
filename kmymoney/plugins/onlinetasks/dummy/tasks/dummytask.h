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

#ifndef DUMMYTASK_H
#define DUMMYTASK_H

#include "onlinetasks/interfaces/tasks/onlinetask.h"

class dummyTask : public onlineTask
{
public:
  ONLINETASK_META(dummyTask, "org.kmymoney.onlinetasks.dummy");
  dummyTask()
      : m_testNumber(0) {

  }

  dummyTask(const dummyTask& other)
      : onlineTask(other),
      m_testNumber(other.m_testNumber) {
  }

  /**
   * @brief Checks if the task is ready for sending
   */
  virtual bool isValid() const {
    return true;
  };

  /**
   * @brief Human readable type-name
   */
  virtual QString jobTypeName() const {
    return QLatin1String("Dummy task");
  };

  void setTestNumber(const int& number) {
    m_testNumber = number;
  }
  int testNumber() {
    return m_testNumber;
  }
  virtual QString storagePluginIid() const {
    return QString();
  }

  virtual bool sqlSave(QSqlDatabase databaseConnection, const QString& onlineJobId) const {
    Q_UNUSED(databaseConnection); Q_UNUSED(onlineJobId); return false;
  }
  virtual bool sqlModify(QSqlDatabase databaseConnection, const QString& onlineJobId) const {
    Q_UNUSED(databaseConnection); Q_UNUSED(onlineJobId); return false;
  }
  virtual bool sqlRemove(QSqlDatabase databaseConnection, const QString& onlineJobId) const {
    Q_UNUSED(databaseConnection); Q_UNUSED(onlineJobId); return false;
  }

protected:

  virtual dummyTask* clone() const {
    return (new dummyTask(*this));
  }
  virtual bool hasReferenceTo(const QString &id) const {
    Q_UNUSED(id); return false;
  }
  virtual void writeXML(QDomDocument&, QDomElement&) const {}
  virtual dummyTask* createFromXml(const QDomElement&) const {
    return (new dummyTask);
  }
  virtual onlineTask* createFromSqlDatabase(QSqlDatabase connection, const QString& onlineJobId) const {
    Q_UNUSED(connection); Q_UNUSED(onlineJobId);  return (new dummyTask);
  }
  virtual QString responsibleAccount() const {
    return QString();
  };

  int m_testNumber;
};

#endif // DUMMYTASK_H
