/***************************************************************************
                         editintrowizardpage  -  description
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

#ifndef EDITINTROWIZARDPAGE_H
#define EDITINTROWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class EditIntroWizardPage; }

/**
 * This class implements the Edit Intro page of the
 * @ref KNewLoanWizard.
 */

class EditIntroWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit EditIntroWizardPage(QWidget *parent = nullptr);
  ~EditIntroWizardPage();

private:
  Ui::EditIntroWizardPage *ui;
};

#endif
