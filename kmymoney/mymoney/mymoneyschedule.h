/*
    SPDX-FileCopyrightText: 2000-2004 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSCHEDULE_H
#define MYMONEYSCHEDULE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneyunittestable.h"
#include "mymoneyobject.h"

class QString;
class QDate;

class IMyMoneyProcessingCalendar;
class MyMoneyAccount;
class MyMoneyTransaction;

namespace eMyMoney {
namespace Schedule {
enum class Type;
enum class Occurrence;
enum class PaymentType;
enum class WeekendOption;
}
}

template <typename T> class QList;

/**
  * @author Michael Edwardes
  */

/**
  * This class represents a schedule. (A series of bills, deposits or
  * transfers).
  *
  * @short A class to represent a schedule.
  * @see MyMoneyScheduled
  */
class MyMoneySchedulePrivate;
class KMM_MYMONEY_EXPORT MyMoneySchedule : public MyMoneyObject
{
    Q_DECLARE_PRIVATE(MyMoneySchedule)

    friend class MyMoneyStorageANON;
    KMM_MYMONEY_UNIT_TESTABLE

public:
    /**
      * Standard constructor
      */
    MyMoneySchedule();
    explicit MyMoneySchedule(const QString &id);

    /**
      * Constructor for initialising the object.
      *
      * Please note that the optional fields are not set and the transaction
      * MUST be set before it can be used.
      *
      * @a startDate is not used anymore and internally set to QDate()
      */
    explicit MyMoneySchedule(const QString& name,
                             eMyMoney::Schedule::Type type,
                             eMyMoney::Schedule::Occurrence occurrence,
                             int occurrenceMultiplier,
                             eMyMoney::Schedule::PaymentType paymentType,
                             const QDate& startDate,
                             const QDate& endDate,
                             bool fixed,
                             bool autoEnter);

    MyMoneySchedule(const QString& id,
                    const MyMoneySchedule& other);

    MyMoneySchedule(const MyMoneySchedule & other);
    MyMoneySchedule(MyMoneySchedule && other);
    MyMoneySchedule & operator=(MyMoneySchedule other);
    friend void swap(MyMoneySchedule& first, MyMoneySchedule& second);

    /**
      * Standard destructor
      */
    ~MyMoneySchedule();

    /**
      * Simple get method that returns the base occurrence frequency.
      *
      * @return eMyMoney::Schedule::Occurrence The instance frequency
      *         reduced to the simple units.
      */
    eMyMoney::Schedule::Occurrence baseOccurrence() const;

    /**
      * Simple get method that returns the occurrence period
      * multiplier and occurrence
      *
      * @return eMyMoney::Schedule::Occurrence The instance period
      *
      */
    eMyMoney::Schedule::Occurrence occurrence() const;

    /**
      * Simple get method that returns the occurrence period multiplier.
      *
      * @return int The frequency multiplier
      */
    int occurrenceMultiplier() const;

    /**
      * Simple get method that returns the schedule type.
      *
      * @return eMyMoney::Schedule::Type The instance type.
      */
    eMyMoney::Schedule::Type type() const;

    /**
      * Simple get method that returns the schedule startDate. If
      * the schedule has been executed once, the date of the first
      * execution is returned. Otherwise, the next due date is
      * returned.
      *
      * @return reference to QDate containing the start date.
      */
    QDate startDate() const;

    /**
      * Simple get method that returns the schedule paymentType.
      *
      * @return eMyMoney::Schedule::PaymentType The instance paymentType.
      */
    eMyMoney::Schedule::PaymentType paymentType() const;

    /**
      * Simple get method that returns true if the schedule is fixed.
      *
      * @return bool To indicate whether the instance is fixed.
      */
    bool isFixed() const;

    /**
      * Simple get method that returns true if the schedule will end
      * at some time.
      *
      * @return bool Indicates whether the instance will end.
      */
    bool willEnd() const;

    /**
      * Simple get method that returns the number of transactions remaining.
      *
      * @return int The number of transactions remaining for the instance.
      */
    int transactionsRemaining() const;

