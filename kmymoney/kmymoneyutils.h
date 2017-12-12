/***************************************************************************
                          kmymoneyutils.h  -  description
                             -------------------
    begin                : Wed Feb 5 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYUTILS_H
#define KMYMONEYUTILS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardPaths>
#include <QMap>
#include <QUrl>
// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

#include "selectedtransactions.h"

class QIcon;

/**
  * @author Thomas Baumgart
  */

static QString m_lastNumberUsed;

class QPixmap;
class QWizard;
class QWidget;

class KGuiItem;
class KXmlGuiWindow;

class MyMoneyMoney;
class MyMoneyAccount;
class MyMoneySecurity;
class MyMoneySchedule;
class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneyStatement;
class MyMoneyInstitution;

namespace eMyMoney { namespace Schedule { enum class Occurrence;
                                          enum class PaymentType;
                                          enum class WeekendOption;
                                          enum class Type; }
                     namespace Split { enum class State;
                                       enum class InvestmentTransactionType; }
                   }

class KMyMoneyUtils
{
public:
  enum transactionTypeE {
    /**
      * Unknown transaction type (e.g. used for a transaction with only
      * a single split)
      */
    Unknown,

    /**
      * A 'normal' transaction is one that consists out two splits: one
      * referencing an income/expense account, the other referencing
      * an asset/liability account.
      */
    Normal,

    /**
      * A transfer denotes a transaction consisting of two splits.
      * Both of the splits reference an asset/liability
      * account.
      */
    Transfer,

    /**
      * Whenever a transaction consists of more than 2 splits,
      * it is treated as 'split transaction'.
      */
    SplitTransaction,

    /**
      * This transaction denotes a specific transaction where
      * a loan account is involved. Usually, a special dialog
      * is used to modify this transaction.
      */
    LoanPayment,

    /**
      * This transaction denotes a specific transaction where
      * an investment is involved. Usually, a special dialog
      * is used to modify this transaction.
      */
    InvestmentTransaction
  };

  static const int maxHomePageItems = 5;

  KMyMoneyUtils();
  ~KMyMoneyUtils();

  /**
    * This method is used to convert the occurrence type from its
    * internal representation into a human readable format.
    *
    * @param occurrence numerical representation of the MyMoneySchedule
    *                  occurrence type
    *
    * @return QString representing the human readable format translated according to the language cataglogue
    *
    * @sa MyMoneySchedule::occurrenceToString()
    *
    * @deprecated Use i18n(MyMoneySchedule::occurrenceToString(occurrence)) instead
    */
  static const QString occurrenceToString(const eMyMoney::Schedule::Occurrence occurrence);

  /**
    * This method is used to convert the payment type from its
    * internal representation into a human readable format.
    *
    * @param paymentType numerical representation of the MyMoneySchedule
    *                  payment type
    *
    * @return QString representing the human readable format translated according to the language cataglogue
    *
    * @sa MyMoneySchedule::paymentMethodToString()
    */
  static const QString paymentMethodToString(eMyMoney::Schedule::PaymentType paymentType);

  /**
    * This method is used to convert the schedule weekend option from its
    * internal representation into a human readable format.
    *
    * @param weekendOption numerical representation of the MyMoneySchedule
    *                  weekend option
    *
    * @return QString representing the human readable format translated according to the language cataglogue
    *
    * @sa MyMoneySchedule::weekendOptionToString()
    */
  static const QString weekendOptionToString(eMyMoney::Schedule::WeekendOption weekendOption);

  /**
    * This method is used to convert the schedule type from its
    * internal representation into a human readable format.
    *
    * @param type numerical representation of the MyMoneySchedule
    *                  schedule type
    *
    * @return QString representing the human readable format translated according to the language cataglogue
    *
    * @sa MyMoneySchedule::scheduleTypeToString()
    */
  static const QString scheduleTypeToString(eMyMoney::Schedule::Type type);

  /**
    * This method is used to convert a numeric index of an item
    * represented on the home page into its string form.
    *
    * @param idx numeric index of item
    *
    * @return QString with text of this item
    */
  static const QString homePageItemToString(const int idx);

  /**
    * This method is used to convert the name of a home page item
    * to its internal numerical representation
    *
    * @param txt QString reference of the items name
    *
    * @retval 0 @p txt is unknown
    * @retval >0 numeric value for @p txt
    */
  static int stringToHomePageItem(const QString& txt);

  /**
    * Retrieve a KDE KGuiItem for the new schedule button.
    *
    * @return The KGuiItem that can be used to display the icon and text
    */
  static KGuiItem scheduleNewGuiItem();

  /**
    * Retrieve a KDE KGuiItem for the account filter button
    *
    * @return The KGuiItem that can be used to display the icon and text
    */
  static KGuiItem accountsFilterGuiItem();

  /**
    * This method adds the file extension passed as argument @p extension
    * to the end of the file name passed as argument @p name if it is not present.
    * If @p name contains an extension it will be removed.
    *
    * @param name filename to be checked
    * @param extension extension to be added (w/o the dot)
    *
    * @retval true if @p name was changed
    * @retval false if @p name remained unchanged
    */
  static bool appendCorrectFileExt(QString& name, const QString& extension);

  /**
    * Check that internal MyMoney engine constants use the same
    * values as the KDE constants.
    */
  static void checkConstants();

