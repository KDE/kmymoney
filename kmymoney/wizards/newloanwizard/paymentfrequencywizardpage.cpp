/***************************************************************************
                         paymentfrequencywizardpage  -  description
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

#include "paymentfrequencywizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_paymentfrequencywizardpage.h"

#include "mymoneyenums.h"

PaymentFrequencyWizardPage::PaymentFrequencyWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::PaymentFrequencyWizardPage)
{
  ui->setupUi(this);

  registerField("paymentFrequencyUnitEdit", ui->m_paymentFrequencyUnitEdit, "data", SIGNAL(currentDataChanged(QVariant)));
  ui->m_paymentFrequencyUnitEdit->setCurrentIndex(ui->m_paymentFrequencyUnitEdit->findData(QVariant((int)eMyMoney::Schedule::Occurrence::Monthly), Qt::UserRole, Qt::MatchExactly));
}

PaymentFrequencyWizardPage::~PaymentFrequencyWizardPage()
{
  delete ui;
}
