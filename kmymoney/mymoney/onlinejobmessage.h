/*
 * SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ONLINEJOBMESSAGE_H
#define ONLINEJOBMESSAGE_H

#include "kmm_mymoney_export.h"

#include <qglobal.h>

class QDateTime;

namespace eMyMoney { namespace OnlineJob { enum class MessageType; } }

/**
 * @brief Represents a log message for onlineJobs
 */
class onlineJobMessagePrivate;
class KMM_MYMONEY_EXPORT onlineJobMessage
{
  Q_DECLARE_PRIVATE(onlineJobMessage)
  onlineJobMessagePrivate * d_ptr;

public:
  explicit onlineJobMessage(eMyMoney::OnlineJob::MessageType type,
                            QString sender,
                            QString message,
                            QDateTime timestamp);

  explicit onlineJobMessage(eMyMoney::OnlineJob::MessageType type,
                            QString sender,
                            QString message);

  onlineJobMessage(const onlineJobMessage & other);
  onlineJobMessage(onlineJobMessage && other);
  onlineJobMessage & operator=(onlineJobMessage other);
  friend void swap(onlineJobMessage& first, onlineJobMessage& second);
  ~onlineJobMessage();

  bool isDebug() const;
  bool isLog() const;
  bool isInformation() const;
  bool isWarning() const;
  bool isError() const;
  bool isPersistant() const;

  /** @see messageType */
  eMyMoney::OnlineJob::MessageType type() const;

  /**
   * @brief Who "wrote" this message?
   *
   * Could be "OnlinePlugin" or "Bank"
   */
  QString sender() const;

  /**
   * @brief What happend?
   */
  QString message() const;

  /** @brief DateTime of message */
  QDateTime timestamp() const;

  /**
   * @brief Set an error code of the plugin
   */
  void setSenderErrorCode(const QString& errorCode);
  QString senderErrorCode();

private:
  onlineJobMessage();
};

inline void swap(onlineJobMessage& first, onlineJobMessage& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
}

inline onlineJobMessage::onlineJobMessage(onlineJobMessage && other) : onlineJobMessage() // krazy:exclude=inline
{
  swap(*this, other);
}

inline onlineJobMessage & onlineJobMessage::operator=(onlineJobMessage other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

#endif // ONLINEJOBMESSAGE_H