    /**
      * Simple method that returns the number of transactions remaining
      * until a given date.
      *
      * @param endDate Date to count transactions to.
      * @return int The number of transactions remaining for the instance.
      */
    int transactionsRemainingUntil(const QDate& endDate) const;

    /**
      * Simple get method that returns the schedule end date.
      *
      * @return QDate The end date for the instance.
      */
    QDate endDate() const;

    /**
      * Get the state if the schedule should be processed at the last day
      * of a month
      *
      * @return state of the flag
      */
    bool lastDayInMonth() const;

    /**
      * Simple get method that returns true if the transaction should be
      * automatically entered into the register.
      *
      * @return bool Indicates whether the instance will be automatically entered.
      */
    bool autoEnter() const;

    /**
      * Simple get method that returns the transaction data for the schedule.
      *
      * @return MyMoneyTransaction The transaction data for the instance.
      */
    MyMoneyTransaction transaction() const;

    /**
      * Simple method that sets the transaction for the schedule.
      * The transaction must have a valid postDate set, otherwise
      * it will not be accepted. This test is bypassed, if @a noDateCheck
      * is set to true
      *
      * @param transaction The new transaction.
      * @param noDateCheck if @a true, the date check is bypassed
      * @return none
      */
    void setTransaction(const MyMoneyTransaction& transaction, bool noDateCheck);

    /**
      * Simple method that returns the schedules last payment. If the
      * schedule has never been executed, QDate() will be returned.
      *
      * @return QDate The last payment for the schedule.
      */
    QDate lastPayment() const;

    /**
      * Simple method that returns the next due date for the schedule.
      *
      * @return reference to QDate containing the next due date.
      *
      * @note The date returned can represent a value that is past
      *       a possible end of the schedule. Make sure to consider
      *       the return value of isFinished() when using the value returned.
      */
    QDate nextDueDate() const;

    /**
      * This method returns the next due date adjusted
      * according to the rules specified by the schedule's weekend option.
      *
      * @return QDate containing the adjusted next due date. If the
      *         schedule is finished (@sa isFinished()) then the method
      *         returns an invalid QDate.
      *
      * @sa weekendOption()
      * @sa adjustedDate()
      */
    QDate adjustedNextDueDate() const;

    /**
      * This method adjusts returns the date adjusted according to the
      * rules specified by the schedule's weekend option.
      *
      * @return QDate containing the adjusted date.
      */
    QDate adjustedDate(QDate date, eMyMoney::Schedule::WeekendOption option) const;

    /**

      * Get the weekendOption that determines how the schedule check code
      * will enter transactions that occur on a non-processing day (usually
      * a weekend).
      *
      * This not used by MyMoneySchedule but by the support code.
    **/
    eMyMoney::Schedule::WeekendOption weekendOption() const;

    /**
      * Simple method that sets the frequency for the schedule.
      *
      * @param occ The new occurrence (frequency).
      * @return none
      */
    void setOccurrence(eMyMoney::Schedule::Occurrence occ);

    /**
      * Simple method that sets the schedule period
      *
      * @param occ The new occurrence period (frequency)
      * @return none
      */
    void setOccurrencePeriod(eMyMoney::Schedule::Occurrence occ);

    /**
      * Simple method that sets the frequency multiplier for the schedule.
      *
      * @param occmultiplier The new occurrence (frequency) multiplier.
      * @return none
      */
    void setOccurrenceMultiplier(int occmultiplier);

    /**
      * Simple method that sets the type for the schedule.
      *
      * @param type The new type.
      * @return none
      */
    void setType(eMyMoney::Schedule::Type type);

    /**
      * Simple method that sets the start date for the schedule.
      *
      * @param date The new start date.
      * @return none
      */
    void setStartDate(const QDate& date);

    /**
      * Simple method that sets the payment type for the schedule.
      *
      * @param type The new payment type.
      * @return none
      */
    void setPaymentType(eMyMoney::Schedule::PaymentType type);

