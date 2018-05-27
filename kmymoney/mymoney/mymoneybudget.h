/*
 * Copyright 2006       Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef MYMONEYBUDGET_H
#define MYMONEYBUDGET_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject.h"
#include "kmm_mymoney_export.h"
#include "mymoneyunittestable.h"

class QString;
class QDate;
class QDomElement;
class QDomDocument;
class MyMoneyMoney;

template <typename T> class QList;
template <class Key, class T> class QMap;

/**
  * This class defines a Budget within the MyMoneyEngine.  The Budget class
  * contains all the configuration parameters needed to run a Budget, plus
  * XML serialization.
  *
  * As noted above, this class only provides a Budget DEFINITION.  The
  * generation and presentation of the Budget itself are left to higher
  * level classes.
  *
  * @author Darren Gould <darren_gould@gmx.de>
  */
class MyMoneyBudgetPrivate;
class KMM_MYMONEY_EXPORT MyMoneyBudget: public MyMoneyObject
{
  Q_DECLARE_PRIVATE(MyMoneyBudget)

  KMM_MYMONEY_UNIT_TESTABLE

public:
  MyMoneyBudget();
  explicit MyMoneyBudget(const QString& name);
  /**
    * This constructor creates an object based on the data found in the
    * QDomElement referenced by @p node. If problems arise, the @p id of
    * the object is cleared (see MyMoneyObject::clearId()).
    */
  explicit MyMoneyBudget(const QDomElement& node);

  /**
    * This constructor creates an object based on the data found in the
    * MyMoneyBudget budget object.
    */
  MyMoneyBudget(const QString& id,
                const MyMoneyBudget& other);

  MyMoneyBudget(const MyMoneyBudget& other);
  MyMoneyBudget(MyMoneyBudget && other);
  MyMoneyBudget & operator=(MyMoneyBudget other);
  friend void swap(MyMoneyBudget& first, MyMoneyBudget& second);

  ~MyMoneyBudget();

  /**
    * Helper class for MyMoneyBudget
    *
    * This is an abstraction of the PERIOD stored in the BUDGET/ACCOUNT tag in XML
    *
    * @author Darren Gould
    */
  class PeriodGroupPrivate;
  class KMM_MYMONEY_EXPORT PeriodGroup
  {
    Q_DECLARE_PRIVATE(PeriodGroup)
    PeriodGroupPrivate* d_ptr;

  public:
    PeriodGroup();
    PeriodGroup(const PeriodGroup & other);
    PeriodGroup(PeriodGroup && other);
    PeriodGroup & operator=(PeriodGroup other);
    friend void swap(PeriodGroup& first, PeriodGroup& second);

    ~PeriodGroup();

    QDate startDate() const;
    void setStartDate(const QDate& start);

    MyMoneyMoney amount() const;
    void setAmount(const MyMoneyMoney& amount);

    bool operator == (const PeriodGroup &right) const;
  };

  /**
    * Helper class for MyMoneyBudget
    *
    * This is an abstraction of the Account Data stored in the BUDGET tag in XML
    *
    * @author Darren Gould
    */
  class AccountGroupPrivate;
  class KMM_MYMONEY_EXPORT AccountGroup
  {
    Q_DECLARE_PRIVATE(AccountGroup)
    AccountGroupPrivate* d_ptr;

  public:
    typedef enum {
      eNone = 0,
      eMonthly,
      eMonthByMonth,
      eYearly,
      eMax
    } eBudgetLevel;

    static const QStringList kBudgetLevelText;

  public:
    AccountGroup();
    AccountGroup(const AccountGroup & other);
    AccountGroup(AccountGroup && other);
    AccountGroup & operator=(AccountGroup other);
    friend void swap(AccountGroup& first, AccountGroup& second);

    ~AccountGroup();

    QString id() const;
    void setId(const QString& id);

    bool budgetSubaccounts() const;
    void setBudgetSubaccounts(bool budgetsubaccounts);

    eBudgetLevel budgetLevel() const;
    void setBudgetLevel(eBudgetLevel level);

    PeriodGroup period(const QDate& date) const;
    void addPeriod(const QDate& date, PeriodGroup& period);
    const QMap<QDate, PeriodGroup> getPeriods() const;
    void clearPeriods();

    MyMoneyMoney balance() const;
    MyMoneyMoney totalBalance() const;

    // This member adds the value of another account group
    // m_budgetlevel is adjusted to the larger one of both
    // m_budgetsubaccounts remains unaffected
    AccountGroup operator += (const AccountGroup& right);

    bool operator == (const AccountGroup &right) const;

    bool isZero() const;

  protected:
    void convertToMonthly();
    void convertToYearly();
    void convertToMonthByMonth();
  };

  /**
   * This operator tests for equality of two MyMoneyBudget objects
   */
  bool operator == (const MyMoneyBudget &) const;

  QString name() const;
  void setName(const QString& name);

  QDate budgetStart() const;
  void setBudgetStart(const QDate& start);

  const AccountGroup & account(const QString &id) const;
  void setAccount(const AccountGroup& account, const QString &id);

  bool contains(const QString &id) const;
  QList<AccountGroup> getaccounts() const;

  /**
    * This method writes this Budget to the DOM element @p e,
    * within the DOM document @p doc.
    *
    * @param e The element which should be populated with info from this Budget
    * @param doc The document which we can use to create new sub-elements
    *              if needed
    */
  void write(QDomElement& e, QDomDocument *doc) const;

  /**
    * This method reads a Budget from the DOM element @p e, and
    * populates this Budget with the results.
    *
    * @param e The element from which the Budget should be read
    *
    * @return bool True if a Budget was successfully loaded from the
    *    element @p e.  If false is returned, the contents of this Budget
    *    object are undefined.
    */
  bool read(const QDomElement& e);

  /**
    * This method creates a QDomElement for the @p document
    * under the parent node @p parent.  (This version overwrites the
    * MMObject base class.)
    *
    * @param document reference to QDomDocument
    * @param parent reference to QDomElement parent node
    */
  void writeXML(QDomDocument& document, QDomElement& parent) const override;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id and the balance() returned is zero.
    * If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  bool hasReferenceTo(const QString& id) const override;

  /**
    * This member removes all references to object identified by @p id. Used
    * to remove objects which are about to be removed from the engine.
    */
  void removeReference(const QString& id);
};

inline void swap(MyMoneyBudget::PeriodGroup& first, MyMoneyBudget::PeriodGroup& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyBudget::PeriodGroup::PeriodGroup(MyMoneyBudget::PeriodGroup && other) : PeriodGroup() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyBudget::PeriodGroup & MyMoneyBudget::PeriodGroup::operator=(MyMoneyBudget::PeriodGroup other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

inline void swap(MyMoneyBudget::AccountGroup& first, MyMoneyBudget::AccountGroup& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyBudget::AccountGroup::AccountGroup(MyMoneyBudget::AccountGroup && other) : AccountGroup() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyBudget::AccountGroup & MyMoneyBudget::AccountGroup::operator=(MyMoneyBudget::AccountGroup other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

inline void swap(MyMoneyBudget& first, MyMoneyBudget& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyBudget::MyMoneyBudget(MyMoneyBudget && other) : MyMoneyBudget() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyBudget & MyMoneyBudget::operator=(MyMoneyBudget other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

/**
  * Make it possible to hold @ref MyMoneyBudget objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyBudget)

#endif // MYMONEYBudget_H
