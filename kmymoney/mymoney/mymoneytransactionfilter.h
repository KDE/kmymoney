/*
    SPDX-FileCopyrightText: 2003-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2008-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYTRANSACTIONFILTER_H
#define MYMONEYTRANSACTIONFILTER_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QString;
class QDate;

template <typename T> class QList;

class MyMoneyMoney;
class MyMoneySplit;
class MyMoneyAccount;

namespace eMyMoney {
namespace TransactionFilter {
enum class Date;
enum class Validity;
}
}

/**
  * @author Thomas Baumgart
  * @author Łukasz Wojniłowicz
  */

class MyMoneyTransaction;
class MyMoneyTransactionFilterPrivate;
class KMM_MYMONEY_EXPORT MyMoneyTransactionFilter
{
    Q_DECLARE_PRIVATE(MyMoneyTransactionFilter)

protected:
    MyMoneyTransactionFilterPrivate* d_ptr;  // name shouldn't colide with the one in mymoneyreport.h

public:
    enum FilterFlags {
        textFilterActive     = 0x0001,
        accountFilterActive  = 0x0002,
        payeeFilterActive    = 0x0004,
        tagFilterActive      = 0x0008,
        categoryFilterActive = 0x0010,
        nrFilterActive       = 0x0020,
        dateFilterActive     = 0x0040,
        amountFilterActive   = 0x0080,
        typeFilterActive     = 0x0100,
        stateFilterActive    = 0x0200,
        validityFilterActive = 0x0400
    };
    Q_DECLARE_FLAGS(FilterSet, FilterFlags)

    /**
      * This is the standard constructor for a transaction filter.
      * It creates the object and calls setReportAllSplits() to
      * report all matching splits as separate entries. Use
      * setReportAllSplits() to override this behaviour.
      */
    MyMoneyTransactionFilter();

    /**
      * This is a convenience constructor to allow construction of
      * a simple account filter. It is basically the same as the
      * following:
      *
      * @code
      * :
      *   MyMoneyTransactionFilter filter;
      *   filter.setReportAllSplits(false);
      *   filter.addAccount(id);
      * :
      * @endcode
      *
      * @param id reference to account id
      */
    explicit MyMoneyTransactionFilter(const QString& id);

    MyMoneyTransactionFilter(const MyMoneyTransactionFilter & other);
    MyMoneyTransactionFilter(MyMoneyTransactionFilter && other);
    MyMoneyTransactionFilter & operator=(MyMoneyTransactionFilter other);
    friend void swap(MyMoneyTransactionFilter& first, MyMoneyTransactionFilter& second);

    virtual ~MyMoneyTransactionFilter();

    /**
      * This method is used to clear the filter. All settings will be
      * removed.
      */
    void clear();

    /**
      * This method is used to clear the accounts filter only.
      */
    void clearAccountFilter();

    /**
      * This method is used to set the regular expression filter to the value specified
      * as parameter @p exp. The following text based fields are searched:
      *
      * - Memo
      * - Payee
      * - Tag
      * - Category
      * - Shares / Value
      * - Number
      *
      * @param exp The regular expression that must be found in a transaction
      *            before it is included in the result set.
      * @param invert If true, value must not be contained in any of the above mentioned fields
      *
      */
    void setTextFilter(const QRegExp& exp, bool invert = false);

    /**
      * This method will add the account with id @p id to the list of matching accounts.
      * If the list is empty, any transaction will match.
      *
      * @param id internal ID of the account
      */
    void addAccount(const QString& id);

    /**
      * This is a convenience method and behaves exactly like the above
      * method but for a list of id's.
      */
    void addAccount(const QStringList& ids);

    /**
      * This method will add the category with id @p id to the list of matching categories.
      * If the list is empty, only transaction with a single asset/liability account will match.
      *
      * @param id internal ID of the account
      */
    void addCategory(const QString& id);

    /**
      * This is a convenience method and behaves exactly like the above
      * method but for a list of id's.
      */
    void addCategory(const QStringList& ids);

