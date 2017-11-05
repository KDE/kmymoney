/***************************************************************************
                         interesteditwizardpage  -  description
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

#ifndef INTERESTEDITWIZARDPAGE_H
#define INTERESTEDITWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class InterestEditWizardPage; }

/**
 * This class implements the Interest Edit page of the
 * @ref KNewLoanWizard.
 */

class InterestEditWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit InterestEditWizardPage(QWidget *parent = nullptr);
  ~InterestEditWizardPage();

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

  Ui::InterestEditWizardPage *ui;
};

#endif
