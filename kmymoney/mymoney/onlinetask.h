#ifndef onlineJob_H
#define onlineJob_H

#include <QtCore/QString>
#include <QtCore/QDateTime>

#include "mymoneyaccount.h"
#include "onlinejobmessage.h"

class onlineJob;

/**
 * @brief Enables onlineTask meta system
 *
 * Use ONLINETASK_META in your onlineTask derived class to create an onlineTask type hash
 * and name. Use ONLINETASK_META_INIT() in your .cpp file to init it (important!).
 *
 * @param onlineTaskSubClass the class type (e.g. onlineTask)
 * @param nameString A unique name for the task, should be replaced by IID of Qt5 plugins
 *
 * \section onlineTaskMeta The onlineTask Meta System
 *
 * To prevent accidently using a super-class when sub-class should be used the onlineTasks
 * have a meta system. Each onlineTask has a hash and a name - analogue to C++11's type_index.
 * Both can be ask using a static way and a virtual method.
 *
 * The @b hash of type @c size_t can be requested using
 * \code
 * // get the hash of a known type
 * onlineTask::hash;
 * // get the hash of a pointer
 * onlineTask* unknownTask = new onlineTaskSubClass();
 * unknownTask->taskHash();
 * \endcode
 *
 * The hash is created at startup and only contant during runtime. After a programm
 * restart it might change! You should only use it for testing.
 * It can be used in switchs as well.
 * \code
 * if (unknownTask->taskHash() == onlineTask::hash)
 *    // do something
 * \endcode
 *
 * The (more complex) task @b name (type @c QString) is also constant after a restart and set by the programmer.
 * It is used in communication with the plugins and if persistance is important.
 * \code
 * // get the name of a known type
 * onlineTask::name()
 * // get the name of a pointer
 * onlineTask* unknownTask = new onlineTaskSubClass();
 * unknownTask->taskName();
 * \endcode
 *
 * There is no way to transform a name to a hash or the other way round.
 *
 * Activate the meta system using ONLINETASK_META() and ONLINETASK_META_INIT().
 *
 * @internal I am not proud of this. But I had no better idea without using C++11. Sorry.
 */
#define ONLINETASK_META(onlineTaskClass, nameString) \
/** @brief Returns the hash of onlineTask type (part of @ref onlineTaskMeta) @depreciated */ \
virtual size_t taskHash() const \
{ \
  return ( onlineTaskClass::hash ); \
} \
/** @brief Returns the name of onlineTask type (part of @ref onlineTaskMeta) */ \
static const QString& name() { \
  static const QString _name = nameString; \
  return _name; \
} \
/** @brief Returns the name of onlineTask type (part of @ref onlineTaskMeta) */ \
virtual QString taskName() const { \
  return onlineTaskClass::name(); \
} \
/** @brief The hash of onlineTask type (part of @ref onlineTaskMeta) @depreciated */ \
static const size_t hash; \
friend class onlineJobAdministration

/**
 * @brief Init the onlineTask meta system
 * To be used with ONLINETASK_META().
 *
 * @param onlineTaskClass the class type (e.g. onlineTask)
 *
 * @internal C++11 should make this unnecessary.
 */
#define ONLINETASK_META_INIT(onlineTaskClass) \
  const size_t onlineTaskClass::hash = onlineTask::_internal_getNextMetaHash()

/**
 * @brief The onlineTask class
 *
 * @notice This docu describes the inteded way of the onlineTask/Job system. The loading during runtime or plugin
 * infrastructure is not realized yet (and needs further changes at the storage). The docu is just forward compatible.
 * 
 * Everything an onlinePlugin can do is represented as a task. Due to the huge amount of possible onlineTasks
 * they are loaded during runtime. Which also allows a third party online plugin to introduce it's own
 * tasks. However tasks are seperated from the onlinePlugins to allow more than one plugin to use the same task.
 * 
 * The widgets to edit an onlineTask are created by subclassing IonlineJobEdit.
 * 
 * @important Do not delete onlineTasks or use pointers to onlineTasks directly. Use onlineJob instead!
 * This prevents common C++ pitfalls.
 * 
 * If you inherit onlineTask, take care of clone() and \ref onlineTaskMeta.
 * Maybe you want to look at @ref onlineTask::settings as well.
 */
class KMM_MYMONEY_EXPORT onlineTask
{
public:
  ONLINETASK_META(onlineTask, "org.kmymoney.onlineTask");

  onlineTask();
  virtual ~onlineTask() {}

