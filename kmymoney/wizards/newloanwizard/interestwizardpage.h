/***************************************************************************
                         interestwizardpage  -  description
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

#ifndef INTERESTWIZARDPAGE_H
#define INTERESTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class InterestWizardPage; }

/**
 * This class implements the Interest page of the
 * @ref KNewLoanWizard.
 */

class InterestWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit InterestWizardPage(QWidget *parent = nullptr);
  ~InterestWizardPage() override;
  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage() final override;

  Ui::InterestWizardPage *ui;

public Q_SLOTS:
  void resetCalculator();
};

#endif
