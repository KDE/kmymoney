/***************************************************************************
                             kgeneralloaninfopage.cpp
                             -------------------
    begin                : Tue Sep 25 2006
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kgeneralloaninfopage.h"
#include "kgeneralloaninfopage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QPushButton>
#include <QSpinBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kgeneralloaninfopage.h"

#include "kmymoneydateinput.h"
#include "kmymoneyedit.h"
#include "kmymoneyfrequencycombo.h"
#include "kmymoneypayeecombo.h"
#include "kmymoneywizardpage.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kgeneralloaninfopage_p.h"
#include "kloandetailspage.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "kguiutils.h"
#include "wizardpage.h"

using namespace eMyMoney;

namespace NewAccountWizard
{
  GeneralLoanInfoPage::GeneralLoanInfoPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new GeneralLoanInfoPagePrivate(wizard), StepDetails, this, wizard)
  {
    Q_D(GeneralLoanInfoPage);
    d->ui->setupUi(this);
    d->m_firstTime = true;
    d->m_mandatoryGroup->add(d->ui->m_payee);

    // remove the unsupported payment and compounding frequencies and setup default
    d->ui->m_paymentFrequency->removeItem((int)Schedule::Occurrence::Once);
    d->ui->m_paymentFrequency->removeItem((int)Schedule::Occurrence::EveryOtherYear);
    d->ui->m_paymentFrequency->setCurrentItem((int)Schedule::Occurrence::Monthly);
    d->ui->m_compoundFrequency->removeItem((int)Schedule::Occurrence::Once);
    d->ui->m_compoundFrequency->removeItem((int)Schedule::Occurrence::EveryOtherYear);
    d->ui->m_compoundFrequency->setCurrentItem((int)Schedule::Occurrence::Monthly);

    slotLoadWidgets();

    connect(d->ui->m_payee, &KMyMoneyMVCCombo::createItem, wizard, &KMyMoneyWizard::createPayee);
    connect(d->ui->m_anyPayments, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), object(),  &KMyMoneyWizardPagePrivate::completeStateChanged);
    connect(d->ui->m_recordings, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), object(), &KMyMoneyWizardPagePrivate::completeStateChanged);

    connect(d->ui->m_interestType, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), object(),  &KMyMoneyWizardPagePrivate::completeStateChanged);
    connect(d->ui->m_interestChangeDateEdit, &KMyMoneyDateInput::dateChanged, object(),  &KMyMoneyWizardPagePrivate::completeStateChanged);
    connect(d->ui->m_openingBalance, &KMyMoneyEdit::textChanged, object(),  &KMyMoneyWizardPagePrivate::completeStateChanged);

    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &GeneralLoanInfoPage::slotLoadWidgets);
  }

  GeneralLoanInfoPage::~GeneralLoanInfoPage()
  {
  }

  KMyMoneyWizardPage* GeneralLoanInfoPage::nextPage() const
  {
    Q_D(const GeneralLoanInfoPage);
    return d->m_wizard->d_func()->m_loanDetailsPage;
  }

  bool GeneralLoanInfoPage::recordAllPayments() const
  {
    Q_D(const GeneralLoanInfoPage);
    bool rc = true;     // all payments
    if (d->ui->m_recordings->isEnabled()) {
        if (d->ui->m_recordings->currentIndex() != 0)
          rc = false;
      }
    return rc;
  }

  void GeneralLoanInfoPage::enterPage()
  {
    Q_D(GeneralLoanInfoPage);
    if (d->m_firstTime) {
        // setup default dates to last of this month and one year on top
        QDate firstDay(QDate::currentDate().year(), QDate::currentDate().month(), 1);
        d->ui->m_firstPaymentDate->setDate(firstDay.addMonths(1).addDays(-1));
        d->ui->m_interestChangeDateEdit->setDate(d->ui->m_firstPaymentDate->date().addYears(1));;
        d->m_firstTime = false;
      }
  }

  bool GeneralLoanInfoPage::isComplete() const
  {
    Q_D(const GeneralLoanInfoPage);
    d->m_wizard->d_func()->setStepHidden(StepPayout, !d->m_wizard->openingBalance().isZero());
    bool rc = KMyMoneyWizardPage::isComplete();
    if (!rc) {
        d->m_wizard->d_func()->m_nextButton->setToolTip(i18n("No payee supplied"));
      }

    // fixup availability of items on this page
    d->ui->m_recordings->setDisabled(d->ui->m_anyPayments->currentIndex() == 0);

    d->ui->m_interestFrequencyAmountEdit->setDisabled(d->ui->m_interestType->currentIndex() == 0);
    d->ui->m_interestFrequencyUnitEdit->setDisabled(d->ui->m_interestType->currentIndex() == 0);
    d->ui->m_interestChangeDateEdit->setDisabled(d->ui->m_interestType->currentIndex() == 0);

    d->ui->m_openingBalance->setDisabled(recordAllPayments());

    if (d->ui->m_openingBalance->isEnabled() && d->ui->m_openingBalance->lineedit()->text().length() == 0) {
        rc = false;
        d->m_wizard->d_func()->m_nextButton->setToolTip(i18n("No opening balance supplied"));
      }

    if (rc
        && (d->ui->m_interestType->currentIndex() != 0)
        && (d->ui->m_interestChangeDateEdit->date() <= d->ui->m_firstPaymentDate->date())) {
        rc = false;
        d->m_wizard->d_func()->m_nextButton->setToolTip(i18n("An interest change can only happen after the first payment"));
      }
    return rc;
  }

  const MyMoneyAccount& GeneralLoanInfoPage::parentAccount()
  {
    Q_D(GeneralLoanInfoPage);
    return (d->ui->m_loanDirection->currentIndex() == 0)
        ? MyMoneyFile::instance()->liability()
        : MyMoneyFile::instance()->asset();
  }

  QWidget* GeneralLoanInfoPage::initialFocusWidget() const
  {
    Q_D(const GeneralLoanInfoPage);
    return d->ui->m_loanDirection;
  }

  void GeneralLoanInfoPage::slotLoadWidgets()
  {
    Q_D(GeneralLoanInfoPage);
    d->ui->m_payee->loadPayees(MyMoneyFile::instance()->payeeList());
  }
  
}
