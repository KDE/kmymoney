/***************************************************************************
                         newintrowizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
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
