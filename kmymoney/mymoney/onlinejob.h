/*
 * Copyright 2013-2015  Christian Dávid <christian-david@web.de>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ONLINEJOB_H
#define ONLINEJOB_H

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define BADTASKEXCEPTION badTaskCast("Casted onlineTask with wrong type. " __FILE__ ":" TOSTRING(__LINE__))
#define EMPTYTASKEXCEPTION emptyTask("Requested onlineTask of onlineJob without any task. " __FILE__ ":" TOSTRING(__LINE__))

#include <stdexcept>
#include <QMetaType>
#include <QString>
#include "mymoneyobject.h"
#include "onlinejobmessage.h"

class onlineTask;
class MyMoneyAccount;

namespace eMyMoney { namespace OnlineJob { enum class sendingState; } }

/**
 * @brief Class to share jobs which can be processed by an online banking plugin
 *
 * This class stores only the status information and a pointer to an @r onlineTask which stores
 * the real data. So onlineJob is similar to an shared pointer.
 *
 * If you know the type of the onlineTask, @r onlineJobTyped is the first choice to use.
 *
 * It is save to use because accesses to pointers (e.g. task() ) throw an exception if onlineJob is null.
 *
 * Online jobs are usually not created directly but over @r onlineJobAdministration::createOnlineJob. This is
 * required to allow loading of onlineTasks at runtime and only if needed.
 *
 * This class was created to help writing stable and reliable code. Before an unsafe structure (= pointer)
 * is accessed it is checked. Exceptions are thrown if the content is unsafe.
 *
 * @see onlineTask
 * @see onlineJobTyped
 * @todo LOW make data implicitly shared
 */
class onlineJobPrivate;
class KMM_MYMONEY_EXPORT onlineJob : public MyMoneyObject
{
  Q_DECLARE_PRIVATE(onlineJob)

  KMM_MYMONEY_UNIT_TESTABLE

public:
   /**
   * @brief Constructor for null onlineJobs
   *
   * A onlineJob which is null cannot become valid again.
   * @see isNull()
   */
  onlineJob();
  explicit onlineJob(const QString &id);

  /**
   * @brief Default constructor
   *
   * The onlineJob takes ownership of the task. The task is deleted in the destructor.
   */
  onlineJob(onlineTask* task, const QString& id); // krazy:exclude=explicit
  onlineJob(onlineTask* task); // krazy:exclude=explicit

  /**
   * @brief Create new onlineJob as copy of other
   *
   * This constructor does not copy the status information but the task only.
   */
  onlineJob(const QString &id,
            const onlineJob& other);

  onlineJob(const onlineJob & other);
  onlineJob(onlineJob && other);
  onlineJob & operator=(onlineJob other);
  friend void swap(onlineJob& first, onlineJob& second);

  virtual ~onlineJob();

  void setTask(onlineTask *task);

  /**
   * @brief Returns task attached to this onlineJob
   *
   * You should not store this pointer but use onlineJob::task() (or @r onlineJobTyped::task())
   * every time you access it.
   *
   * @note The return type may change in future (e.g. to an atomic pointer). But you can always expect
   * the operator @c -> to work like it does for onlineTask*.
   *
   * @throws emptyTask if isNull()
   */
  onlineTask* task();

  /** @copydoc task(); */
  const onlineTask* task() const;

  /**
   * @brief Returns task attached to this onlineJob as const
   * @throws emptyTask if isNull()
   */
  const onlineTask* constTask() const;

  /**
   * @brief Returns task of type T attached to this onlineJob
   *
   * Internally a dynamic_cast is done and the result is checked.
   *
   * @throws emptyTask if isNull()
   * @throws badTaskCast if attached task cannot be casted to T
   */
  template<class T> T* task();

  /** @copydoc task() */
  template<class T> const T* task() const;
  template<class T> const T* constTask() const {
    return task<T>();
  }

  template<class T> bool canTaskCast() const;

  QString taskIid() const;

  /** @todo implement */
  bool hasReferenceTo(const QString &id) const override;

  /**
   * @copydoc MyMoneyObject::referencedObjects
   */
  QSet<QString> referencedObjects() const override;

  /**
   * @brief Account this job is related to
   *
   * Each job must have an account on which the job operates. This is used to determine
   * the correct onlinePlugin which can execute this job. If the job is related to more
   * than one account (e.g. a password change) select a random one.
   *
   * @return accountId or QString() if none is set or job isNull.
   */
  virtual QString responsibleAccount() const;

  /**
   * @brief Returns the MyMoneyAccount this job is related to
   * @see responsibleAccount()
   */
  MyMoneyAccount responsibleMyMoneyAccount() const;

  /**
   * @brief Check if this onlineJob is editable by the user
   *
   * A job is no longer editable by the user if it is used for documentary purposes
   * e.g. the job was sent to the bank. In that case create a new job based on the
   * old one.
   *
   * @todo make it possible to use onlineJobs as templates
   */
  virtual bool isEditable() const;

