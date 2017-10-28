/***************************************************************************
                         konlineupdatewizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "additionalfeeswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewloanwizard.h"
#include "ksplittransactiondlg.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"

AdditionalFeesWizardPage::AdditionalFeesWizardPage(QWidget *parent)
    : AdditionalFeesWizardPageDecl(parent)
{

  registerField("additionalCost", m_additionalCost, "text");
  registerField("periodicPayment", m_periodicPayment, "text");
  registerField("basePayment", m_basePayment, "text");
  // load button icons
  KGuiItem additionalFeeButtonItem(i18n("&Additional fees..."),
                                   0, //QIcon::fromTheme("document-new"),
                                   i18n("Enter additional fees"),
                                   i18n("Use this to add any additional fees other than principal and interest contained in your periodical payments."));
  KGuiItem::assign(m_additionalFeeButton, additionalFeeButtonItem);
  connect(m_additionalFeeButton, SIGNAL(clicked()), this, SLOT(slotAdditionalFees()));
}

void AdditionalFeesWizardPage::slotAdditionalFees()
{
  // KMessageBox::information(0, QString("Not yet implemented ... if you want to help, contact kmymoney-devel@kde.org"), QString("Development notice"));
  MyMoneyAccount account("Phony-ID", MyMoneyAccount());

  QMap<QString, MyMoneyMoney> priceInfo;
  QPointer<KSplitTransactionDlg> dlg = new KSplitTransactionDlg(qobject_cast<KNewLoanWizard*>(wizard())->m_transaction, qobject_cast<KNewLoanWizard*>(wizard())->m_split, account, false, !field("borrowButton").toBool(), MyMoneyMoney(), priceInfo);
  connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

  if (dlg->exec() == QDialog::Accepted) {
    qobject_cast<KNewLoanWizard*>(wizard())->m_transaction = dlg->transaction();
    // sum up the additional fees
    MyMoneyMoney fees;
    foreach (const MyMoneySplit& it, qobject_cast<KNewLoanWizard*>(wizard())->m_transaction.splits()) {
      if (it.accountId() != account.id()) {
        fees += it.value();
      }
    }
    setField("additionalCost", fees.formatMoney(qobject_cast<KNewLoanWizard*>(wizard())->m_account.fraction(MyMoneyFile::instance()->security(qobject_cast<KNewLoanWizard*>(wizard())->m_account.currencyId()))));
  }

  delete dlg;

  updatePeriodicPayment(qobject_cast<KNewLoanWizard*>(wizard())->m_account);
}

void AdditionalFeesWizardPage::updatePeriodicPayment(const MyMoneyAccount& account)
{
  MyMoneyMoney base(m_basePayment->text());
  MyMoneyMoney add(m_additionalCost->text());

  m_periodicPayment->setText((base + add).formatMoney(account.fraction(MyMoneyFile::instance()->security(account.currencyId()))));
}
