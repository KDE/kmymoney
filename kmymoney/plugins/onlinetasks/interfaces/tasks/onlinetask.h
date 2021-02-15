/*

    SPDX-FileCopyrightText: 2013 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#ifndef ONLINETASK_H
#define ONLINETASK_H

#include "onlinetask_interfaces_export.h"

#include <QString>

#include <qobject.h>

class onlineJob;

/**
 * @brief Enables onlineTask meta system
 *
 * Use ONLINETASK_META in your onlineTask derived class to create an onlineTask type iid
 * (the name). This is should be like the plugin iid of Qt5 (ONLINATASK_META was written
 * for Qt4).
 *
 * Given this code
 * @code
   onlineTask* taskA = // unknown;
   onlineTaskB *taskB = new onlineTaskB;
 * @endcode
 *
 * The following constrains are used by KMyMoney:
 * - @c taskA->taskName() == onlineTaskA::name() => dynamic_cast<onlineTaskA*>()
 * - @c dynamic_cast<onlineTaskC*>(taskB) == 0 => taskB->taskName() != onlineTaskC::name()
 *
 * @param onlineTaskSubClass the class type (e.g. onlineTask)
 * @param IID A unique name for the task, should be replaced by IID of Qt5 plugins
 *
 * \section onlineTaskMeta The onlineTask Meta System
 *
 * To prevent accidentally using a super-class if sub-class should be used, the onlineTasks
 * have a meta system. Each onlineTask has an iid which can be requested with a virtual
 * method or a static method.
 *
 * The task @b IID (type @c QString) is constant (even after a restart) and set by the programmer.
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
#define ONLINETASK_META_BASE(onlineTaskClass, IID, ATTRIBUTE) \
  /** @brief Returns the iid of onlineTask type (part of @ref onlineTaskMeta) */ \
  static const QString& name() { \
    static const QString _name = IID; \
    return _name; \
  } \
  /** @brief Returns the iid of onlineTask type (part of @ref onlineTaskMeta) */ \
  virtual QString taskName() const ATTRIBUTE { \
    return onlineTaskClass::name(); \
  } \
  friend class onlineJobAdministration

#define ONLINETASK_META(onlineTaskClass, IID) ONLINETASK_META_BASE(onlineTaskClass, IID, override)

/**
 * @brief Base class for tasks which can be proceeded by online banking plugins
 *
 * @notice This docu describes the intended way of the onlineTask/Job system. The loading during runtime or plugin
 * infrastructure is not realized yet (and needs further changes at the storage). The docu is just forward compatible.
 *
 * Everything an online plugin can do is represented as a task. But also imported data can be represented by an
 * onlineTask.
 *
 * Due to the huge amount of possible onlineTasks they are loaded during runtime. Which also allows a third party
 * online plugin to introduce its own tasks. However tasks are separated from the onlinePlugins to allow more than one
 * plugin to use the same task. Usually you will have an interface and an implementation (which derives from the
 * interface) to enable this.
 *
 * As user of an online Task you use @r onlineJobAdministration::createOnlineJob() to create a task (within an
 * onlineJob).
 *
 * The widgets to edit an onlineTask are created by subclassing @a IonlineJobEdit. To enable KMyMoney to convert
 * one task into another use @a onlineTaskConverter.
 *
 * @important Do not delete onlineTasks or use pointers to onlineTasks directly. Use @r onlineJob or @r onlineJobTyped instead!
 * This prevents common C++ pitfalls.
 *
 * If you inherit onlineTask, take care of clone() and @ref onlineTaskMeta.
 * Maybe you want to look at @ref onlineTask::settings as well.
 *
 * @see onlineJob
 */
class QDomDocument;
class QDomElement;
class ONLINETASK_INTERFACES_EXPORT onlineTask
{
public:
  ONLINETASK_META_BASE(onlineTask, "org.kmymoney.onlineTask", /* no attribute here */);
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

  /** @see MyMoneyObject::writeXML() */
  virtual void writeXML(QDomDocument &document, QDomElement &parent) const = 0;

protected:
  onlineTask(const onlineTask& other);

  /**
   * @brief Copy this instance including inherited information
   *
   * This method copies an onlineJob including all information which are stored in inherited classes
   * even if you do not know the final type of an reference or pointer.
   */
  virtual onlineTask* clone() const = 0;

  /** @see MyMoneyObject::hasReferenceTo() */
  virtual bool hasReferenceTo(const QString &id) const = 0;

  /**
   * @copydoc MyMoneyObject::referencedObjects
   */
  virtual QSet<QString> referencedObjects() const = 0;

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
   * You can make this method public if it is useful for you.
   *
   * @return accountId
   */
  virtual QString responsibleAccount() const = 0;

  friend class onlineJob;
};

Q_DECLARE_INTERFACE(onlineTask, "org.kmymoney.onlinetask");

#endif // ONLINETASK_H
