/***************************************************************************
                         newintrowizardpage  -  description
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

#ifndef NEWINTROWIZARDPAGE_H
#define NEWINTROWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class NewIntroWizardPage; }

/**
 * This class implements the New Intro page of the
 * @ref KNewLoanWizard.
 */

class NewIntroWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit NewIntroWizardPage(QWidget *parent = nullptr);
  ~NewIntroWizardPage();

private:
  Ui::NewIntroWizardPage *ui;
};

#endif
