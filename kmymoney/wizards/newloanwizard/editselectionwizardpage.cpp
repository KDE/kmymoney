/***************************************************************************
                         editselectionwizardpage  -  description
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

#include "editselectionwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_editselectionwizardpage.h"

EditSelectionWizardPage::EditSelectionWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::EditSelectionWizardPage)
{
  ui->setupUi(this);
  ui->m_selectionButtonGroup->setId(ui->m_editInterestRateButton, 0);
  ui->m_selectionButtonGroup->setId(ui->m_editOtherCostButton, 1);
  ui->m_selectionButtonGroup->setId(ui->m_editOtherInfoButton, 2);
  ui->m_selectionButtonGroup->setId(ui->m_editAttributesButton, 3);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("editInterestRateButton", ui->m_editInterestRateButton);
  registerField("editOtherCostButton", ui->m_editOtherCostButton);
  registerField("editOtherInfoButton", ui->m_editOtherInfoButton);
  registerField("editAttributesButton", ui->m_editAttributesButton);

  ui->m_editInterestRateButton->click();
}

EditSelectionWizardPage::~EditSelectionWizardPage()
{
  delete ui;
}
