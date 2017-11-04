/***************************************************************************
                         kinvestmenttypewizardpage  -  description
                            -------------------
   begin                : Sun Jun 27 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
                          (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KINVESTMENTTYPEWIZARDPAGE_H
#define KINVESTMENTTYPEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySecurity;

namespace Ui { class KInvestmentTypeWizardPage; }

/**
 * This class implements the investment type page of the
 * @ref KNewInvestmentWizard.
 */
class KInvestmentTypeWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit KInvestmentTypeWizardPage(QWidget *parent = nullptr);
  ~KInvestmentTypeWizardPage();

  void init2(const MyMoneySecurity& security);
  void setIntroLabelText(const QString& text);

private:
  Ui::KInvestmentTypeWizardPage  *ui;
};

#endif
