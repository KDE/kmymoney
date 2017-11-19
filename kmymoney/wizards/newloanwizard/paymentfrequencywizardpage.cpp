/***************************************************************************
                         paymentfrequencywizardpage  -  description
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
