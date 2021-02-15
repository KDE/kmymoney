/***************************************************************************
                         paymenteditwizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "paymenteditwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_paymenteditwizardpage.h"

PaymentEditWizardPage::PaymentEditWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::PaymentEditWizardPage)
{
  ui->setupUi(this);
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("newPaymentEdit", ui->m_newPaymentEdit, "value", SIGNAL(textChanged()));
  registerField("newPaymentEditValid", ui->m_newPaymentEdit, "valid", SIGNAL(textChanged()));

  connect(ui->m_newPaymentEdit, &AmountEdit::textChanged, this, &QWizardPage::completeChanged);

  ui->m_newPaymentEdit->setAllowEmpty(true);
}

PaymentEditWizardPage::~PaymentEditWizardPage()
{
  delete ui;
}