    /**
      * Simple method to set whether the schedule is fixed or not.
      *
      * @param fixed boolean to indicate whether the instance is fixed.
      * @return none
      */
    void setFixed(bool fixed);

    /**
      * Simple method that sets the transaction for the schedule.
      * The transaction must have a valid postDate set, otherwise
      * it will not be accepted.
      *
      * @param transaction The new transaction.
      * @return none
      */
    void setTransaction(const MyMoneyTransaction& transaction);

    /**
      * Simple set method to set the end date for the schedule.
      *
      * @param date The new end date.
      * @return none
      */
    void setEndDate(const QDate& date);

    /**
      * Simple method to set whether the schedule should be performed at
      * the last day of a month.
      *
      * @param state boolean The state to set
      * @return none
      */
    void setLastDayInMonth(bool state);

    /**
      * Simple set method to set whether this transaction should be automatically
      * entered into the journal whenever it is due.
      *
      * @param autoenter boolean to indicate whether we need to automatically
      *                  enter the transaction.
      * @return none
      */
    void setAutoEnter(bool autoenter);

    /**
      * Simple set method to set the schedule's next payment date.
      *
      * @param date The next payment date.
      * @return none
      */
    void setNextDueDate(const QDate& date);

    /**
      * Simple set method to set the schedule's last payment. If
      * this method is called for the first time on the object,
      * the @a m_startDate member will be set to @a date as well.
      *
      * This method should be called whenever a schedule is entered or skipped.
      *
      * @param date The last payment date.
      * @return none
      */
    void setLastPayment(const QDate& date);

    /**
      * Set the weekendOption that determines how the schedule check code
      * will enter transactions that occur on a non-processing day (usually
      * a weekend). The following values
      * are valid:
      *
      * - MoveNothing: don't modify date
      * - MoveBefore: modify the date to the previous processing day
      * - MoveAfter: modify the date to the next processing day
      *
      * If an invalid option is given, the option is set to MoveNothing.
      *
      * @param option See list in description
      * @return none
      *
      * @note This not used by MyMoneySchedule but by the support code.
      **/
    void setWeekendOption(const eMyMoney::Schedule::WeekendOption option);

    /**
      * Validates the schedule instance.
      *
      * Makes sure the paymentType matches the type and that the required
      * fields have been set.
      *
      * @param id_check if @p true, the method will check for an empty id.
      *                 if @p false, this check is skipped. Default is @p true.
      *
      * @return If this method returns, all checks are passed. Otherwise,
      *         it will throw a MyMoneyException object.
      *
      * @exception MyMoneyException with detailed error information is thrown
      *            in case of failure of any check.
      */
    void validate(bool id_check = true) const;

    /**
      * Calculates the date of the next payment adjusted according to the
      * rules specified by the schedule's weekend option.
      *
      * @param refDate The reference date from which the next payment
      *                date will be calculated (defaults to current date)
      *
      * @return QDate The adjusted date the next payment is due. This date is
      *               always past @a refDate.  In case of an error or if there
      *               are no more payments then an empty/invalid QDate() will
      *               be returned.
      */
    QDate adjustedNextPayment(const QDate& refDate) const;
    QDate adjustedNextPayment() const;

    /**
      * Calculates the date of the next payment.
      *
      * @param refDate The reference date from which the next payment
      *                date will be calculated (defaults to current date)
      *
      * @return QDate The date the next payment is due. This date is
      *         always past @a refDate.  In case of an error or
      *         if there are no more payments then an empty/invalid QDate()
      *         will be returned.
      */
    QDate nextPayment(const QDate& refDate) const;
    QDate nextPayment() const;

    /**
      * Calculates the date of the next payment and adjusts if asked.
      *
      * @param adjust Whether to adjust the calculated date according to the
      *               rules specified by the schedule's weekend option.
      * @param refDate The reference date from which the next payment
      *                date will be calculated (defaults to current date)
      *
      * @return QDate The date the next payment is due. This date is
      *         always past @a refDate.  In case of an error or
      *         if there is no more payments then an empty/invalid QDate()
      *         will be returned.
      */
    QDate nextPaymentDate(const bool& adjust, const QDate& refDate) const;
    QDate nextPaymentDate(const bool& adjust) const;

