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
#include "onlinetask.h"

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
    m_task( 0 ),
    m_jobSend ( other.m_jobSend ),
    m_jobBankAnswerDate( other.m_jobBankAnswerDate ),
    m_jobBankAnswerState( other.m_jobBankAnswerState),
    m_messageList( other.m_messageList ),
    m_locked( other.m_locked )
{
  copyPointerFromOtherJob(other);
}

void onlineJob::copyPointerFromOtherJob(const onlineJob &other)
{
  if (!other.isNull())
    m_task = other.constTask()->clone();
}

onlineJob onlineJob::operator = (const onlineJob& other)
{
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
    throw new emptyTask(__FILE__, __LINE__);
  return m_task;
}

const onlineTask* onlineJob::task() const
{
  if( m_task == 0 )
    throw new emptyTask(__FILE__, __LINE__);
  return m_task;
}

QString onlineJob::responsibleAccount() const {
    try {
      return task()->responsibleAccount();
    } catch ( emptyTask* ) {
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
  return isLocked();
}

void onlineJob::setJobSend( const QDateTime &dateTime )
{
  m_jobSend = dateTime;
}

void onlineJob::setBankAnswer( const sendingState sendingState, const QDateTime &dateTime )
{
  Q_ASSERT( sendingState == abortedByUser || sendingState == rejectedByBank || sendingState == acceptedByBank || sendingState == sendingError );
  m_jobBankAnswerState = sendingState;
  m_jobBankAnswerDate = dateTime;
}

void onlineJob::addJobMessage(const onlineJobMessage& message)
{
  m_messageList.append(message);
}

QList<onlineJobMessage> onlineJob::jobMessageList() const
{
  return m_messageList;
}

/** @todo give life */
void onlineJob::writeXML(QDomDocument &document, QDomElement &parent) const
{
  Q_UNUSED(document);
  Q_UNUSED(parent);
}

