/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kbrokeragepage.h"
#include "kbrokeragepage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QLabel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kbrokeragepage.h"

#include "kmymoneycurrencyselector.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kaccounttypepage.h"
#include "kaccounttypepage_p.h"
#include "khierarchypage.h"
#include "kinstitutionpage.h"
#include "kinstitutionpage_p.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "wizardpage.h"

namespace NewAccountWizard
{
  BrokeragePage::BrokeragePage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new BrokeragePagePrivate(wizard), StepBroker, this, wizard)
  {
    Q_D(BrokeragePage);
    d->ui->setupUi(this);
    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &BrokeragePage::slotLoadWidgets);
  }

  BrokeragePage::~BrokeragePage()
  {
  }

  void BrokeragePage::slotLoadWidgets()
  {
    Q_D(BrokeragePage);
    d->ui->m_brokerageCurrency->update(QString("x"));
  }

  void BrokeragePage::enterPage()
  {
    Q_D(BrokeragePage);
    // assign the currency of the investment account to the
    // brokerage account if nothing else has ever been selected
    if (d->ui->m_brokerageCurrency->security().id().isEmpty()) {
        d->ui->m_brokerageCurrency->setSecurity(d->m_wizard->d_func()->m_accountTypePage->d_func()->ui->m_currencyComboBox->security());
      }

    // check if the institution relevant fields should be enabled or not
    bool enabled = d->m_wizard->d_func()->m_institutionPage->d_func()->ui->m_accountNumber->isEnabled();
    d->ui->m_accountNumberLabel->setEnabled(enabled);
    d->ui->m_accountNumber->setEnabled(enabled);
    d->ui->m_ibanLabel->setEnabled(enabled);
    d->ui->m_iban->setEnabled(enabled);
  }

  QWidget* BrokeragePage::initialFocusWidget() const
  {
    Q_D(const BrokeragePage);
    return d->ui->m_createBrokerageButton;
  }

  KMyMoneyWizardPage* BrokeragePage::nextPage() const
  {
    Q_D(const BrokeragePage);
    return d->m_wizard->d_func()->m_hierarchyPage;
  }
}
