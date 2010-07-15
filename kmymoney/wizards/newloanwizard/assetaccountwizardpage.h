/***************************************************************************
                         assetaccountwizardpage  -  description
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

#ifndef ASSETACCOUNTWIZARDPAGE_H
#define ASSETACCOUNTWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_assetaccountwizardpagedecl.h"

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */
class AssetAccountWizardPageDecl : public QWizardPage, public Ui::AssetAccountWizardPageDecl
{
public:
  AssetAccountWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class AssetAccountWizardPage : public AssetAccountWizardPageDecl
{
  Q_OBJECT
public:
  explicit AssetAccountWizardPage(QWidget *parent = 0);

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;
};

#endif
