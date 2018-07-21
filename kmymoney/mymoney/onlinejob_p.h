/*
 * Copyright 2013-2015  Christian Dávid <christian-david@web.de>
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

#ifndef ONLINEJOB_P_H
#define ONLINEJOB_P_H

#include "onlinejob.h"

#include <QDateTime>
#include <QHash>
#include <QMap>

#include "mymoneyobject_p.h"
#include "onlinejobmessage.h"
#include "mymoneyenums.h"

namespace eMyMoney { namespace OnlineJob { enum class sendingState; } }

class onlineJobPrivate : public MyMoneyObjectPrivate
{
public:
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
  eMyMoney::OnlineJob::sendingState m_jobBankAnswerState;

  /**
   * @brief Validation result status
   */
  QList<onlineJobMessage> m_messageList;

  /**
   * @brief Locking state
   */
  bool m_locked;
};

#endif
