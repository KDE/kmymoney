/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
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

#ifndef ONLINETASK_H
#define ONLINETASK_H

#include <QtCore/QString>
#include <QtCore/QDateTime>

#include "mymoneyaccount.h"
#include "onlinejobmessage.h"

class onlineJob;

/**
 * @brief Enables onlineTask meta system
 *
 * Use ONLINETASK_META in your onlineTask derived class to create an onlineTask type iid
 * (the name). This is should be like the plugin iid of Qt5 (ONLINATASK_META was written
 * for Qt4).
 *
 * @param onlineTaskSubClass the class type (e.g. onlineTask)
 * @param IID A unique name for the task, should be replaced by IID of Qt5 plugins
 *
 * \section onlineTaskMeta The onlineTask Meta System
 *
 * To prevent accidently using a super-class if sub-class should be used, the onlineTasks
 * have a meta system. Each onlineTask has an iid which can be requested with a virtual
 * method or a static method.
 *
 * The task @b IID (type @c QString) is constant after a restart and set by the programmer.
 * \code
 * // get the name of a known type
 * onlineTask::name()
 * // get the name of a pointer
 * onlineTask* unknownTask = new onlineTaskSubClass();
 * unknownTask->taskName();
 * \endcode
 *
 * Activate the meta system using ONLINETASK_META() in your classes public section.
 */
#define ONLINETASK_META(onlineTaskClass, IID) \
/** @brief Returns the iid of onlineTask type (part of @ref onlineTaskMeta) */ \
static const QString& name() { \
  static const QString _name = IID; \
  return _name; \
} \
/** @brief Returns the iid of onlineTask type (part of @ref onlineTaskMeta) */ \
virtual QString taskName() const { \
  return onlineTaskClass::name(); \
} \
friend class onlineJobAdministration

/**
 * @brief The onlineTask class
 *
 * @notice This docu describes the inteded way of the onlineTask/Job system. The loading during runtime or plugin
 * infrastructure is not realized yet (and needs further changes at the storage). The docu is just forward compatible.
 * 
 * Everything an onlinePlugin can do is represented as a task. Due to the huge amount of possible onlineTasks
 * they are loaded during runtime. Which also allows a third party online plugin to introduce its own
 * tasks. However tasks are seperated from the onlinePlugins to allow more than one plugin to use the same task.
 * 
 * The widgets to edit an onlineTask are created by subclassing @a IonlineJobEdit. To enable KMyMoney to convert
 * one task into another use @a onlineTaskConverter.
 * 
 * @important Do not delete onlineTasks or use pointers to onlineTasks directly. Use onlineJob instead!
 * This prevents common C++ pitfalls.
 * 
 * If you inherit onlineTask, take care of clone() and \ref onlineTaskMeta.
 * Maybe you want to look at @ref onlineTask::settings as well.
 */
class KMM_MYMONEY_EXPORT onlineTask
{
public:
  ONLINETASK_META(onlineTask, "org.kmymoney.onlineTask");

  onlineTask();
  virtual ~onlineTask() {}

  /**
   * @brief Checks if the task is ready for sending
   */
  virtual bool isValid() const = 0;

  /**
   * @brief Human readable type-name
   */
  virtual QString jobTypeName() const = 0;

  /**
   * @brief Account/plugin dependent settings for an onlineTask
   *
   * Many onlineTasks settings vary due to multiple reasons. E.g.
   * a credit transfer could have a maximum amount it can transfer at
   * once. But this amount could depend on the account and the user's
   * contract with the bank.
   *
   * Therefor onlineTasks can offer thier own set of configurations. There
   * is no predifined behavior, only subclass onlineTask::settings.
   * Of course onlinePlugins and widgets which support that task
   * need to know how to handle that specific settings.
   *
   * Using @ref onlineJobAdministration::taskSettings() KMyMoney will
   * request the correct onlinePlugin to create the settings and return
   * them as shared pointer. Please note that KMyMoney will try to reuse
   * that pointer if possible, so do not edit it.
   */
  class settings
  {
  public:
    virtual ~settings();
  };

protected:
  onlineTask( const onlineTask& other );

  /**
   * @brief Copy this instance including inherited information
   *
   * This method copies an onlineJob including all information which are stored in inherited classes
   * even if you do not know the final type of an reference or pointer.
   */
  virtual onlineTask* clone() const = 0;

  /** @see MyMoneyObject::hasReferenceTo() */
  virtual bool hasReferenceTo(const QString &id) const = 0;

  /** @see MyMoneyObject::writeXML() */
  virtual void writeXML(QDomDocument &document, QDomElement &parent) const = 0;
  
  /**
   * @brief Create a new instance of this task based on xml data
   * 
   * This method is used to load an onlineTask from a xml file.
   * 
   * This method is created const as it should create a @emph new onlineTask.
   * @return A pointer to a new instance, caller takes ownership
   */
  virtual onlineTask* createFromXml(const QDomElement &element) const = 0;
  
  /**
   * @brief Account this job is related to
   *
   * Each task must have an account on which it operates. This is used to determine
   * the correct onlinePlugin which can execute this job. If the job is related to more
   * than one account (e.g. a password change) select a random one.
   * 
   * You can make this method public if it is usefull for you.
   * 
   * @return accountId
   */
  virtual QString responsibleAccount() const = 0;

  friend class onlineJob;

private:

};

Q_DECLARE_INTERFACE(onlineTask, "org.kmymoney.onlinetask");

#endif // ONLINETASK_H
