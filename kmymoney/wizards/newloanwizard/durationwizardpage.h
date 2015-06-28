/***************************************************************************
                         durationwizardpage  -  description
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

#ifndef DURATIONWIZARDPAGE_H
#define DURATIONWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_durationwizardpagedecl.h"

/**
 * This class implements the Duration page of the
 * @ref KNewLoanWizard.
 */
class DurationWizardPageDecl : public QWizardPage, public Ui::DurationWizardPageDecl
{
public:
  DurationWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class DurationWizardPage : public DurationWizardPageDecl
{
  Q_OBJECT
public:
  explicit DurationWizardPage(QWidget *parent = 0);

  QString updateTermWidgets(const double val);
  int term() const;

public slots:
  void resetCalculator();

};

#endif
