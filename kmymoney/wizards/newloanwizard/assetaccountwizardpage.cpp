/***************************************************************************
                         assetaccountwizardpage  -  description
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

#include "assetaccountwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney.h"
#include "icons/icons.h"

using namespace Icons;

AssetAccountWizardPage::AssetAccountWizardPage(QWidget *parent)
    : AssetAccountWizardPageDecl(parent)
{
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("dontCreatePayoutCheckBox", m_dontCreatePayoutCheckBox);
  registerField("paymentDate", m_paymentDate, "date");
  registerField("assetAccountEdit", m_assetAccountEdit, "selectedItems");

  connect(m_assetAccountEdit,  SIGNAL(stateChanged()), this, SIGNAL(completeChanged()));
  connect(m_dontCreatePayoutCheckBox,  SIGNAL(clicked()), this, SIGNAL(completeChanged()));

  // load button icons
  KGuiItem createAssetButtonItem(i18n("&Create..."),
                                 QIcon::fromTheme(g_Icons[Icon::DocumentNew]),
                                 i18n("Create a new asset account"),
                                 i18n("Use this to create a new account to which the initial payment should be made"));
  KGuiItem::assign(m_createNewAssetButton, createAssetButtonItem);
  connect(m_createNewAssetButton, SIGNAL(clicked()), kmymoney, SLOT(slotAccountNew()));

  m_assetAccountEdit->removeButtons();
  m_dontCreatePayoutCheckBox->setChecked(false);

}

/**
 * Update the "Next" button
 */
bool AssetAccountWizardPage::isComplete() const
{
  if (m_dontCreatePayoutCheckBox->isChecked()) {
    m_assetAccountEdit->setEnabled(false);
    m_paymentDate->setEnabled(false);
    m_createNewAssetButton->setEnabled(false);
    return true;
  } else {
    m_assetAccountEdit->setEnabled(true);
    m_paymentDate->setEnabled(true);
    m_createNewAssetButton->setEnabled(true);
    if (!m_assetAccountEdit->selectedItems().isEmpty()
        && m_paymentDate->date().isValid())
      return true;
  }
  return false;
}