    /**
      * Calculates the dates of the payment over a certain period of time.
      *
      * An empty list is returned for no payments or error.
      *
      * @param startDate The start date for the range calculations
      * @param endDate The end date for the range calculations.
      * @return QList<QDate> The dates on which the payments are due.
      */
    QList<QDate> paymentDates(const QDate& startDate, const QDate& endDate) const;

    /**
      * Returns the instances name
      *
      * @return The name
      */
    QString name() const;

    /**
      * Changes the instance name
      *
      * @param nm The new name
      * @return none
      */
    void setName(const QString& nm);

    bool operator ==(const MyMoneySchedule& right) const;
    bool operator !=(const MyMoneySchedule& right) const;

    bool operator <(const MyMoneySchedule& right) const;

    MyMoneyAccount account(int cnt = 1) const;
    MyMoneyAccount transferAccount() const;
    QDate dateAfter(int transactions) const;

    bool isOverdue() const;
    bool isFinished() const;
    bool hasRecordedPayment(const QDate&) const;
    void recordPayment(const QDate&);
    QList<QDate> recordedPayments() const;

    /**
      * This method checks if a reference to the given object exists. It returns,
      * a @p true if the object is referencing the one requested by the
      * parameter @p id. If it does not, this method returns @p false.
      *
      * @param id id of the object to be checked for references
      * @retval true This object references object with id @p id.
      * @retval false This object does not reference the object with id @p id.
      */
    virtual bool hasReferenceTo(const QString& id) const final override;

    /**
     * This method replaces all occurrences of id @a oldId with
     * @a newId.  All other ids are not changed.
     *
     * @return true if any change has been performed
     * @return false if nothing has been modified
     */
    bool replaceId(const QString& newId, const QString& oldId);

    /**
     * Returns the human-readable format of Schedule's occurrence
     *
     * @return QString representing the human readable format
     */
    QString occurrenceToString() const;

    /**
     * This method is used to convert the occurrence type from its
     * internal representation into a human readable format.
     *
     * @param type numerical representation of the MyMoneySchedule
     *                  occurrence type
     *
     * @return QString representing the human readable format
     */
    static QString occurrenceToString(eMyMoney::Schedule::Occurrence type);

    /**
     * This method is used to convert a multiplier and base occurrence type
     * from its internal representation into a human readable format.
     * When multiplier * occurrence is equivalent to a simple occurrence
     * the method returns the same as occurrenceToString of the simple occurrence
     *
     * @param mult occurrence multiplier
     * @param type occurrence period
     *
     * @return QString representing the human readable format
     */
    static QString occurrenceToString(int mult, eMyMoney::Schedule::Occurrence type);

    /**
     * This method is used to convert an occurrence period from
     * its internal representation into a human-readable format.
     *
     * @param type numerical representation of the MyMoneySchedule
     *                  occurrence type
     *
     * @return QString representing the human readable format
     */
    static QString occurrencePeriodToString(eMyMoney::Schedule::Occurrence type);

    /**
     * This method is used to convert the payment type from its
     * internal representation into a human readable format.
     *
     * @param paymentType numerical representation of the MyMoneySchedule
     *                  payment type
     *
     * @return QString representing the human readable format
     */
    static QString paymentMethodToString(eMyMoney::Schedule::PaymentType paymentType);

    /**
     * This method is used to convert the schedule weekend option from its
     * internal representation into a human readable format.
     *
     * @param weekendOption numerical representation of the MyMoneySchedule
     *                  weekend option
     *
     * @return QString representing the human readable format
     */
    static QString weekendOptionToString(eMyMoney::Schedule::WeekendOption weekendOption);

    /**
     * This method is used to convert the schedule type from its
     * internal representation into a human readable format.
     *
     * @param type numerical representation of the MyMoneySchedule
     *                  schedule type
     *
     * @return QString representing the human readable format
     */
    static QString scheduleTypeToString(eMyMoney::Schedule::Type type);

