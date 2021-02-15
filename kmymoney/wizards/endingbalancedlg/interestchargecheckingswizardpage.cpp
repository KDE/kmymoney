/***************************************************************************
                        interestchargecheckingswizardpage.cpp - description
                            -------------------
   begin                : Sun Jul 18 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "interestchargecheckingswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interestchargecheckingswizardpage.h"

InterestChargeCheckingsWizardPage::InterestChargeCheckingsWizardPage(QWidget *parent) :
  QWizardPage(parent),
  ui(new Ui::InterestChargeCheckingsWizardPage)
{
  ui->setupUi(this);
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("interestDateEdit", ui->m_interestDateEdit, "date", SIGNAL(dateChanged(QDate)));
  registerField("chargesDateEdit", ui->m_chargesDateEdit, "date", SIGNAL(dateChanged(QDate)));

  registerField("interestEdit", ui->m_interestEdit, "value", SIGNAL(textChanged()));
  registerField("interestEditValid", ui->m_interestEdit, "valid", SIGNAL(textChanged()));
  registerField("chargesEdit", ui->m_chargesEdit, "value", SIGNAL(textChanged()));
  registerField("chargesEditValid", ui->m_chargesEdit, "valid", SIGNAL(textChanged()));

  registerField("interestCategoryEdit", ui->m_interestCategoryEdit, "selectedItem", SIGNAL(itemSelected(QString)));
  registerField("chargesCategoryEdit", ui->m_chargesCategoryEdit, "selectedItem", SIGNAL(itemSelected(QString)));

  registerField("payeeEdit", ui->m_payeeEdit, "selectedItem", SIGNAL(itemSelected(QString)));

  connect(ui->m_interestEdit, &AmountEdit::textChanged, this, &QWizardPage::completeChanged);
  connect(ui->m_interestCategoryEdit, &QComboBox::editTextChanged, this, &QWizardPage::completeChanged);
  connect(ui->m_chargesEdit, &AmountEdit::textChanged, this, &QWizardPage::completeChanged);
  connect(ui->m_chargesCategoryEdit, &QComboBox::editTextChanged, this, &QWizardPage::completeChanged);
}

InterestChargeCheckingsWizardPage::~InterestChargeCheckingsWizardPage()
{
  delete ui;
}

bool InterestChargeCheckingsWizardPage::isComplete() const
{
  auto cnt1 = !ui->m_interestEdit->value().isZero() + !ui->m_interestCategoryEdit->selectedItem().isEmpty();
  auto cnt2 = !ui->m_chargesEdit->value().isZero() + !ui->m_chargesCategoryEdit->selectedItem().isEmpty();
  if (cnt1 == 1 || cnt2 == 1)
    return false;

  return true;
}

