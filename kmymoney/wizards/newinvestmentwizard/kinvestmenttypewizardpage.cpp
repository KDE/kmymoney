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
  QStringListModel *model = new QStringListModel();
  QStringList types;
  types << i18n("Stock") << i18n("Mutual Fund") << i18n("Bond");
  model->setStringList(types);
  model->sort(0, Qt::AscendingOrder);

  m_securityType->setModel(model);

  // Register the fields with the QWizard
  registerField("securityType", m_securityType, "currentText", SIGNAL(currentIndexChanged(QString)));
}

void KInvestmentTypeWizardPage::init2(const MyMoneySecurity& security)
{
  //get the current text of the security and set the combo index accordingly
  QString text = KMyMoneyUtils::securityTypeToString(security.securityType());
  for (int i = 0; i < m_securityType->count(); ++i) {
    if (m_securityType->itemText(i) == text)
      m_securityType->setCurrentIndex(i);
  }
}

void KInvestmentTypeWizardPage::setIntroLabelText(const QString& text)
{
  m_introLabel->setText(text);
}
