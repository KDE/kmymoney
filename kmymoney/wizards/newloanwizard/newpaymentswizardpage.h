/***************************************************************************
                         newpaymentswizardpage  -  description
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

#ifndef NEWPAYMENTSWIZARDPAGE_H
#define NEWPAYMENTSWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class NewPaymentsWizardPage; }

/**
 * This class implements the New Payments page of the
 * @ref KNewLoanWizard.
 */

class NewPaymentsWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit NewPaymentsWizardPage(QWidget *parent = nullptr);
  ~NewPaymentsWizardPage();

private:
  Ui::NewPaymentsWizardPage *ui;
};

#endif
