/***************************************************************************
                         summaryeditwizardpage  -  description
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

#include "summaryeditwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_summaryeditwizardpage.h"

SummaryEditWizardPage::SummaryEditWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::SummaryEditWizardPage)
{
  ui->setupUi(this);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("affectedPayments", ui->m_affectedPayments, "text");
}

SummaryEditWizardPage::~SummaryEditWizardPage()
{
  delete ui;
}

void SummaryEditWizardPage::initializePage()
{
  ui->m_payment7->setText(field("payment6").toString());
  ui->m_additionalFees7->setText(field("additionalCost").toString());
  ui->m_totalPayment7->setText(field("periodicPayment").toString());
  ui->m_interestRate7->setText(field("interestRate6").toString());
  ui->m_startDateChanges->setText(QLocale().toString(field("effectiveChangeDateEdit").toDate()));
}
