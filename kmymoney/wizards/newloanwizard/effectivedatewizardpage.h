/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EFFECTIVEDATEWIZARDPAGE_H
#define EFFECTIVEDATEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class EffectiveDateWizardPage; }

/**
 * This class implements the Effective Date page of the
 * @ref KNewLoanWizard.
 */

class EffectiveDateWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit EffectiveDateWizardPage(QWidget *parent = nullptr);
  ~EffectiveDateWizardPage();

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const final override;

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage() final override;

  Ui::EffectiveDateWizardPage *ui;
};

#endif