    int variation() const;
    void setVariation(int var);

    /**
     *
     * Convert an occurrence to the maximum number of events possible during a single
     * calendar year.
     * A fortnight is treated as 15 days.
     *
     * @param occurrence  The occurrence
     *
     * @return int  Number of days between events
     */
    static int eventsPerYear(eMyMoney::Schedule::Occurrence occurrence);

    /**
     *
     * Convert an occurrence to the number of days between events
     * Treats a month as 30 days.
     * Treats a fortnight as 15 days.
     *
     * @param occurrence  The occurrence
     *
     * @return int  Number of days between events
     */
    static int daysBetweenEvents(eMyMoney::Schedule::Occurrence occurrence);

    /**
      * Helper method to convert simple occurrence to compound occurrence + multiplier
      *
      * @param multiplier Returned by reference.  Adjusted multiplier
      * @param occurrence Returned by reference.  Occurrence type
      */
    static void simpleToCompoundOccurrence(int& multiplier, eMyMoney::Schedule::Occurrence& occurrence);

    /**
      * Helper method to convert compound occurrence + multiplier to simple occurrence
      *
      * @param multiplier Returned by reference.  Adjusted multiplier
      * @param occurrence Returned by reference.  Occurrence type
      */
    static void compoundToSimpleOccurrence(int& multiplier, eMyMoney::Schedule::Occurrence& occurrence);

    /**
      * This method is used to set the static point to relevant
      * IMyMoneyProcessingCalendar.
      */
    static void setProcessingCalendar(IMyMoneyProcessingCalendar* pc);

private:
    /**
      * This method returns a pointer to the processing calendar object.
      *
      * @return const pointer to the current attached processing calendar object.
      *         If no object is attached, returns 0.
      */
    IMyMoneyProcessingCalendar* processingCalendar() const;

    /**
      * This method forces the day of the passed @p date to
      * be the day of the start date of this schedule kept
      * in m_startDate. It is internally used when calculating
      * the payment dates over several periods.
      *
      * @param date reference to QDate object to be checked and adjusted
      */
    void fixDate(QDate& date) const;

    /**
      * This method adds a number of Half Months to the given Date.
      * This is used for EveryHalfMonth occurrences.
      * The addition uses the following rules to add a half month:
      * Day 1-13: add 15 days
      * Day 14: add 15 days (except February: the last day of the month)
      * Day 15: last day of the month
      * Day 16-29 (not last day in February): subtract 15 days and add 1 month
      * 30 and last day: 15th of next month
      *
      * This calculation pairs days 1 to 12 with 16 to 27.
      * Day 15 is paired with the last day of every month.
      * Repeated addition has issues in the following cases:
      * - Days 13 to 14 are paired with 28 to 29 until addition hits the last day of February
      *   after which the (15,last) pair will be used.
      * - Addition from Day 30 leads immediately to the (15th,last) day pair.
      *
      * @param date The date
      * @param mult The number of half months to add.  Default is 1.
      *
      * @return QDate date with mult half months added
      */
    QDate addHalfMonths(QDate date, int mult = 1) const;

    /**
      * Checks if a given date should be considered a processing day
      * based on a calendar. See @a IMyMoneyProcessingCalendar and
      * setProcessingCalendar(). If no processingCalendar has been
      * setup using setProcessingCalendar it returns @c true on Mon..Fri
      * and @c false on Sat..Sun.
      */
    bool isProcessingDate(const QDate& date) const;
};

inline void swap(MyMoneySchedule& first, MyMoneySchedule& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline MyMoneySchedule::MyMoneySchedule(MyMoneySchedule && other) : MyMoneySchedule() // krazy:exclude=inline
{
    swap(*this, other);
}

inline MyMoneySchedule & MyMoneySchedule::operator=(MyMoneySchedule other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

/**
  * Make it possible to hold @ref MyMoneySchedule objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneySchedule)

#endif
