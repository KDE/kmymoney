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

#include <QStringListModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"

KInvestmentTypeWizardPage::KInvestmentTypeWizardPage(QWidget *parent)
    : KInvestmentTypeWizardPageDecl(parent)
{
  m_securityType->addItem(i18nc("Security type", "Stock"), (int)eMyMoney::Security::Stock);
  m_securityType->addItem(i18nc("Security type", "Mutual Fund"), (int)eMyMoney::Security::MutualFund);
  m_securityType->addItem(i18nc("Security type", "Bond"), (int)eMyMoney::Security::Bond);
  registerField("securityType", m_securityType, "currentData", SIGNAL(currentIndexChanged(int)));
}

void KInvestmentTypeWizardPage::init2(const MyMoneySecurity& security)
{
  //get the current text of the security and set the combo index accordingly
  auto text = MyMoneySecurity::securityTypeToString(security.securityType());
  for (int i = 0; i < m_securityType->count(); ++i) {
    if (m_securityType->itemText(i) == text)
      m_securityType->setCurrentIndex(i);
  }
}

void KInvestmentTypeWizardPage::setIntroLabelText(const QString& text)
{
  m_introLabel->setText(text);
}
