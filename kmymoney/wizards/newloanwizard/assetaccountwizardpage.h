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

namespace Ui { class AssetAccountWizardPage; }

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */

class AssetAccountWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit AssetAccountWizardPage(QWidget *parent = nullptr);
  ~AssetAccountWizardPage();

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;

  Ui::AssetAccountWizardPage *ui;

public Q_SLOTS:
  void slotAccountNew();
};

#endif
