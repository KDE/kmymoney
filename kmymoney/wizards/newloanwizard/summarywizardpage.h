/***************************************************************************
                         summarywizardpage  -  description
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

#ifndef SUMMARYWIZARDPAGE_H
#define SUMMARYWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class SummaryWizardPage; }

/**
 * This class implements the Summary page of the
 * @ref KNewLoanWizard.
 */

class SummaryWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit SummaryWizardPage(QWidget *parent = nullptr);
  ~SummaryWizardPage();

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage();

private:
  Ui::SummaryWizardPage *ui;
};

#endif
