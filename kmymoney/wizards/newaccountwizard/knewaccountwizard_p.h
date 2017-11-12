/***************************************************************************
                             knewaccountwizard_p.h
                             -------------------
    begin                : Tue Sep 25 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWACCOUNTWIZARD_P_H
#define KNEWACCOUNTWIZARD_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcombobox.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneywizardpage.h"
#include <kmymoneywizard.h>
#include <kmymoneydateinput.h>
#include <kmymoneycurrencyselector.h>
#include <mymoneyaccount.h>
#include <kmymoneyedit.h>
#include "accountsproxymodel.h"

#include "ui_kinstitutionpagedecl.h"
#include "ui_kaccounttypepagedecl.h"
#include "ui_kbrokeragepagedecl.h"
#include "ui_kschedulepagedecl.h"
#include "ui_kgeneralloaninfopagedecl.h"
#include "ui_kloandetailspagedecl.h"
#include "ui_kloanpaymentpagedecl.h"
#include "ui_kloanschedulepagedecl.h"
#include "ui_kloanpayoutpagedecl.h"
#include "ui_khierarchypagedecl.h"
#include "ui_kaccountsummarypagedecl.h"

class Wizard;
class MyMoneyInstitution;

namespace NewAccountWizard
{


class KInstitutionPageDecl : public QWidget, public Ui::KInstitutionPageDecl
{
public:
  KInstitutionPageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class InstitutionPage : public KInstitutionPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  InstitutionPage(Wizard* parent);
  ~InstitutionPage();
  KMyMoneyWizardPage* nextPage(void) const;

  QWidget* initialFocusWidget(void) const {
    return m_institutionComboBox;
  }

  /**
    * Returns the information about an institution if entered by
    * the user. If the id field is empty, then he did not enter
    * such information.
    */
  const MyMoneyInstitution& institution(void) const;

  void selectExistingInstitution(const QString& id);

private slots:
  void slotLoadWidgets(void);
  void slotNewInstitution(void);
  void slotSelectInstitution(int index);

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};


class KAccountTypePageDecl : public QWidget, public Ui::KAccountTypePageDecl
{
public:
  KAccountTypePageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class AccountTypePage : public KAccountTypePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  AccountTypePage(Wizard* parent);
  virtual bool isComplete(void) const;
  KMyMoneyWizardPage* nextPage(void) const;

  QWidget* initialFocusWidget(void) const {
    return m_accountName;
  }

  eMyMoney::Account accountType(void) const;
  const MyMoneyAccount& parentAccount(void);
  bool allowsParentAccount(void) const;
  const MyMoneySecurity& currency(void) const;

  void setAccount(const MyMoneyAccount& acc);

private:
  void hideShowPages(eMyMoney::Account i) const;
  void priceWarning(bool);

private slots:
  void slotUpdateType(int i);
  void slotUpdateCurrency(void);
  void slotUpdateConversionRate(const QString&);
  void slotGetOnlineQuote(void);
  void slotPriceWarning(void);

private:
  bool m_showPriceWarning;
};


class KBrokeragePageDecl : public QWidget, public Ui::KBrokeragePageDecl
{
public:
  KBrokeragePageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class BrokeragePage : public KBrokeragePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  BrokeragePage(Wizard* parent);
  KMyMoneyWizardPage* nextPage(void) const;
  void enterPage(void);

  QWidget* initialFocusWidget(void) const {
    return m_createBrokerageButton;
  }

private slots:
  void slotLoadWidgets(void);
};


class KSchedulePageDecl : public QWidget, public Ui::KSchedulePageDecl
{
public:
  KSchedulePageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class CreditCardSchedulePage : public KSchedulePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  CreditCardSchedulePage(Wizard* parent);
  KMyMoneyWizardPage* nextPage(void) const;
  virtual bool isComplete(void) const;
  void enterPage(void);

  QWidget* initialFocusWidget(void) const {
    return m_reminderCheckBox;
  }

private slots:
  void slotLoadWidgets(void);
};


class KGeneralLoanInfoPageDecl : public QWidget, public Ui::KGeneralLoanInfoPageDecl
{
public:
  KGeneralLoanInfoPageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class GeneralLoanInfoPage : public KGeneralLoanInfoPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  GeneralLoanInfoPage(Wizard* parent);
  KMyMoneyWizardPage* nextPage(void) const;
  virtual bool isComplete(void) const;
  void enterPage(void);
  const MyMoneyAccount& parentAccount(void);

  QWidget* initialFocusWidget(void) const {
    return m_loanDirection;
  }

