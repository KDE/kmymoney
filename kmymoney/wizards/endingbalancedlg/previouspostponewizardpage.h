/***************************************************************************
                         previouspostponewizardpage.h  -  description
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

#ifndef PREVIOUSPOSTPONEWIZARDPAGE_H
#define PREVIOUSPOSTPONEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class PreviousPostponeWizardPage; }

/**
 * This class implements the PreviousPostpone page of the
 * @ref KEndingBalanceDlg wizard.
 */

class PreviousPostponeWizardPage : public QWizardPage
{
  Q_OBJECT
  Q_DISABLE_COPY(PreviousPostponeWizardPage)

public:
  explicit PreviousPostponeWizardPage(QWidget *parent = nullptr);
  ~PreviousPostponeWizardPage();

  Ui::PreviousPostponeWizardPage *ui;
};

#endif