    /**
      * This method sets the date filter to match only transactions with posting dates in
      * the date range specified by @p from and @p to. If @p from equal QDate()
      * all transactions with dates prior to @p to match. If @p to equals QDate()
      * all transactions with posting dates past @p from match. If @p from and @p to
      * are equal QDate() the filter is not activated and all transactions match.
      *
      * @param from from date
      * @param to   to date
      */
    void setDateFilter(const QDate& from, const QDate& to);

    void setDateFilter(eMyMoney::TransactionFilter::Date range);

    /**
      * This method sets the amount filter to match only transactions with
      * an amount in the range specified by @p from and @p to.
      * If a specific amount should be searched, @p from and @p to should be
      * the same value.
      *
      * @param from smallest value to match
      * @param to   largest value to match
      */
    void setAmountFilter(const MyMoneyMoney& from, const MyMoneyMoney& to);

    /**
      * This method will add the payee with id @p id to the list of matching payees.
      * If the list is empty, any transaction will match.
      *
      * @param id internal id of the payee
      */
    void addPayee(const QString& id);

    /**
      * This method will add the tag with id @ta id to the list of matching tags.
      * If the list is empty, any transaction will match.
      *
      * @param id internal id of the tag
      */
    void addTag(const QString& id);

    /**
      */
    void addType(const int type);

    /**
      */
    void addValidity(const int type);

    /**
      */
    void addState(const int state);

    /**
      * This method sets the number filter to match only transactions with
      * a number in the range specified by @p from and @p to.
      * If a specific number should be searched, @p from and @p to should be
      * the same value.
      *
      * @param from smallest value to match
      * @param to   largest value to match
      *
      * @note @p from and @p to can contain alphanumeric text
      */
    void setNumberFilter(const QString& from, const QString& to);

    /**
      * This method is used to check a specific transaction against the filter.
      * The transaction will match the whole filter, if all specified filters
      * match. If the filter is cleared using the clear() method, any transaction
      * matches. Matching splits from the transaction are returned by @ref
      * matchingSplits().
      *
      * @param transaction A transaction
      *
      * @retval true The transaction matches the filter set
      * @retval false The transaction does not match at least one of
      *               the filters in the filter set
      */
    bool match(const MyMoneyTransaction& transaction);

    /**
      * This method is used to check a specific split against the
      * text filter. The split will match if all specified and
      * checked filters match. If the filter is cleared using the clear()
      * method, any split matches.
      *
      * @param sp pointer to the split to be checked
      *
      * @retval true The split matches the filter set
      * @retval false The split does not match at least one of
      *               the filters in the filter set
      */
    bool matchText(const MyMoneySplit& s, const MyMoneyAccount &acc) const;

    /**
      * This method is used to check a specific split against the
      * amount filter. The split will match if all specified and
      * checked filters match. If the filter is cleared using the clear()
      * method, any split matches.
      *
      * @param sp const reference to the split to be checked
      *
      * @retval true The split matches the filter set
      * @retval false The split does not match at least one of
      *               the filters in the filter set
      */
    bool matchAmount(const MyMoneySplit& s) const;

    /**
     * Convenience method which actually returns matchText(sp) && matchAmount(sp).
     */
    bool match(const MyMoneySplit& s) const;

    /**
      * This method is used to switch the amount of splits reported
      * by matchingSplits(). If the argument @p report is @p true (the default
      * if no argument specified) then matchingSplits() will return all
      * matching splits of the transaction. If @p report is set to @p false,
      * then only the very first matching split will be returned by
      * matchingSplits().
      *
      * @param report controls the behaviour of matchingsSplits() as explained above.
      */
    void setReportAllSplits(const bool report = true);

    /**
     * Consider splits in categories
     *
     * With this setting, splits in categories that are not considered
     * by default are taken into account.
     *
     * @param check check state
     */
    void setConsiderCategorySplits(const bool check = true);

    /**
     * Consider income and expense categories
     *
     * If the account or category filter is enabled, categories of
     * income and expense type are included if enabled with this
     * method.
     *
     * @param check check state
     */
    void setConsiderCategory(const bool check = true);

    void setTreatTransfersAsIncomeExpense(const bool check = true);

