/***************************************************************************
                         kinvestmentdetailswizardpage  -  description
                            -------------------
   begin                : Sun Jun 27 2010
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

#ifndef KINVESTMENTDETAILSWIZARDPAGE_H
#define KINVESTMENTDETAILSWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinvestmentdetailswizardpagedecl.h"

class MyMoneySecurity;

/**
 * This class implements the investment details page  of the
 * @ref KNewInvestmentWizard.
 */
class KInvestmentDetailsWizardPageDecl : public QWizardPage, public Ui::KInvestmentDetailsWizardPageDecl
{
public:
  KInvestmentDetailsWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class KInvestmentDetailsWizardPage : public KInvestmentDetailsWizardPageDecl
{
  Q_OBJECT
public:
  explicit KInvestmentDetailsWizardPage(QWidget *parent = 0);
  void init2(const MyMoneySecurity& security);

  /**
   * Overload isComplete to handle the required fields
   */
  bool isComplete() const;

  /**
   * Functions to control or read the m_priceMode widget
   */
  int priceMode() const;
  void setCurrentPriceMode(int mode);
  void setPriceModeEnabled(bool enabled);

  /**
   * load or set the name of the m_investmentName item widget. The difference
   * can be seen in the @ref kMyMoneyLineEdit type.
   */
  void loadName(const QString& name);
  void setName(const QString& name);

  void setupInvestmentSymbol();

signals:
  void checkForExistingSymbol(const QString& symbol);

};

#endif
