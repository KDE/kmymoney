/***************************************************************************
                             kinstitutionpage.cpp
                             -------------------
    begin                : Tue Sep 25 2006
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef KINSTITUTION_P_H
#define KINSTITUTION_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinstitutionpage.h"

#include "wizardpage_p.h"
#include "mymoneyinstitution.h"

namespace NewAccountWizard
{
  class Wizard;

  class InstitutionPagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(InstitutionPagePrivate)

  public:
    explicit InstitutionPagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KInstitutionPage)
    {
    }

    ~InstitutionPagePrivate()
    {
      delete ui;
    }

    Ui::KInstitutionPage      *ui;
    QList<MyMoneyInstitution>  m_list;
  };
}

#endif