  /**
   * @brief Checks if this onlineJob has an attached task
   *
   * @return true if no task is attached to this job
   */
  virtual bool isNull() const;

  /**
   * @brief Checks if an valid onlineTask is attached
   *
   * @return true if task().isValid(), false if isNull() or !task.isValid()
   */
  virtual bool isValid() const;

  /**
   * @brief DateTime the job was sent to the bank
   *
   * A valid return does not mean that this job was accepted by the bank.
   *
   * @return A valid QDateTime if send to bank, an QDateTime() if not send.
   */
  virtual QDateTime sendDate() const;

  /**
   * @brief Mark this job as send
   *
   * To be used by online plugin only!
   *
   * Set dateTime to QDateTime to mark unsend.
   */
  virtual void setJobSend(const QDateTime &dateTime);
  virtual void setJobSend();

  /**
   * @brief The bank's answer to this job
   *
   * To be used by online plugin only!
   *
   * Set dateTime to QDateTime() and bankAnswer to noState to mark unsend. If bankAnswer == noState dateTime.isNull() must be true!
   */
  void setBankAnswer(const eMyMoney::OnlineJob::sendingState state, const QDateTime &dateTime);
  void setBankAnswer(const eMyMoney::OnlineJob::sendingState state);

  /**
   * @brief DateTime of the last status update by the bank
   *
   */
  QDateTime bankAnswerDate() const;

  /**
   * @brief Returns last status sand by bank
   * @return
   */
  eMyMoney::OnlineJob::sendingState bankAnswerState() const;

  /**
   * @brief locks the onlineJob for sending it
   *
   * Used when the job is in sending process by the online plugin.
   *
   * A locked onlineJob cannot be removed from the storage.
   *
   * @note The onlineJob can still be edited and stored. But it should be done by
   * the one how owns the lock only.
   *
   * @todo Enforce the lock somehow? Note: the onlinePlugin must still be able to
   * write to the job.
   *
   * @param enable true locks the job, false unlocks the job
   */
  virtual bool setLock(bool enable = true);

  /**
   * @brief Get lock status
   */
  virtual bool isLocked() const;

  /**
   * @brief Make this onlineJob a "new" onlineJob
   *
   * Removes all status information, log, and the id. Only
   * the task is keept.
   */
  virtual void reset();

  /**
   * @brief addJobMessage
   *
   * To be used by online plugin only.
   * @param message
   */
  void addJobMessage(const onlineJobMessage &message);

  /**
   * @brief Convenient method to set add a log message
   */
  void addJobMessage(const eMyMoney::OnlineJob::MessageType& type, const QString& sender, const QString& message, const QString& errorCode, const QDateTime& timestamp);
  void addJobMessage(const eMyMoney::OnlineJob::MessageType& type, const QString& sender, const QString& message, const QString& errorCode);
  void addJobMessage(const eMyMoney::OnlineJob::MessageType& type, const QString& sender, const QString& message);


  /**
   * @brief jobMessageList
   * @return
   */
  virtual QList<onlineJobMessage> jobMessageList() const;


  void clearJobMessageList();

  /**
   * @brief Thrown if a cast of a task fails
   *
   * This is inspired by std::bad_cast
   */
  class badTaskCast : public std::runtime_error
  {
  public:
    explicit badTaskCast(const char *msg) : std::runtime_error(msg) {}  // krazy:exclude=inline
  };

  /**
   * @brief Thrown if a task of an invalid onlineJob is requested
   */
  class emptyTask : public std::runtime_error
  {
  public:
    explicit emptyTask(const char *msg) : std::runtime_error(msg) {}  // krazy:exclude=inline
  };

  /** @brief onlineTask attached to this job */
  onlineTask* m_task;

private:

  /** @brief Copies stored pointers (used by copy constructors) */
  inline void copyPointerFromOtherJob(const onlineJob& other);
};

inline void swap(onlineJob& first, onlineJob& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
  swap(first.m_task, second.m_task);
}

inline onlineJob::onlineJob(onlineJob && other) : onlineJob() // krazy:exclude=inline
{
  swap(*this, other);
}

inline onlineJob & onlineJob::operator=(onlineJob other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

template<class T>
T* onlineJob::task()
{
  T* ret = dynamic_cast<T*>(m_task);
  if (ret == 0)
    throw EMPTYTASKEXCEPTION;
  return ret;
}

template<class T>
const T* onlineJob::task() const
{
  const T* ret = dynamic_cast<const T*>(m_task);
  if (ret == 0)
    throw BADTASKEXCEPTION;
  return ret;
}

Q_DECLARE_METATYPE(onlineJob)

#endif // ONLINEJOB_H