  /**
   * Returns @p true if the user decided to record all payments, @p false otherwise.
   */
  bool recordAllPayments(void) const;

private slots:
  void slotLoadWidgets(void);

private:
  bool      m_firstTime;
};


class KLoanDetailsPageDecl : public QWidget, public Ui::KLoanDetailsPageDecl
{
public:
  KLoanDetailsPageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class LoanDetailsPage : public KLoanDetailsPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  LoanDetailsPage(Wizard* parent);
  void enterPage(void);
  KMyMoneyWizardPage* nextPage(void) const;
  virtual bool isComplete(void) const;

  QWidget* initialFocusWidget(void) const {
    return m_paymentDue;
  }

  /**
   * This method returns the number of payments depending on
   * the settings of m_termAmount and m_termUnit widgets
   */
  int term(void) const;

private:
  /**
   * This method is used to update the term widgets
   * according to the length of the given @a term.
   * The term is also converted into a string and returned.
   */
  QString updateTermWidgets(const double term);

private:
  bool                m_needCalculate;

private slots:
  void slotValuesChanged(void);
  void slotCalculate(void);
};


class KLoanPaymentPageDecl : public QWidget, public Ui::KLoanPaymentPageDecl
{
public:
  KLoanPaymentPageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class LoanPaymentPage : public KLoanPaymentPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  LoanPaymentPage(Wizard* parent);
  ~LoanPaymentPage();

  KMyMoneyWizardPage* nextPage(void) const;

  void enterPage(void);

  /**
   * This method returns the sum of the additional fees
   */
  MyMoneyMoney additionalFees(void) const;

  /**
   * This method returns the base payment, that's principal and interest
   */
  MyMoneyMoney basePayment(void) const;

  /**
   * This method returns the splits that make up the additional fees in @p list.
   * @note The splits may contain assigned ids which the caller must remove before
   * adding the splits to a MyMoneyTransaction object.
   */
  void additionalFeesSplits(QList<MyMoneySplit>& list);

protected slots:
  void slotAdditionalFees(void);

protected:
  void updateAmounts(void);

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};


class KLoanSchedulePageDecl  : public QWidget, public Ui::KLoanSchedulePageDecl
{
public:
  KLoanSchedulePageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class LoanSchedulePage : public KLoanSchedulePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  LoanSchedulePage(Wizard* parent);
  void enterPage(void);

  KMyMoneyWizardPage* nextPage(void) const;

  /**
   * This method returns the due date of the first payment to be recorded.
   */
  QDate firstPaymentDueDate(void) const;

  QWidget* initialFocusWidget(void) const {
    return m_interestCategory;
  }

private slots:
  void slotLoadWidgets(void);
  void slotCreateCategory(const QString& name, QString& id);
};


class KLoanPayoutPageDecl : public QWidget, public Ui::KLoanPayoutPageDecl
{
public:
  KLoanPayoutPageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class LoanPayoutPage : public KLoanPayoutPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  LoanPayoutPage(Wizard* parent);
  void enterPage(void);
  virtual bool isComplete(void) const;

  KMyMoneyWizardPage* nextPage(void) const;

  QWidget* initialFocusWidget(void) const {
    return m_noPayoutTransaction;
  }

  QString payoutAccountId(void) const;

private slots:
  void slotLoadWidgets(void);
  void slotCreateAssetAccount(void);
  void slotButtonsToggled(void);
};

class HierarchyFilterProxyModel : public AccountsProxyModel
{
  Q_OBJECT

public:
  HierarchyFilterProxyModel(QObject *parent = 0);

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
};

class KHierarchyPageDecl : public QWidget, public Ui::KHierarchyPageDecl
{
public:
  KHierarchyPageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class HierarchyPage : public KHierarchyPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT

public:
  HierarchyPage(Wizard* parent);
  void enterPage(void);
  KMyMoneyWizardPage* nextPage(void) const;
  QWidget* initialFocusWidget(void) const {
    return m_parentAccounts;
  }
  const MyMoneyAccount& parentAccount(void);

  bool isComplete(void) const;

protected slots:
  void parentAccountChanged();

private:
  HierarchyFilterProxyModel *m_filterProxyModel;
  MyMoneyAccount             m_parentAccount;
};


class KAccountSummaryPageDecl : public QWidget, public Ui::KAccountSummaryPageDecl
{
public:
  KAccountSummaryPageDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class AccountSummaryPage : public KAccountSummaryPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  AccountSummaryPage(Wizard* parent);
  void enterPage(void);
  QWidget* initialFocusWidget(void) const {
    return m_dataList;
  }
};

} // namespace

#endif
