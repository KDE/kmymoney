/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ASSETACCOUNTWIZARDPAGE_H
#define ASSETACCOUNTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class AssetAccountWizardPage; }

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */

class AssetAccountWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit AssetAccountWizardPage(QWidget *parent = nullptr);
  ~AssetAccountWizardPage();

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const final override;

  Ui::AssetAccountWizardPage *ui;

public Q_SLOTS:
  void slotAccountNew();
};

#endif
