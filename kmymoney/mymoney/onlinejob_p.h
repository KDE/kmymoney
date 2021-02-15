/*
 * SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