    /**
     * This method is to avoid returning matching splits list
     * if only its count is needed
     * @return count of matching splits
     */
    uint matchingSplitsCount(const MyMoneyTransaction& transaction);

    /**
      * This method returns a list of the matching splits for the filter.
      * If m_reportAllSplits is set to false, then only the very first
      * split will be returned. Use setReportAllSplits() to change the
      * behaviour.
      *
      * @return reference list of MyMoneySplit objects containing the
      *         matching splits. If multiple splits match, only the first
      *         one will be returned.
      *
      * @note an empty list will be returned, if the filter only required
      *       to check the data contained in the MyMoneyTransaction
      *       object (e.g. posting-date, state, etc.).
      *
      * @note The constructors set m_reportAllSplits differently. Please
      *       see the documentation of the constructors MyMoneyTransactionFilter()
      *       and MyMoneyTransactionFilter(const QString&) for details.
      */
    QVector<MyMoneySplit> matchingSplits(const MyMoneyTransaction& transaction);

    /**
      * This method returns the from date set in the filter. If
      * no value has been set up for this filter, then QDate() is
      * returned.
      *
      * @return returns m_fromDate
      */
    QDate fromDate() const;

    /**
      * This method returns the to date set in the filter. If
      * no value has been set up for this filter, then QDate() is
      * returned.
      *
      * @return returns m_toDate
      */
    QDate toDate() const;

    /**
      * This method is used to return information about the
      * presence of a specific category in the category filter.
      * The category in question is included in the filter set,
      * if it has been set or no category filter is set.
      *
      * @param cat id of category in question
      * @return true if category is in filter set, false otherwise
      */
    bool includesCategory(const QString& cat) const;

    /**
      * This method is used to return information about the
      * presence of a specific account in the account filter.
      * The account in question is included in the filter set,
      * if it has been set or no account filter is set.
      *
      * @param acc id of account in question
      * @return true if account is in filter set, false otherwise
      */
    bool includesAccount(const QString& acc) const;

    /**
      * This method is used to return information about the
      * presence of a specific payee in the account filter.
      * The payee in question is included in the filter set,
      * if it has been set or no account filter is set.
      *
      * @param pye id of payee in question
      * @return true if payee is in filter set, false otherwise
      */
    bool includesPayee(const QString& pye) const;

    /**
      * This method is used to return information about the
      * presence of a specific tag in the account filter.
      * The tag in question is included in the filter set,
      * if it has been set or no account filter is set.
      *
      * @param tag id of tag in question
      * @return true if tag is in filter set, false otherwise
      */
    bool includesTag(const QString& tag) const;

    /**
      * This method is used to return information about the
      * presence of a date filter.
      *
      * @param from result value for the beginning of the date range
      * @param to result value for the end of the date range
      * @return true if a date filter is set
      */
    bool dateFilter(QDate& from, QDate& to) const;

    /**
      * This method is used to return information about the
      * presence of an amount filter.
      *
      * @param from result value for the low end of the amount range
      * @param to result value for the high end of the amount range
      * @return true if an amount filter is set
      */
    bool amountFilter(MyMoneyMoney& from, MyMoneyMoney& to) const;

    /**
      * This method is used to return information about the
      * presence of an number filter.
      *
      * @param from result value for the low end of the number range
      * @param to result value for the high end of the number range
      * @return true if a number filter is set
      */
    bool numberFilter(QString& from, QString& to) const;

    /**
      * This method returns whether a payee filter has been set,
      * and if so, it returns all the payees set in the filter.
      *
      * @param list list to append payees into
      * @return return true if a payee filter has been set
      */
    bool payees(QStringList& list) const;

    /**
      * This method returns whether a tag filter has been set,
      * and if so, it returns all the tags set in the filter.
      *
      * @param list list to append tags into
      * @return return true if a tag filter has been set
      */
    bool tags(QStringList& list) const;

    /**
      * This method returns whether an account filter has been set,
      * and if so, it returns all the accounts set in the filter.
      *
      * @param list list to append accounts into
      * @return return true if an account filter has been set
      */
    bool accounts(QStringList& list) const;

