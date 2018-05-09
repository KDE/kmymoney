/***************************************************************************
                         summaryeditwizardpage  -  description
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
