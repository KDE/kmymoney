/***************************************************************************
                         summaryeditwizardpage  -  description
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

#ifndef SUMMARYEDITWIZARDPAGE_H
#define SUMMARYEDITWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class SummaryEditWizardPage; }

/**
 * This class implements the Summary Edit page of the
 * @ref KNewLoanWizard.
 */

class SummaryEditWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit SummaryEditWizardPage(QWidget *parent = nullptr);
  ~SummaryEditWizardPage() override;

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage() final override;

private:
  Ui::SummaryEditWizardPage *ui;
};

#endif
