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
 *   SPDX-License-Identifier: GPL-2.0-or-later
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
