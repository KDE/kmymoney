/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

    // keep button text without alteration by wizard page logic
    m_buttonText.append(ui->m_editInterestRateButton->text());
    m_buttonText.append(ui->m_editOtherCostButton->text());
    m_buttonText.append(ui->m_editOtherInfoButton->text());
    m_buttonText.append(ui->m_editAttributesButton->text());

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

int EditSelectionWizardPage::selectedOption() const
{
    return ui->m_selectionButtonGroup->checkedId();
}

QString EditSelectionWizardPage::selectedOptionText(int option) const
{
    if ((option >= 0) && (option < m_buttonText.count())) {
        return m_buttonText.at(option);
    }
    return {};
}
