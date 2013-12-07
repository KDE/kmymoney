#ifndef ONLINEJOBMESSAGE_H
#define ONLINEJOBMESSAGE_H

#include <QtCore/QString>
#include <QtCore/QDateTime>

/**
 * @brief Represets a log message for onlineJobs
 */
class onlineJobMessage
{
public:
  /**
   * @brief Type of message
   *
   * An usually it is not easy to categorise log messages. This description is only a hint.
   */
  enum messageType {
    debug, /**< Just for debug purposes. In normal scenarios the user should not see this. No need to store this message. Plugins should
      not create them at all if debug mode is not enabled. */
    log, /**< A piece of information the user should not see during normal operation. It is not shown in any UI by default. It is stored persistantly. */
    information, /**< Information that should be kept but without the need to burden the user. The user can
      see this during normal operation. */
    warning, /**< A piece of information the user should see but not be enforced to do so (= no modal dialog). E.g. a task is expected to have
      direct effect but insted you have to wait a day (and that is commen behavior). */
    error /**< Important for the user - he must be warned. E.g. a task could unexpectedly not be executed */
  };

  onlineJobMessage( messageType type, QString sender, QString message, QDateTime timestamp = QDateTime::currentDateTime() )
    : m_type( type ),
      m_sender( sender ),
      m_message( message ),
      m_timestamp( timestamp )
  {}

  ~onlineJobMessage() {}

  bool isDebug() const { return (m_type == debug); }
  bool isLog() const { return (m_type == log); }
  bool isInformation() const { return (m_type == information); }
  bool isWarning() const { return (m_type == warning); }
  bool isError() const { return ( m_type == error ); }

  bool isPersistant() const { return (m_type != debug); }

  /** @see messageType */
  messageType type() const { return m_type; }

  /**
   * @brief Who "wrote" this message?
   *
   * Could be "OnlinePlugin" or "Bank"
   */
  QString sender() const { return m_sender; }

  /**
   * @brief What happend?
   */
  QString message() const { return m_message; }

  /** @brief DateTime of message */
  QDateTime timestamp() const { return m_timestamp; }

private:
  messageType m_type;
  QString m_sender;
  QString m_message;
  QDateTime m_timestamp;

  onlineJobMessage();
};

#endif // ONLINEJOBMESSAGE_H
