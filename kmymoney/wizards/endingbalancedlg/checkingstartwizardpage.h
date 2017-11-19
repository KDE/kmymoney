/***************************************************************************
                         checkingstartwizardpage.h  -  description
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

#ifndef CHECKINGSTARTWIZARDPAGE_H
#define CHECKINGSTARTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class CheckingStartWizardPage; }

/**
 * This class implements the CheckingStart page of the
 * @ref KEndingBalanceDlg wizard.
 */

class CheckingStartWizardPage : public QWizardPage
{
  Q_OBJECT
  Q_DISABLE_COPY(CheckingStartWizardPage)

public:
  explicit CheckingStartWizardPage(QWidget *parent = nullptr);
  ~CheckingStartWizardPage();

private:
  Ui::CheckingStartWizardPage *ui;
};

#endif
