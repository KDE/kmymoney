/***************************************************************************
                         namewizardpage  -  description
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

#include "namewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_namewizardpage.h"

NameWizardPage::NameWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::NameWizardPage)
{
  ui->setupUi(this);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("payeeEdit", ui->m_payeeEdit, "selectedItem", SIGNAL(itemSelected(QString)));
  connect(ui->m_nameEdit, &QLineEdit::textChanged, this, &QWizardPage::completeChanged);
}

NameWizardPage::~NameWizardPage()
{
  delete ui;
}

/**
 * Update the "Next" button
 */
bool NameWizardPage::isComplete() const
{
  return !ui->m_nameEdit->text().isEmpty();
}

void NameWizardPage::initializePage()
{
  if (field("borrowButton").toBool()) {
    ui->m_generalReceiverText->setText(i18n("To whom do you make payments?"));
    ui->m_receiverLabel->setText(i18n("Payments to"));
  } else if (field("lendButton").toBool()) {
    ui->m_generalReceiverText->setText(i18n("From whom do you expect payments?"));
    ui->m_receiverLabel->setText(i18n("Payments from"));
  }
  ui->m_nameEdit->setFocus();
}
