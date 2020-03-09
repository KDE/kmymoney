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

#include <QColor>
#include <QFont>
#include <QPixmap>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kguiitem.h>
#include <kxmlguiwindow.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneysecurity.h>
#include "mymoneyschedule.h"
#include <mymoneytransaction.h>

/**
  * @author Thomas Baumgart
  */

static QString m_lastNumberUsed;
class QWizard;
class KMyMoneyUtils
{
public:
  /**
    * This enum is used to describe the bits of an account type filter mask.
    * Each bit is used to define a specific account class. Multiple classes
    * can be specified by OR'ing multiple entries. The special entry @p last
    * marks the left most bit in the mask and is used by scanners of this
    * bitmask to determine the end of processing.
    */
  enum categoryTypeE {
    none =       0x000,         ///< no account class selected
    liability =  0x001,         ///< liability accounts selected
    asset =      0x002,         ///< asset accounts selected
    expense =    0x004,         ///< expense accounts selected
    income =     0x008,         ///< income accounts selected
    equity =     0x010,         ///< equity accounts selected
    checking =   0x020,         ///< checking accounts selected
    savings =    0x040,         ///< savings accounts selected
    investment = 0x080,         ///< investment accounts selected
    creditCard = 0x100,         ///< credit card accounts selected
    last =       0x200          ///< the leftmost bit in the mask
  };

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

  enum EnterScheduleResultCodeE {
    Cancel = 0,    // cancel the operation
    Enter,         // enter the schedule
    Skip,          // skip the schedule
    Ignore         // ignore the schedule
  };

  enum CanCloseAccountCodeE {
    AccountCanClose = 0,    // can close the account
    AccountBalanceNonZero,         // balance is non zero
    AccountChildrenOpen,          // account has open children account
    AccountScheduleReference         // account is referenced in a schedule
  };

  /**
   * Specify a page in the settings dialog
   */
  enum class SettingsPage {
    Undefined,
    Colors,
    Encryption,
    Fonts,
    Forecast,
    General,
    Gpg,
    Home,
    OnlineQuotes,
    Plugins,
    Register,
    Reports,
    Schedules,
  };

  static const int maxHomePageItems = 5;

  KMyMoneyUtils();
  ~KMyMoneyUtils();

  /**
    * This method is used to convert the internal representation of
    * an account type into a human readable format
    *
    * @param accountType numerical representation of the account type.
    *                    For possible values, see MyMoneyAccount::accountTypeE
    * @return QString representing the human readable form translated according to the language cataglogue
    *
    * @sa MyMoneyAccount::accountTypeToString()
    */
  static const QString accountTypeToString(const MyMoneyAccount::accountTypeE accountType);

  /**
    * This method is used to convert an account type from its
    * string form to the internal used numeric value.
    *
    * @param type reference to a QString containing the string to convert
    * @return accountTypeE containing the internal used numeric value. For possible
    *         values see MyMoneyAccount::accountTypeE
    */
  static MyMoneyAccount::accountTypeE stringToAccountType(const QString& type);

  /**
    * This method is used to convert a security type from its
    * string form to the internal used numeric value.
    *
    * @param txt reference to a QString containing the string to convert
    * @return eSECURITYTYPE containing the internal used numeric value. For possible
    *         values see MyMoneySecurity::eSECURITYTYPE
    */
  static MyMoneySecurity::eSECURITYTYPE stringToSecurity(const QString& txt);

  /**
    * This method is used to convert the internal representation of
    * an security type into a human readable format
    *
    * @param securityType enumerated representation of the security type.
    *                     For possible values, see MyMoneySecurity::eSECURITYTYPE
    * @return QString representing the human readable form translated according to the language cataglogue
    *
    * @sa MyMoneySecurity::securityTypeToString()
    */
  static const QString securityTypeToString(const MyMoneySecurity::eSECURITYTYPE securityType);

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
  static const QString occurrenceToString(const MyMoneySchedule::occurrenceE occurrence);

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
  static const QString paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType);

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
  static const QString weekendOptionToString(MyMoneySchedule::weekendOptionE weekendOption);

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
  static const QString scheduleTypeToString(MyMoneySchedule::typeE type);

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
  static QString findResource(const char* type, const QString& filename);

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
  static QString reconcileStateToString(MyMoneySplit::reconcileFlagE flag, bool text = false);

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

  /**
    * This method overlays an icon over another one, to get a composite one
    * eg. an icon to add accounts
    */
  static QPixmap overlayIcon(const QString source, const QString overlay, const Qt::Corner corner = Qt::BottomRightCorner, int size = 0);



};

#endif
