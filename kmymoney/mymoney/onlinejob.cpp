/*
 * SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "onlinejob.h"
#include "onlinejob_p.h"

#include "mymoneyfile.h"
#include "mymoneyaccount.h"

#include "tasks/onlinetask.h"
#include "onlinejobadministration.h"

onlineJob::onlineJob() :
  MyMoneyObject(*new onlineJobPrivate),
  m_task(0)
{
  Q_D(onlineJob);
  d->m_jobSend = QDateTime();
  d->m_jobBankAnswerDate = QDateTime();
  d->m_jobBankAnswerState = eMyMoney::OnlineJob::sendingState::noBankAnswer;
  d->m_messageList = QList<onlineJobMessage>();
  d->m_locked = false;
}

onlineJob::onlineJob(const QString &id) :
  MyMoneyObject(*new onlineJobPrivate, id),
  m_task(nullptr)
{
}

onlineJob::onlineJob(onlineTask* onlinetask, const QString &id) :
  MyMoneyObject(*new onlineJobPrivate, id),
  m_task(onlinetask)
{
  Q_D(onlineJob);
  d->m_jobSend = QDateTime();
  d->m_jobBankAnswerDate = QDateTime();
  d->m_jobBankAnswerState = eMyMoney::OnlineJob::sendingState::noBankAnswer;
  d->m_messageList = QList<onlineJobMessage>();
  d->m_locked = false;
}

onlineJob::onlineJob(onlineTask* onlinetask) :
    MyMoneyObject(*new onlineJobPrivate, QString()),
    m_task(onlinetask)
{
  Q_D(onlineJob);
  d->m_jobSend = QDateTime();
  d->m_jobBankAnswerDate = QDateTime();
  d->m_jobBankAnswerState = eMyMoney::OnlineJob::sendingState::noBankAnswer;
  d->m_messageList = QList<onlineJobMessage>();
  d->m_locked = false;
}

onlineJob::onlineJob(onlineJob const& other) :
  MyMoneyObject(*new onlineJobPrivate(*other.d_func()), other.id()),
  m_task(0)
{
  copyPointerFromOtherJob(other);
}

onlineJob::onlineJob(const QString &id, const onlineJob& other) :
  MyMoneyObject(*new onlineJobPrivate(*other.d_func()), id),
  m_task()
{
  Q_D(onlineJob);
  d->m_jobSend = QDateTime();
  d->m_jobBankAnswerDate = QDateTime();
  d->m_jobBankAnswerState = eMyMoney::OnlineJob::sendingState::noBankAnswer;
  d->m_messageList = QList<onlineJobMessage>();
  d->m_locked = false;
  copyPointerFromOtherJob(other);
}

void onlineJob::copyPointerFromOtherJob(const onlineJob &other)
{
  if (!other.isNull())
    m_task = other.constTask()->clone();
}

void onlineJob::reset()
{
  Q_D(onlineJob);
  clearId();
  d->m_jobSend = QDateTime();
  d->m_jobBankAnswerDate = QDateTime();
  d->m_jobBankAnswerState = eMyMoney::OnlineJob::sendingState::noBankAnswer;
  d->m_locked = false;
}

onlineJob::~onlineJob()
{
  delete m_task;
}

void onlineJob::setTask(onlineTask *_task)
{
  m_task = _task;
}

onlineTask* onlineJob::task()
{
  if (m_task == 0)
    throw EMPTYTASKEXCEPTION;
  return m_task;
}

const onlineTask* onlineJob::task() const
{
  if (m_task == 0)
    throw EMPTYTASKEXCEPTION;
  return m_task;
}

const onlineTask* onlineJob::constTask() const
{
  return task();
}


QString onlineJob::taskIid() const
{
  try {
    return task()->taskName();
  } catch (const emptyTask&) {
  }
  return QString();
}

QString onlineJob::responsibleAccount() const
{
  try {
    return task()->responsibleAccount();
  } catch (const emptyTask&) {
  }
  return QString();
}

MyMoneyAccount onlineJob::responsibleMyMoneyAccount() const
{
  QString accountId = responsibleAccount();
  if (!accountId.isEmpty())
    return MyMoneyFile::instance()->account(accountId);

  return MyMoneyAccount();
}

bool onlineJob::setLock(bool enable)
{
  Q_D(onlineJob);
  d->m_locked = enable;
  return true;
}

bool onlineJob::isLocked() const
{
  Q_D(const onlineJob);
  return d->m_locked;
}

bool onlineJob::isEditable() const
{
  Q_D(const onlineJob);
  return (!isLocked() && sendDate().isNull() && (d->m_jobBankAnswerState == eMyMoney::OnlineJob::sendingState::noBankAnswer || d->m_jobBankAnswerState == eMyMoney::OnlineJob::sendingState::sendingError));
}

bool onlineJob::isNull() const
{
  return (m_task == 0);
}

void onlineJob::setJobSend(const QDateTime &dateTime)
{
  Q_D(onlineJob);
  d->m_jobSend = dateTime;
}

void onlineJob::setJobSend()
{
  setJobSend(QDateTime::currentDateTime());
}

void onlineJob::setBankAnswer(const eMyMoney::OnlineJob::sendingState state, const QDateTime &dateTime)
{
  Q_D(onlineJob);
  d->m_jobBankAnswerState = state;
  d->m_jobBankAnswerDate = dateTime;
}

void onlineJob::setBankAnswer(const eMyMoney::OnlineJob::sendingState state)
{
  setBankAnswer(state, QDateTime::currentDateTime());
}

QDateTime onlineJob::bankAnswerDate() const
{
  Q_D(const onlineJob);
  return d->m_jobBankAnswerDate;
}

eMyMoney::OnlineJob::sendingState onlineJob::bankAnswerState() const
{
  Q_D(const onlineJob);
  return d->m_jobBankAnswerState;
}

void onlineJob::addJobMessage(const onlineJobMessage& message)
{
  Q_D(onlineJob);
  d->m_messageList.append(message);
}

void onlineJob::addJobMessage(const eMyMoney::OnlineJob::MessageType& type, const QString& sender, const QString& message, const QString& errorCode, const QDateTime& timestamp)
{
  Q_D(onlineJob);
  onlineJobMessage logMessage(type, sender, message, timestamp);
  logMessage.setSenderErrorCode(errorCode);
  d->m_messageList.append(logMessage);
}

void onlineJob::addJobMessage(const eMyMoney::OnlineJob::MessageType& type, const QString& sender, const QString& message, const QString& errorCode)
{
  addJobMessage(type, sender, message, errorCode, QDateTime::currentDateTime());
}

void onlineJob::addJobMessage(const eMyMoney::OnlineJob::MessageType& type, const QString& sender, const QString& message)
{
  addJobMessage(type, sender, message, QString(), QDateTime::currentDateTime());

}

QList<onlineJobMessage> onlineJob::jobMessageList() const
{
  Q_D(const onlineJob);
  return d->m_messageList;
}

void onlineJob::clearJobMessageList()
{
  Q_D(onlineJob);
  d->m_messageList = QList<onlineJobMessage>();
}

bool onlineJob::isValid() const
{
  if (m_task != 0)
    return m_task->isValid();
  return false;
}

QDateTime onlineJob::sendDate() const
{
  Q_D(const onlineJob);
  return d->m_jobSend;
}

bool onlineJob::hasReferenceTo(const QString& id) const
{
  if (m_task != 0)
    return m_task->hasReferenceTo(id);
  return false;
}
