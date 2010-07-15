/***************************************************************************
                         namewizardpage  -  description
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

#ifndef NAMEWIZARDPAGE_H
#define NAMEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_namewizardpagedecl.h"

/**
 * This class implements the Name page of the
 * @ref KNewLoanWizard.
 */
class NameWizardPageDecl : public QWizardPage, public Ui::NameWizardPageDecl
{
public:
  NameWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class NameWizardPage : public NameWizardPageDecl
{
  Q_OBJECT
public:
  explicit NameWizardPage(QWidget *parent = 0);

  /**
   * Some things on this page depend on previous pages for correct
   * initialization, so overload initializePage()
   */
  void initializePage();

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

};

#endif
