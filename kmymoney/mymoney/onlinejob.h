/*
  This file is part of KMyMoney, A Personal Finance Manager by KDE
  Copyright (C) 2013 Christian Dávid <christian-david@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef ONLINEJOB_H
#define ONLINEJOB_H

#include "mymoneyobject.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "onlinejobmessage.h"

class onlineTask;

/**
 * @brief Class to share jobs which can be procceded by an online banking plugin
 *
 * This class stores only the status information and a pointer to an @r onlineTask which stores
 * the real data. So onlineJob is similar to an shared pointer.
 *
 * If you know the type of the onlineTask, @r onlineJobTyped is the first choice to use.
 *
 * It is save to use because accesses to pointers (e.g. task() ) throw an execption if onlineJob is null.
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
class KMM_MYMONEY_EXPORT onlineJob : public MyMoneyObject
{
  Q_GADGET
  KMM_MYMONEY_UNIT_TESTABLE

public:
  enum elNameE { enOnlineTask };
  Q_ENUM(elNameE)

  enum attrNameE { anSend, anBankAnswerDate, anBankAnswerState, anIID,
                   anAbortedByUser, anAcceptedByBank, anRejectedByBank, anSendingError
                 };
  Q_ENUM(attrNameE)

  /**
   * @brief Contructor for null onlineJobs
   *
   * A onlineJob which is null cannot become valid again.
   * @see isNull()
   */
  onlineJob();

  /**
   * @brief Default construtor
   *
   * The onlineJob takes ownership of the task. The task is deleted in the destructor.
   */
  onlineJob(onlineTask* task, const QString& id = MyMoneyObject::m_emptyId);

  /** @brief Copy constructor */
  onlineJob(onlineJob const& other);

  /**
   * @brief Create new onlineJob as copy of other
   *
   * This constructor does not copy the status information but the task only.
   */
  onlineJob(const QString &id, const onlineJob& other);

  /** @brief Contruct from xml */
  onlineJob(const QDomElement&);

  virtual ~onlineJob();

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
  const onlineTask* constTask() const {
    return task();
  }

  /**
   * @brief Returns task of type T attached to this onlineJob
   *
   * Internaly a dynamic_cast is done and the result is checked.
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
  virtual bool hasReferenceTo(const QString &id) const;
  virtual void writeXML(QDomDocument &document, QDomElement &parent) const;

  /**
   * @brief The state of a job given by the onlinePlugin
   */
  enum sendingState {
    noBankAnswer, /**< Used during or before sending or if sendDate().isValid() the job was successfully sent */
    acceptedByBank, /**< bank definetly confirmed the job */
    rejectedByBank, /**< bank definetly rejected this job */
    abortedByUser, /**< aborted by user during sending */
    sendingError /**< an error occurred, the job is certainly not executed by the bank */
  };

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
  virtual bool isNull() const {
    return (m_task == 0);
  }

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
  virtual QDateTime sendDate() const {
    return m_jobSend;
  }

  /**
   * @brief Mark this job as send
   *
   * To be used by online plugin only!
   *
   * Set dateTime to QDateTime to mark unsend.
   */
  virtual void setJobSend(const QDateTime &dateTime = QDateTime::currentDateTime());

  /**
   * @brief The bank's answer to this job
   *
   * To be used by online plugin only!
   *
   * Set dateTime to QDateTime() and bankAnswer to noState to mark unsend. If bankAnswer == noState dateTime.isNull() must be true!
   */
  void setBankAnswer(const sendingState sendingState, const QDateTime &dateTime = QDateTime::currentDateTime());

  /**
   * @brief DateTime of the last status update by the bank
   *
   */
  QDateTime bankAnswerDate() const {
    return m_jobBankAnswerDate;
  }

  /**
   * @brief Returns last status sand by bank
   * @return
   */
  sendingState bankAnswerState() const {
    return m_jobBankAnswerState;
  }

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
  virtual bool isLocked() const {
    return m_locked;
  }

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
  virtual void addJobMessage(const onlineJobMessage &message);

  /**
   * @brief Convenient method to set add a log message
   */
  virtual void addJobMessage(const onlineJobMessage::messageType& type, const QString& sender, const QString& message, const QString& errorCode = QString(), const QDateTime& timestamp = QDateTime::currentDateTime());

  /**
   * @brief jobMessageList
   * @return
   */
  virtual QList<onlineJobMessage> jobMessageList() const;

  onlineJob operator =(const onlineJob&);

  /**
   * @brief Thrown if a cast of a task fails
   *
   * This is inspired by std::bad_cast
   */
  class badTaskCast : public MyMoneyException
  {
  public:
    badTaskCast(const QString& file = "", const long unsigned int& line = 0)
        : MyMoneyException("Casted onlineTask with wrong type", file, line) {}
  };

  /**
   * @brief Thrown if a task of an invalid onlineJob is requested
   */
  class emptyTask : public MyMoneyException
  {
  public:
    emptyTask(const QString& file = "", const long unsigned int& line = 0)
        : MyMoneyException("Requested onlineTask of onlineJob without any task", file, line) {}
  };

  /** @brief onlineTask attatched to this job */
  onlineTask* m_task;

private:

  /**
   * @brief Date-time the job was sent to the bank
   *
   * This does not mean an answer was given by the bank
   */
  QDateTime m_jobSend;

  /**
   * @brief Date-time of confirmation/rejection of the bank
   *
   * which state this timestamp belongs to is stored in m_jobBankAnswerState
   */
  QDateTime m_jobBankAnswerDate;

  /**
   * @brief Answer of the bank
   *
   * combined with m_jobBankAnswerDate
   */
  sendingState m_jobBankAnswerState;

  /**
   * @brief Validation result status
   */
  QList<onlineJobMessage> m_messageList;

  /**
   * @brief Locking state
   */
  bool m_locked;

  /** @brief Copies stored pointers (used by copy constructors) */
  inline void copyPointerFromOtherJob(const onlineJob& other);

  static const QString getElName(const elNameE _el);
  static const QString getAttrName(const attrNameE _attr);
};

template<class T>
T* onlineJob::task()
{
  T* ret = dynamic_cast<T*>(m_task);
  if (ret == 0)
    throw badTaskCast(__FILE__, __LINE__);
  return ret;
}

template<class T>
const T* onlineJob::task() const
{
  const T* ret = dynamic_cast<const T*>(m_task);
  if (ret == 0)
    throw badTaskCast(__FILE__, __LINE__);
  return ret;
}

Q_DECLARE_METATYPE(onlineJob)

#endif // ONLINEJOB_H
