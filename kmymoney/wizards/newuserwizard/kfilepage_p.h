/***************************************************************************
                             kfilepage_p.h
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
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

#ifndef KFILEPAGE_P_H
#define KFILEPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kfilepage.h"
#include "wizardpage_p.h"

namespace NewUserWizard
{
  class Wizard;

  class FilePagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(FilePagePrivate)

  public:
    FilePagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KFilePage)
    {
    }

    ~FilePagePrivate()
    {
      delete ui;
    }

    Ui::KFilePage *ui;
  };
}
#endif