  static QString variableCSS();

  /**
    * This method searches a KDE specific resource and applies country and
    * language settings during the search. Therefore, the parameter @p filename must contain
    * the characters '%1' which gets replaced with the language/country values.
    *
    * The search is performed in the following order (stopped immediately if a file was found):
    * - @c \%1 is replaced with <tt>_\<country\>.\<language\></tt>
    * - @c \%1 is replaced with <tt>_\<language\></tt>
    * - @c \%1 is replaced with <tt>_\<country\></tt>
    * - @c \%1 is replaced with the empty string
    *
    * @c \<country\> and @c \<language\> denote the respective KDE settings.
    *
    * Example: The KDE settings for country is Spain (es) and language is set
    * to Galician (gl). The code for looking up a file looks like this:
    *
    * @code
    *
    *  :
    *  QString fname = KMyMoneyUtils::findResource("appdata", "html/home%1.html")
    *  :
    *
    * @endcode
    *
    * The method calls KStandardDirs::findResource() with the following values for the
    * parameter @p filename:
    *
    * - <tt>html/home_es.gl.html</tt>
    * - <tt>html/home_gl.html</tt>
    * - <tt>html/home_es.html</tt>
    * - <tt>html/home.html</tt>
    *
    * @note See KStandardDirs::findResource() for details on the parameters
    */
  static QString findResource(QStandardPaths::StandardLocation type, const QString& filename);

  /**
    * This method returns the split referencing a stock account if
    * one exists in the transaction passed as @p t. If none is present
    * in @p t, an empty MyMoneySplit() object will be returned.
    *
    * @param t transaction to be checked for a stock account
    * @return MyMoneySplit object referencing a stock account or an
    *         empty MyMoneySplit object.
    */
  static const MyMoneySplit stockSplit(const MyMoneyTransaction& t);

  /**
    * This method analyses the splits of a transaction and returns
    * the type of transaction. Possible values are defined by the
    * KMyMoneyUtils::transactionTypeE enum.
    *
    * @param t const reference to the transaction
    *
    * @return KMyMoneyUtils::transactionTypeE value of the action
    */
  static transactionTypeE transactionType(const MyMoneyTransaction& t);

  /**
    * This method modifies a scheduled loan transaction such that all
    * references to automatic calculated values are resolved to actual values.
    *
    * @param schedule const reference to the schedule the transaction is based on
    * @param transaction reference to the transaction to be checked and modified
    * @param balances QMap of (account-id,balance) pairs to be used as current balance
    *                 for the calculation of interest. If map is empty, the engine
    *                 will be interrogated for current balances.
    */
  static void calculateAutoLoan(const MyMoneySchedule& schedule, MyMoneyTransaction& transaction, const QMap<QString, MyMoneyMoney>& balances);

  /**
    * Return next check number for account @a acc.
    */
  static QString nextCheckNumber(const MyMoneyAccount& acc);

  static void updateLastNumberUsed(const MyMoneyAccount& acc, const QString& number);

  static void setLastNumberUsed(const QString& num);

  static QString lastNumberUsed();

  /**
    * Returns previous number if offset is -1 or
    * the following number if offset is 1.
    */
  static QString getAdjacentNumber(const QString& number, int offset = 1);


  /**
  * remove any non-numeric characters from check number
  * to allow validity check
  */
  static quint64 numericPart(const QString & num);

  /**
    * Returns the text representing the reconcile flag. If @a text is @p true
    * then the full text will be returned otherwise a short form (usually one character).
    */
  static QString reconcileStateToString(eMyMoney::Split::State flag, bool text = false);

  /**
   * Returns the transaction for @a schedule. In case of a loan payment the
   * transaction will be modified by calculateAutoLoan().
   * The ID of the transaction as well as the entryDate will be reset.
   *
   * @returns adjusted transaction
   */
  static MyMoneyTransaction scheduledTransaction(const MyMoneySchedule& schedule);

  /**
   * This method replaces the deprecated QApplication::mainWidget() from Qt 3.x.
   * It assumes that there is only one KXmlGuiWindow in the application, and
   * returns it.
   *
   * @return the first KXmlGuiWindow found in QApplication::topLevelWidgets()
   */
  static KXmlGuiWindow* mainWindow();

  /**
    * This method sets the button text and icons to the KDE standard ones
    * for the QWizard passed as argument.
    */
  static void updateWizardButtons(QWizard *);

  static void dissectTransaction(const MyMoneyTransaction& transaction, const MyMoneySplit& split, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency, eMyMoney::Split::InvestmentTransactionType& transactionType);

  static void processPriceList(const MyMoneyStatement& st);

  /**
    * This method deletes security and associated price list but asks beforehand.
    */
  static void deleteSecurity(const MyMoneySecurity &security, QWidget *parent = nullptr);

  /**
   * Check whether the url links to an existing file or not
   * @returns whether the file exists or not
   */
  static bool fileExists(const QUrl &url);

  static bool newPayee(const QString& newnameBase, QString& id);

  static void newTag(const QString& newnameBase, QString& id);

  /**
    * Creates a new institution entry in the MyMoneyFile engine
    *
    * @param institution MyMoneyInstitution object containing the data of
    *                    the institution to be created.
    */
  static void newInstitution(MyMoneyInstitution& institution);
};

#endif
