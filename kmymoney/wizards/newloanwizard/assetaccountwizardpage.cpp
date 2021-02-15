/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "assetaccountwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include "ui_assetaccountwizardpage.h"

#include <KLocalizedString>
#include <KGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"
#include "mymoneyaccount.h"
#include "wizards/newaccountwizard/knewaccountwizard.h"
#include "icons/icons.h"

using namespace Icons;

AssetAccountWizardPage::AssetAccountWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::AssetAccountWizardPage)
{
  ui->setupUi(this);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("dontCreatePayoutCheckBox", ui->m_dontCreatePayoutCheckBox);
  registerField("paymentDate", ui->m_paymentDate, "date");
  registerField("assetAccountEdit", ui->m_assetAccountEdit, "selectedItems");

  connect(ui->m_assetAccountEdit,  &KMyMoneySelector::stateChanged, this, &QWizardPage::completeChanged);
  connect(ui->m_dontCreatePayoutCheckBox,  &QAbstractButton::clicked, this, &QWizardPage::completeChanged);

  // load button icons
  KGuiItem createAssetButtonItem(i18n("&Create..."),
                                 Icons::get(Icon::DocumentNew),
                                 i18n("Create a new asset account"),
                                 i18n("Use this to create a new account to which the initial payment should be made"));
  KGuiItem::assign(ui->m_createNewAssetButton, createAssetButtonItem);

  connect(ui->m_createNewAssetButton, &QAbstractButton::clicked, this, &AssetAccountWizardPage::slotAccountNew);

  ui->m_assetAccountEdit->removeButtons();
  ui->m_dontCreatePayoutCheckBox->setChecked(false);
}

AssetAccountWizardPage::~AssetAccountWizardPage()
{
  delete ui;
}

/**
 * Update the "Next" button
 */
bool AssetAccountWizardPage::isComplete() const
{
  if (ui->m_dontCreatePayoutCheckBox->isChecked()) {
    ui->m_assetAccountEdit->setEnabled(false);
    ui->m_paymentDate->setEnabled(false);
    ui->m_createNewAssetButton->setEnabled(false);
    return true;
  } else {
    ui->m_assetAccountEdit->setEnabled(true);
    ui->m_paymentDate->setEnabled(true);
    ui->m_createNewAssetButton->setEnabled(true);
    if (!ui->m_assetAccountEdit->selectedItems().isEmpty()
        && ui->m_paymentDate->date().isValid())
      return true;
  }
  return false;
}

void AssetAccountWizardPage::slotAccountNew()
{
  MyMoneyAccount account;
  account.setOpeningDate(KMyMoneySettings::firstFiscalDate());
  NewAccountWizard::Wizard::newAccount(account);
}
