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

#include "ui_interestchargecheckingswizardpagedecl.h"

/**
 * This class implements the InterestChargeCheckings page of the
 * @ref KEndingBalanceDlg wizard.
 */
class InterestChargeCheckingsWizardPageDecl : public QWizardPage, public Ui::InterestChargeCheckingsWizardPageDecl
{
public:
  InterestChargeCheckingsWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class InterestChargeCheckingsWizardPage : public InterestChargeCheckingsWizardPageDecl
{
  Q_OBJECT
public:
  explicit InterestChargeCheckingsWizardPage(QWidget *parent = 0);

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

};

#endif
