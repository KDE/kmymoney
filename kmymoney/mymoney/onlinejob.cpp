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

#include "onlinejob.h"
#include "onlinejob_p.h"

#include "mymoneyfile.h"
#include "mymoneyaccount.h"

#include "tasks/onlinetask.h"
#include "onlinejobadministration.h"
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;

onlineJob::onlineJob() :
  MyMoneyObject(),
  d_ptr(new onlineJobPrivate),
  m_task(0)
{
  Q_D(onlineJob);
  d->m_jobSend = QDateTime();
  d->m_jobBankAnswerDate = QDateTime();
  d->m_jobBankAnswerState = noBankAnswer;
  d->m_messageList = QList<onlineJobMessage>();
  d->m_locked = false;
}

onlineJob::onlineJob(onlineTask* task, const QString &id) :
  MyMoneyObject(id),
  d_ptr(new onlineJobPrivate),
  m_task(task)
{
  Q_D(onlineJob);
  d->m_jobSend = QDateTime();
  d->m_jobBankAnswerDate = QDateTime();
  d->m_jobBankAnswerState = noBankAnswer;
  d->m_messageList = QList<onlineJobMessage>();
  d->m_locked = false;
}

onlineJob::onlineJob(onlineTask* task) :
    MyMoneyObject(MyMoneyObject::m_emptyId),
    d_ptr(new onlineJobPrivate),
    m_task(task)
{
  Q_D(onlineJob);
  d->m_jobSend = QDateTime();
  d->m_jobBankAnswerDate = QDateTime();
  d->m_jobBankAnswerState = noBankAnswer;
  d->m_messageList = QList<onlineJobMessage>();
  d->m_locked = false;
}

onlineJob::onlineJob(onlineJob const& other) :
  MyMoneyObject(other.id()),
  d_ptr(new onlineJobPrivate(*other.d_func())),
  m_task(0)
{
  copyPointerFromOtherJob(other);
}

onlineJob::onlineJob(const QString &id, const onlineJob& other) :
  MyMoneyObject(id),
  d_ptr(new onlineJobPrivate(*other.d_func())),
  m_task()
{
  Q_D(onlineJob);
  d->m_jobSend = QDateTime();
  d->m_jobBankAnswerDate = QDateTime();
  d->m_jobBankAnswerState = noBankAnswer;
  d->m_messageList = QList<onlineJobMessage>();
  d->m_locked = false;
  copyPointerFromOtherJob(other);
}

onlineJob::onlineJob(const QDomElement& element) :
  MyMoneyObject(element, true),
  d_ptr(new onlineJobPrivate)
{
  Q_D(onlineJob);
  d->m_messageList = QList<onlineJobMessage>();
  d->m_locked = false;
  d->m_jobSend = QDateTime::fromString(element.attribute(d->getAttrName(OnlineJob::Attribute::Send)), Qt::ISODate);
  d->m_jobBankAnswerDate = QDateTime::fromString(element.attribute(d->getAttrName(OnlineJob::Attribute::BankAnswerDate)), Qt::ISODate);
  QString state = element.attribute(d->getAttrName(OnlineJob::Attribute::BankAnswerState));
  if (state == d->getAttrName(OnlineJob::Attribute::AbortedByUser))
    d->m_jobBankAnswerState = abortedByUser;
  else if (state == d->getAttrName(OnlineJob::Attribute::AcceptedByBank))
    d->m_jobBankAnswerState = acceptedByBank;
  else if (state == d->getAttrName(OnlineJob::Attribute::RejectedByBank))
    d->m_jobBankAnswerState = rejectedByBank;
  else if (state == d->getAttrName(OnlineJob::Attribute::SendingError))
    d->m_jobBankAnswerState = sendingError;
  else
    d->m_jobBankAnswerState = noBankAnswer;

  QDomElement taskElem = element.firstChildElement(d->getElName(OnlineJob::Element::OnlineTask));
  m_task = onlineJobAdministration::instance()->createOnlineTaskByXml(taskElem.attribute(d->getAttrName(OnlineJob::Attribute::IID)), taskElem);
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
  d->m_jobBankAnswerState = noBankAnswer;
  d->m_locked = false;
}

onlineJob::~onlineJob()
{
  Q_D(onlineJob);
  delete d;
  delete m_task;
}

onlineTask* onlineJob::task()
{
  if (m_task == 0)
    throw emptyTask(__FILE__, __LINE__);
  return m_task;
}

const onlineTask* onlineJob::task() const
{
  if (m_task == 0)
    throw emptyTask(__FILE__, __LINE__);
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
  return (!isLocked() && sendDate().isNull() && (d->m_jobBankAnswerState == noBankAnswer || d->m_jobBankAnswerState == sendingError));
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

void onlineJob::setBankAnswer(const onlineJob::sendingState sendingState, const QDateTime &dateTime)
{
  Q_D(onlineJob);
  d->m_jobBankAnswerState = sendingState;
  d->m_jobBankAnswerDate = dateTime;
}

void onlineJob::setBankAnswer(const onlineJob::sendingState sendingState)
{
  setBankAnswer(sendingState, QDateTime::currentDateTime());
}

QDateTime onlineJob::bankAnswerDate() const
{
  Q_D(const onlineJob);
  return d->m_jobBankAnswerDate;
}

onlineJob::sendingState onlineJob::bankAnswerState() const
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

/** @todo give life */
void onlineJob::writeXML(QDomDocument &document, QDomElement &parent) const
{
  QDomElement el = document.createElement(nodeNames[nnOnlineJob]);
  writeBaseXML(document, el);


  Q_D(const onlineJob);
  if (!d->m_jobSend.isNull())
    el.setAttribute(d->getAttrName(OnlineJob::Attribute::Send), d->m_jobSend.toString(Qt::ISODate));
  if (!d->m_jobBankAnswerDate.isNull())
    el.setAttribute(d->getAttrName(OnlineJob::Attribute::BankAnswerDate), d->m_jobBankAnswerDate.toString(Qt::ISODate));

  switch (d->m_jobBankAnswerState) {
    case abortedByUser: el.setAttribute(d->getAttrName(OnlineJob::Attribute::BankAnswerState), d->getAttrName(OnlineJob::Attribute::AbortedByUser)); break;
    case acceptedByBank: el.setAttribute(d->getAttrName(OnlineJob::Attribute::BankAnswerState), d->getAttrName(OnlineJob::Attribute::AcceptedByBank)); break;
    case rejectedByBank: el.setAttribute(d->getAttrName(OnlineJob::Attribute::BankAnswerState), d->getAttrName(OnlineJob::Attribute::RejectedByBank)); break;
    case sendingError: el.setAttribute(d->getAttrName(OnlineJob::Attribute::BankAnswerState), d->getAttrName(OnlineJob::Attribute::SendingError)); break;
    case noBankAnswer:
    default: void();
  }

  QDomElement taskEl = document.createElement(d->getElName(OnlineJob::Element::OnlineTask));
  taskEl.setAttribute(d->getAttrName(OnlineJob::Attribute::IID), taskIid());
  try {
    task()->writeXML(document, taskEl); // throws execption if there is no task
    el.appendChild(taskEl); // only append child if there is something to append
  } catch (const emptyTask&) {
  }

  parent.appendChild(el);
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
