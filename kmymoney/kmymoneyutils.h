/*
    SPDX-FileCopyrightText: 2000-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYUTILS_H
#define KMYMONEYUTILS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardPaths>
#include <QMap>
#include <QUrl>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

#ifndef KMYMONEY_DEPRECATED
#  define KMYMONEY_DEPRECATED Q_DECL_DEPRECATED
#endif

class QIcon;

/**
  * @author Thomas Baumgart
  */

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
class MyMoneyForecast;
class SplitModel;

namespace eMyMoney {
namespace Schedule {
enum class Occurrence;
enum class PaymentType;
enum class WeekendOption;
enum class Type;
}
namespace Split {
enum class State;
enum class InvestmentTransactionType;
}
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
        InvestmentTransaction,
    };

    static const int maxHomePageItems = 5;

    KMyMoneyUtils() = default;
    ~KMyMoneyUtils() = default;

    /**
      * This method is used to convert the occurrence type from its
      * internal representation into a human readable format.
      *
      * @param occurrence numerical representation of the MyMoneySchedule
      *                  occurrence type
      *
      * @return QString representing the human readable format translated according to the language catalog
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
      * @return QString representing the human readable format translated according to the language catalog
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
      * @return QString representing the human readable format translated according to the language catalog
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
      * @return QString representing the human readable format translated according to the language catalog
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

    /**
     * Returns common CSS stylesheet used by HTML renderers.
     *
     * Note that some elements are generated on the fly based on the user preferences.
     *
     * @param baseStylesheet filename of the custom CSS stylesheet file that will be used as a base.
     *                           If empty or null, a stock embedded stylesheet will be used.
     *
     *
     * @retval The processed CSS stylesheet with base and dynamic elements combined.
     */
    static QString getStylesheet(QString baseStylesheet = QString());

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
      * Returns the next check number for account @a acc. No check is performed, if the
      * number is already in use.
      */
    static QString nextCheckNumber(const MyMoneyAccount& acc);

    /**
      * Returns the next check free number for account @a acc.
      */
    static QString nextFreeCheckNumber(const MyMoneyAccount& acc);

    /**
     * Returns previous number if offset is -1 or
     * the following number if offset is 1.
     */
    static QString getAdjacentNumber(const QString& number, int offset = 1);

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

    static void dissectInvestmentTransaction(const QModelIndex &investSplitIdx, QModelIndex &assetAccountSplitIdx, SplitModel* feeSplitModel, SplitModel* interestSplitModel, MyMoneySecurity &security, MyMoneySecurity &currency, eMyMoney::Split::InvestmentTransactionType &transactionType);
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

    static QString downloadFile(const QUrl &url);

    static bool newPayee(const QString& newnameBase, QString& id);

    static void newTag(const QString& newnameBase, QString& id);

    /**
      * Creates a new institution entry in the MyMoneyFile engine
      *
      * @param institution MyMoneyInstitution object containing the data of
      *                    the institution to be created.
      */
    static void newInstitution(MyMoneyInstitution& institution);

    static QDebug debug();

    static MyMoneyForecast forecast();

    static bool canUpdateAllAccounts();

    static void showStatementImportResult(const QStringList& resultMessages, uint statementCount);

    /**
      * This method returns a double converted into a QString
      * after removing any group separators, trailing zeros,
      * and decimal separators.
      *
      * @param val reference to a qreal value to be converted
      * @param loc reference to a Qlocale converter
      * @param f format specifier:
      *          e - format as [-]9.9e[+|-]999
      *          E - format as [-]9.9E[+|-]999
      *          f - format as [-]9.9
      *          g - use e or f format, whichever is the most concise
      *          G - use E or f format, whichever is the most concise
      * @param prec precision representing the number of digits
      *             after the decimal point ('e', 'E' and 'f' formats)
      *             or the maximum number of significant digits
      *             (trailing zeroes are omitted) ('g' and 'G' formats)
      * @return QString object containing the converted value
      */
    static QString normalizeNumericString(const qreal& val, const QLocale& loc, const char f = 'g', const int prec = 6);

    /**
     * This method returns the tab order based on the configuration found in
     * the parameter identified in @a name in the section [TabOrder] of
     * the global configuration file kmymoneyrc. If no setting is found in the
     * configuration file, the @a defaultTabOrder is used.
     *
     * @sa setupTabOrder(), storeTabOrder()
     */
    static QStringList tabOrder(const QString& name, const QStringList& defaultTabOrder);

    /**
     * This method sets the tab order based on the order provided
     * in @a tabOrder. The named widgets are searched under
     * @a parent.
     *
     * @note the widgets must carry the respective object name.
     *
     * @sa tabOrder(), storeTabOrder()
     */
    static void setupTabOrder(QWidget* parent, const QStringList& tabOrder);

    /**
     * This method stores the tab order to the parameter identified
     * in @a name in the section [TabOrder] of the global configuration
     * file kmymoneyrc.
     *
     * @param name name of the widget type
     * @param tabOrder QStringList of widget names
     *
     * @sa setupTabOrder()
     */
    static void storeTabOrder(const QString& name, const QStringList& tabOrder);

    static bool tabFocusHelper(QWidget* topLevelWidget, bool next);
};

#endif
