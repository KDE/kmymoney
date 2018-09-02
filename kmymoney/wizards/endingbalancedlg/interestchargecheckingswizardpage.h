/***************************************************************************
                         interestchargecheckingswizardpage.h  -  description
                            -------------------
   begin                : Sun Jul 18 2010
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

#ifndef INTERESTCHARGECHECKINGSWIZARDPAGE_H
#define INTERESTCHARGECHECKINGSWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class InterestChargeCheckingsWizardPage; }

/**
 * This class implements the InterestChargeCheckings page of the
 * @ref KEndingBalanceDlg wizard.
 */

class InterestChargeCheckingsWizardPage : public QWizardPage
{
  Q_OBJECT
  Q_DISABLE_COPY(InterestChargeCheckingsWizardPage)

public:
  explicit InterestChargeCheckingsWizardPage(QWidget *parent = nullptr);
  ~InterestChargeCheckingsWizardPage() override;

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const final override;

  Ui::InterestChargeCheckingsWizardPage *ui;
};

#endif
