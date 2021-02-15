/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

#include "ui_additionalfeeswizardpage.h"

#include "knewloanwizard.h"
#include "knewloanwizard_p.h"
/// @todo port to new model code
// #include "ksplittransactiondlg.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"

AdditionalFeesWizardPage::AdditionalFeesWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::AdditionalFeesWizardPage)
{
  ui->setupUi(this);

  registerField("additionalCost", ui->m_additionalCost, "text");
  registerField("periodicPayment", ui->m_periodicPayment, "text");
  registerField("basePayment", ui->m_basePayment, "text");
  // load button icons
  KGuiItem additionalFeeButtonItem(i18n("&Additional fees..."),
                                   0, //QIcon::fromTheme("document-new"),
                                   i18n("Enter additional fees"),
                                   i18n("Use this to add any additional fees other than principal and interest contained in your periodical payments."));
  KGuiItem::assign(ui->m_additionalFeeButton, additionalFeeButtonItem);
  connect(ui->m_additionalFeeButton, &QAbstractButton::clicked, this, &AdditionalFeesWizardPage::slotAdditionalFees);
}

AdditionalFeesWizardPage::~AdditionalFeesWizardPage()
{
  delete ui;
}

void AdditionalFeesWizardPage::slotAdditionalFees()
{
  /// @todo port to new model code
#if 0
  // KMessageBox::information(0, QString("Not yet implemented ... if you want to help, contact kmymoney-devel@kde.org"), QString("Development notice"));
  MyMoneyAccount account("Phony-ID", MyMoneyAccount());

  QMap<QString, MyMoneyMoney> priceInfo;
  QPointer<KSplitTransactionDlg> dlg = new KSplitTransactionDlg(qobject_cast<KNewLoanWizard*>(wizard())->d_func()->m_transaction, qobject_cast<KNewLoanWizard*>(wizard())->d_func()->m_split, account, false, !field("borrowButton").toBool(), MyMoneyMoney(), priceInfo);
  connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

  if (dlg->exec() == QDialog::Accepted) {
    qobject_cast<KNewLoanWizard*>(wizard())->d_func()->m_transaction = dlg->transaction();
    // sum up the additional fees
    MyMoneyMoney fees;
    foreach (const MyMoneySplit& it, qobject_cast<KNewLoanWizard*>(wizard())->d_func()->m_transaction.splits()) {
      if (it.accountId() != account.id()) {
        fees += it.value();
      }
    }
    setField("additionalCost", fees.formatMoney(qobject_cast<KNewLoanWizard*>(wizard())->d_func()->m_account.fraction(MyMoneyFile::instance()->security(qobject_cast<KNewLoanWizard*>(wizard())->d_func()->m_account.currencyId()))));
  }

  delete dlg;
#endif
  updatePeriodicPayment(qobject_cast<KNewLoanWizard*>(wizard())->d_func()->m_account);
}

void AdditionalFeesWizardPage::updatePeriodicPayment(const MyMoneyAccount& account)
{
  MyMoneyMoney base(ui->m_basePayment->text());
  MyMoneyMoney add(ui->m_additionalCost->text());

  ui->m_periodicPayment->setText((base + add).formatMoney(account.fraction(MyMoneyFile::instance()->security(account.currencyId()))));
}
