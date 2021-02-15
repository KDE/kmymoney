/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
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
  unavailableTask* clone() const override;

private:
  explicit unavailableTask(const QDomElement& element);

  /**
   * The data received by createFromXml(). Written back by writeXML().
   */
  QDomElement m_data;
};

#endif // UNAVAILABLETASK_H
