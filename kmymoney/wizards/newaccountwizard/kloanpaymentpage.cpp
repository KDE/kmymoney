/***************************************************************************
                             kloanpaymentpage.cpp
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

#include "kloanpaymentpage.h"
#include "kloanpaymentpage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kloanpaymentpage.h"

#include "kmymoneyedit.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kloandetailspage.h"
#include "kloandetailspage_p.h"
#include "kloanpaymentpage_p.h"
#include "kloanschedulepage.h"
#include "ksplittransactiondlg.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "wizardpage.h"

namespace NewAccountWizard
{
  LoanPaymentPage::LoanPaymentPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new LoanPaymentPagePrivate(wizard), StepFees, this, wizard)
  {
    Q_D(LoanPaymentPage);
    d->ui->setupUi(this);
    d->phonyAccount = MyMoneyAccount(QLatin1String("Phony-ID"), MyMoneyAccount());

    d->phonySplit.setAccountId(d->phonyAccount.id());
    d->phonySplit.setValue(MyMoneyMoney());
    d->phonySplit.setShares(MyMoneyMoney());

    d->additionalFeesTransaction.addSplit(d->phonySplit);

    connect(d->ui->m_additionalFeesButton, &QAbstractButton::clicked, this, &LoanPaymentPage::slotAdditionalFees);
  }

  LoanPaymentPage::~LoanPaymentPage()
  {
  }

  MyMoneyMoney LoanPaymentPage::basePayment() const
  {
    Q_D(const LoanPaymentPage);
    return d->m_wizard->d_func()->m_loanDetailsPage->d_func()->ui->m_paymentAmount->value();
  }

  MyMoneyMoney LoanPaymentPage::additionalFees() const
  {
    Q_D(const LoanPaymentPage);
    return d->additionalFees;
  }

  void LoanPaymentPage::additionalFeesSplits(QList<MyMoneySplit>& list)
  {
    Q_D(LoanPaymentPage);
    list.clear();

    QList<MyMoneySplit>::ConstIterator it;
    for (it = d->additionalFeesTransaction.splits().constBegin(); it != d->additionalFeesTransaction.splits().constEnd(); ++it) {
        if ((*it).accountId() != d->phonyAccount.id()) {
            list << (*it);
          }
      }
  }

  void LoanPaymentPage::updateAmounts()
  {
    Q_D(LoanPaymentPage);
    d->ui->m_additionalFees->setText(d->additionalFees.formatMoney(d->m_wizard->d_func()->currency().tradingSymbol(), d->m_wizard->d_func()->precision()));
    d->ui->m_totalPayment->setText((basePayment() + d->additionalFees).formatMoney(d->m_wizard->d_func()->currency().tradingSymbol(), d->m_wizard->d_func()->precision()));
  }

  void LoanPaymentPage::enterPage()
  {
    Q_D(LoanPaymentPage);
    const MyMoneySecurity& currency = d->m_wizard->d_func()->currency();

    d->ui->m_basePayment->setText(basePayment().formatMoney(currency.tradingSymbol(), d->m_wizard->d_func()->precision()));
    d->phonyAccount.setCurrencyId(currency.id());
    d->additionalFeesTransaction.setCommodity(currency.id());

    updateAmounts();
  }

  void LoanPaymentPage::slotAdditionalFees()
  {
    Q_D(LoanPaymentPage);
    QMap<QString, MyMoneyMoney> priceInfo;
    QPointer<KSplitTransactionDlg> dlg = new KSplitTransactionDlg(d->additionalFeesTransaction, d->phonySplit, d->phonyAccount, false, !d->m_wizard->moneyBorrowed(), MyMoneyMoney(), priceInfo);

    // connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

    if (dlg->exec() == QDialog::Accepted) {
        d->additionalFeesTransaction = dlg->transaction();
        // sum up the additional fees
        QList<MyMoneySplit>::ConstIterator it;

        d->additionalFees = MyMoneyMoney();
        for (it = d->additionalFeesTransaction.splits().constBegin(); it != d->additionalFeesTransaction.splits().constEnd(); ++it) {
            if ((*it).accountId() != d->phonyAccount.id()) {
                d->additionalFees += (*it).shares();
              }
          }
        updateAmounts();
      }

    delete dlg;
  }

  KMyMoneyWizardPage* LoanPaymentPage::nextPage() const
  {
    Q_D(const LoanPaymentPage);
    return d->m_wizard->d_func()->m_loanSchedulePage;
  }
  
}