    /**
      * This method returns whether a category filter has been set,
      * and if so, it returns all the categories set in the filter.
      *
      * @param list list to append categories into
      * @return return true if a category filter has been set
      */
    bool categories(QStringList& list) const;

    /**
      * This method returns whether a type filter has been set,
      * and if so, it returns the first type in the filter.
      *
      * @param i int to replace with first type filter, untouched otherwise
      * @return return true if a type filter has been set
      */
    bool firstType(int& i) const;

    bool types(QList<int>& list) const;

    /**
      * This method returns whether a state filter has been set,
      * and if so, it returns the first state in the filter.
      *
      * @param i reference to int to replace with first state filter, untouched otherwise
      * @return return true if a state filter has been set
      */
    bool firstState(int& i) const;

    bool states(QList<int>& list) const;

    /**
      * This method returns whether a validity filter has been set,
      * and if so, it returns the first validity in the filter.
      *
      * @param i reference to int to replace with first validity filter, untouched otherwise
      * @return return true if a validity filter has been set
      */
    bool firstValidity(int& i) const;

    bool validities(QList<int>& list) const;

    /**
      * This method returns whether a text filter has been set,
      * and if so, it returns the text filter.
      *
      * @param text regexp to replace with text filter, or blank if none set
      * @return return true if a text filter has been set
      */
    bool textFilter(QRegExp& text) const;

    /**
     * This method returns whether the text filter should return
     * that DO NOT contain the text
     */
    bool isInvertingText() const;

    /**
     * This method returns whether transfers should be treated as
     * income/expense transactions or not
     */
    bool treatTransfersAsIncomeExpense() const;

    /**
      * This method translates a plain-language date range into QDate
      * start & end
      *
      * @param range Plain-language range of dates, e.g. 'CurrentYear'
      * @param start QDate will be set to corresponding to the first date in @p range
      * @param end QDate will be set to corresponding to the last date in @p range
      * @return return true if a range was successfully set, or false if @p range was invalid
      */
    static bool translateDateRange(eMyMoney::TransactionFilter::Date range, QDate& start, QDate& end);

    static void setFiscalYearStart(int firstMonth, int firstDay);

    FilterSet filterSet() const;

    /**
      * This member removes all references to object identified by @p id. Used
      * to remove objects which are about to be removed from the engine.
      */
    void removeReference(const QString& id);

private:
    /**
      * This is a conversion tool from eMyMoney::Split::State
      * to MyMoneyTransactionFilter::stateE types
      *
      * @param split reference to split in question
      *
      * @return converted reconcile flag of the split passed as parameter
      */
    int splitState(const MyMoneySplit& split) const;

    /**
      * This is a conversion tool from MyMoneySplit::action
      * to MyMoneyTransactionFilter::typeE types
      *
      * @param t reference to transaction
      * @param split reference to split in question
      *
      * @return converted action of the split passed as parameter
      */
    int splitType(const MyMoneyTransaction& t, const MyMoneySplit& split, const MyMoneyAccount &acc) const;

    /**
      * This method checks if a transaction is valid or not. A transaction
      * is considered valid, if the sum of all splits is zero, invalid otherwise.
      *
      * @param transaction reference to transaction to be checked
      * @retval valid transaction is valid
      * @retval invalid transaction is invalid
      */
    eMyMoney::TransactionFilter::Validity validTransaction(const MyMoneyTransaction& transaction) const;
};

inline void swap(MyMoneyTransactionFilter& first, MyMoneyTransactionFilter& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyTransactionFilter::MyMoneyTransactionFilter(MyMoneyTransactionFilter && other) : MyMoneyTransactionFilter() // krazy:exclude=inline
{
    swap(*this, other);
}

inline MyMoneyTransactionFilter & MyMoneyTransactionFilter::operator=(MyMoneyTransactionFilter other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

/**
  * Make it possible to hold @ref MyMoneyTransactionFilter objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyTransactionFilter)
Q_DECLARE_OPERATORS_FOR_FLAGS(MyMoneyTransactionFilter::FilterSet)

#endif
