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

#include "newpaymentswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_newpaymentswizardpage.h"

NewPaymentsWizardPage::NewPaymentsWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::NewPaymentsWizardPage)
{
  ui->setupUi(this);
}

NewPaymentsWizardPage::~NewPaymentsWizardPage()
{
  delete ui;
}
