/*
  This file is part of KMyMoney, A Personal Finance Manager for KDE
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

#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "onlinetasks/interfaces/tasks/credittransfer.h"

#include "onlinejobadministration.h"

onlineJob::onlineJob()
  : MyMoneyObject(),
    m_task( 0 ),
    m_jobSend( QDateTime() ),
    m_jobBankAnswerDate( QDateTime() ),
    m_jobBankAnswerState( noBankAnswer ),
    m_messageList( QList<onlineJobMessage>() ),
    m_locked( false )
{
}

onlineJob::onlineJob(onlineTask* task, const QString &id)
  : MyMoneyObject( id ),
    m_task( task ),
    m_jobSend( QDateTime() ),
    m_jobBankAnswerDate( QDateTime() ),
    m_jobBankAnswerState( noBankAnswer ),
    m_messageList( QList<onlineJobMessage>() ),
    m_locked( false )
{
}

onlineJob::onlineJob( onlineJob const& other )
  : MyMoneyObject( other.id() ),
    m_task( 0 ),
    m_jobSend ( other.m_jobSend ),
    m_jobBankAnswerDate( other.m_jobBankAnswerDate ),
    m_jobBankAnswerState( other.m_jobBankAnswerState),
    m_messageList( other.m_messageList ),
    m_locked( other.m_locked )
{
  copyPointerFromOtherJob(other);
}

onlineJob::onlineJob( const QString &id, const onlineJob& other )
  : MyMoneyObject( id ),
    m_task(),
    m_jobSend( QDateTime() ),
    m_jobBankAnswerDate( QDateTime() ),
    m_jobBankAnswerState( noBankAnswer ),
    m_messageList( QList<onlineJobMessage>() ),
    m_locked( false )
{
  copyPointerFromOtherJob(other);
}

onlineJob::onlineJob(const QDomElement& element)
  : MyMoneyObject(element, true),
    m_messageList( QList<onlineJobMessage>() ),
    m_locked( false )
{
  m_jobSend = QDateTime::fromString(element.attribute("send", ""), Qt::ISODate);
  m_jobBankAnswerDate = QDateTime::fromString(element.attribute("bankAnswerDate", ""), Qt::ISODate);
  QString state = element.attribute("bankAnswerState", "");
  if ( state == "abortedByUser")
    m_jobBankAnswerState = abortedByUser;
  else if ( state == "acceptedByBank")
    m_jobBankAnswerState = acceptedByBank;
  else if ( state == "rejectedByBank")
    m_jobBankAnswerState = rejectedByBank;
  else if ( state == "sendingError")
    m_jobBankAnswerState = sendingError;
  else
    m_jobBankAnswerState = noBankAnswer;

  QDomElement taskElem = element.firstChildElement("onlineTask");
  m_task = onlineJobAdministration::instance()->createOnlineTaskByXml(taskElem.attribute("iid", ""), taskElem);
}

void onlineJob::copyPointerFromOtherJob(const onlineJob &other)
{
  if (!other.isNull())
    m_task = other.constTask()->clone();
}

onlineJob onlineJob::operator = (const onlineJob& other)
{
  if ( this == &other )
    return *this;
  delete m_task;
  m_id = other.m_id;
  m_jobSend = other.m_jobSend;
  m_jobBankAnswerDate = other.m_jobBankAnswerDate;
  m_jobBankAnswerState = other.m_jobBankAnswerState;
  m_messageList = other.m_messageList;
  m_locked = other.m_locked;
  copyPointerFromOtherJob(other);
  return *this;
}

void onlineJob::reset()
{
  clearId();
  m_jobSend = QDateTime();
  m_jobBankAnswerDate = QDateTime();
  m_jobBankAnswerState = noBankAnswer;
  m_locked = false;
}

onlineJob::~onlineJob()
{
  delete m_task;
}

onlineTask* onlineJob::task()
{
  if( m_task == 0 )
    throw emptyTask(__FILE__, __LINE__);
  return m_task;
}

const onlineTask* onlineJob::task() const
{
  if( m_task == 0 )
    throw emptyTask(__FILE__, __LINE__);
  return m_task;
}

QString onlineJob::taskIid() const
{
  try {
    return task()->taskName();
  } catch ( const emptyTask& ) {
  }
  return QString();
}

QString onlineJob::responsibleAccount() const {
    try {
      return task()->responsibleAccount();
    } catch ( const emptyTask& ) {
    }
    return QString();
}

MyMoneyAccount onlineJob::responsibleMyMoneyAccount() const
{
  QString accountId = responsibleAccount();
  if ( !accountId.isEmpty() )
    return MyMoneyFile::instance()->account( accountId );

  return MyMoneyAccount();
}

bool onlineJob::setLock(bool enable)
{
  m_locked = enable;
  return true;
}

bool onlineJob::isEditable() const
{
  return (!isLocked() && sendDate().isNull() && (m_jobBankAnswerState == noBankAnswer || m_jobBankAnswerState == sendingError));
}

void onlineJob::setJobSend( const QDateTime &dateTime )
{
  m_jobSend = dateTime;
}

void onlineJob::setBankAnswer( const sendingState sendingState, const QDateTime &dateTime )
{
  m_jobBankAnswerState = sendingState;
  m_jobBankAnswerDate = dateTime;
}

void onlineJob::addJobMessage(const onlineJobMessage& message)
{
  m_messageList.append(message);
}

void onlineJob::addJobMessage(const onlineJobMessage::messageType& type, const QString& sender, const QString& message, const QString& errorCode, const QDateTime& timestamp)
{
  onlineJobMessage logMessage(type, sender, message, timestamp);
  logMessage.setSenderErrorCode(errorCode);
  m_messageList.append( logMessage );
}

QList<onlineJobMessage> onlineJob::jobMessageList() const
{
  return m_messageList;
}

/** @todo give life */
void onlineJob::writeXML(QDomDocument &document, QDomElement &parent) const
{
  QDomElement el = document.createElement("onlineJob");
  writeBaseXML(document, el);

  if (!m_jobSend.isNull())
    el.setAttribute("send", m_jobSend.toString(Qt::ISODate));
  if (!m_jobBankAnswerDate.isNull())
    el.setAttribute("bankAnswerDate", m_jobBankAnswerDate.toString(Qt::ISODate));

  switch (m_jobBankAnswerState) {
    case abortedByUser: el.setAttribute("bankAnswerState", "abortedByUser"); break;
    case acceptedByBank: el.setAttribute("bankAnswerState", "acceptedByBank"); break;
    case rejectedByBank: el.setAttribute("bankAnswerState", "rejectedByBank"); break;
    case sendingError: el.setAttribute("bankAnswerState", "sendingError"); break;
    case noBankAnswer:
    default: void();
  }

  QDomElement taskEl = document.createElement("onlineTask");
  taskEl.setAttribute("iid", taskIid());
  try {
    task()->writeXML(document, taskEl); // throws execption if there is no task
    el.appendChild(taskEl); // only append child if there is something to append
  } catch ( const emptyTask& ) {
  }

  parent.appendChild(el);
}

bool onlineJob::isValid() const
{
    if ( m_task != 0 )
        return m_task->isValid();
    return false;
}

bool onlineJob::hasReferenceTo(const QString& id) const
{
  if (m_task != 0)
    return m_task->hasReferenceTo(id);
  return false;
}

