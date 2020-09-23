/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright 2014       Christian Dávid <christian-david@web.de>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#include <QDomElement>

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
  bool isValid() const override;
  QString jobTypeName() const override;

  void writeXML(QDomDocument& document, QDomElement& parent) const override;
protected:
  QString responsibleAccount() const override;
  unavailableTask* createFromXml(const QDomElement& element) const override;
  bool hasReferenceTo(const QString& id) const override;

  /**
   * @copydoc MyMoneyObject::referencedObjects
   */
  QSet<QString> referencedObjects() const override;

  unavailableTask* clone() const override;

private:
  explicit unavailableTask(const QDomElement& element);

  /**
   * The data received by createFromXml(). Written back by writeXML().
   */
  QDomElement m_data;
};

#endif // UNAVAILABLETASK_H
