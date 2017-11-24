/***************************************************************************
                         kinvestmenttypewizardpage  -  description
                            -------------------
   begin                : Sun Jun 27 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
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

#include "kinvestmenttypewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinvestmenttypewizardpage.h"

#include "mymoneysecurity.h"
#include "mymoneyenums.h"

KInvestmentTypeWizardPage::KInvestmentTypeWizardPage(QWidget *parent) :
  QWizardPage(parent),
  ui(new Ui::KInvestmentTypeWizardPage)
{
  ui->setupUi(this);
  ui->m_securityType->addItem(i18nc("Security type", "Stock"), (int)eMyMoney::Security::Stock);
  ui->m_securityType->addItem(i18nc("Security type", "Mutual Fund"), (int)eMyMoney::Security::MutualFund);
  ui->m_securityType->addItem(i18nc("Security type", "Bond"), (int)eMyMoney::Security::Bond);
  registerField("securityType", ui->m_securityType, "currentData", SIGNAL(currentIndexChanged(int)));
}

KInvestmentTypeWizardPage::~KInvestmentTypeWizardPage()
{
  delete ui;
}

void KInvestmentTypeWizardPage::init2(const MyMoneySecurity& security)
{
  //get the current text of the security and set the combo index accordingly
  auto text = MyMoneySecurity::securityTypeToString(security.securityType());
  for (auto i = 0; i < ui->m_securityType->count(); ++i) {
    if (ui->m_securityType->itemText(i) == text)
      ui->m_securityType->setCurrentIndex(i);
  }
}

void KInvestmentTypeWizardPage::setIntroLabelText(const QString& text)
{
  ui->m_introLabel->setText(text);
}