  /**
   * @brief Checks if the task is ready for sending
   */
  virtual bool isValid() const = 0;

  /**
   * @brief Human readable type-name
   */
  virtual QString jobTypeName() const = 0;

  /**
   * @brief Type of convertion
   * 
   * Used by canConvert().
   */
  enum convertType {
    convertImpossible = 0, /**< Convert operation is not possible */
    convertionLossy, /**< Convertion is accompanied with loss of data. The loss has to be confirmed by the user */
    convertionLoseless /**< Convertion is possible without user interaction */
  };

  /**
   * @brief Account/plugin dependent settings for an onlineTask
   *
   * Many onlineTasks settings vary due to multiple reasons. E.g.
   * a credit transfer could have a maximum amount it can transfer at
   * once. But this amount could depend on the account and the users
   * contract with the bank.
   *
   * Therefor onlineTasks can offer thier own set of configurations. There
   * is no predifined behavior, only subclass onlineTask::settings.
   * Of course onlinePlugins and widgets which support that task
   * need to know how to handle that specific settings.
   *
   * Using @ref onlineJobAdministration::taskSettings() KMyMoney will
   * request the correct onlinePlugin to create the settings and return
   * them as shared pointer. Please note that KMyMoney will try to reuse
   * that pointer if possible, so do not edit it.
   */
  class settings
  {
  public:
    virtual ~settings();
  };

  /**
   * @brief Thrown if convert or convertInto fails
   */
  class badConvert : public MyMoneyException
  {
  public:
    badConvert(const QString file = QString(), const unsigned long line = 0)
      : MyMoneyException("Converted onlineTask with unsupported conversion", file, line)
    {}
  };

protected:
  onlineTask( const onlineTask& other );

  /**
   * @brief Copy this instance including inherited information
   *
   * This method copies an onlineJob including all information which are stored in inherited classes
   * even if you do not know the final type of an reference or pointer.
   */
  virtual onlineTask* clone() const = 0;

  /** @see MyMoneyObject::hasReferenceTo() */
  virtual bool hasReferenceTo(const QString &id) const = 0;
  /** @see MyMoneyObject::writeXML() */
  virtual void writeXML(QDomDocument &document, QDomElement &parent) const = 0;
  
  /**
   * @brief Account this job is related to
   *
   * Each task must have an account on which it operates. This is used to determine
   * the correct onlinePlugin which can execute this job. If the job is related to more
   * than one account (e.g. a password change) select a random one.
   * 
   * You can make this method public if usefull.
   * 
   * @return accountId
   */
  virtual QString responsibleAccount() const = 0;

  /**
   * @brief Checks if this onlineTask can be converted into onlineTaskName
   * 
   * @param onlineTaskName onlineTask::name() to convert into
   */
  virtual convertType canConvertInto( const QString& onlineTaskName ) const = 0;

  /**
   * @brief Checks if another onlineTask can be copied into this one
   * 
   * @param onlineTaskName onlineTask::name() of source
   */
  virtual convertType canConvert( const QString& onlineTaskName ) const = 0;

  /**
   * @brief Creates another onlineTask based on it's own data
   *
   * @param onlineTaskName onlineTask::name() to convert into
   * @param messageString OUT a translated string with description which data was lost during convertion (not needed if only payeeChanged applies).
   * This string is shown by the ui to the user.
   * @param payeeChanged OUT true if the address book was used to change any beneficiary data
   * @return newly created onlineTask, caller gains ownership of this task
   *
   * @throws badConvert if convert is not possible
   */
  virtual onlineTask* convertInto( const QString& onlineTaskName, QString& messageString, bool& payeeChanged ) const = 0;

  /**
   * @brief Replaces it's own data with the data of another task
   *
   * This is similar to operatior= ( other type ). But can be lossy.
   *
   * @param task to convert (do not modify pointer!)
   * @param messageString OUT a translated string with description which data was lost during convertion (not needed if only payeeChanged applies).
   * This string is shown by the ui to the user.
   * @param payeeChanged OUT true if the address book was used to change any beneficiary data
   *
   * @throws badConvert if convert is not possible
   */
  virtual void convert( const onlineTask& task, QString& messageString, bool& payeeChanged ) = 0;

  /**
   * @brief Used by @ref onlineTaskMeta
   * @internal
   * @return a new id
   */
  static size_t _internal_getNextMetaHash() {
    static size_t staticTypeIdCounter = 0;
    return (++staticTypeIdCounter);
  }

  friend class onlineJob;

private:

};

#endif // onlineJob_H
