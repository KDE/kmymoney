/***************************************************************************
                         editselectionwizardpage  -  description
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

#ifndef EDITSELECTIONWIZARDPAGE_H
#define EDITSELECTIONWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class EditSelectionWizardPage; }

/**
 * This class implements the Edit Selection page of the
 * @ref KNewLoanWizard.
 */

class EditSelectionWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit EditSelectionWizardPage(QWidget *parent = nullptr);
  ~EditSelectionWizardPage();

  Ui::EditSelectionWizardPage *ui;
};

#endif
