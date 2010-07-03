/***************************************************************************
                         kinvestmenttypewizardpage  -  description
                            -------------------
   begin                : Sun Jun 27 2010
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

#include "kinvestmenttypewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

KInvestmentTypeWizardPage::KInvestmentTypeWizardPage(QWidget *parent)
    : KInvestmentTypeWizardPageDecl(parent)
{
}

void KInvestmentTypeWizardPage::init2(const MyMoneySecurity& security)
{
  m_securityType->setItemText(m_securityType->currentIndex(), KMyMoneyUtils::securityTypeToString(security.securityType()));
  // Register the fields with the QWizard
  registerField("securityType", m_securityType, "currentText", SIGNAL(currentIndexChanged(const QString&)));
}

void KInvestmentTypeWizardPage::setIntroLabelText(const QString& text)
{
  m_introLabel->setText(text);
}

#include "kinvestmenttypewizardpage.moc"

