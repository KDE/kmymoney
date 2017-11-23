/***************************************************************************
                             kfilepage.h
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KFILEPAGE_H
#define KFILEPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

namespace NewUserWizard
{
  class Wizard;
  /**
  * Wizard page to allow selecting the filename
  *
  * @author Thomas Baumgart
  */
  class FilePagePrivate;
  class FilePage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(FilePage)

  public:
    explicit FilePage(Wizard* parent);
    ~FilePage() override;

    bool isComplete() const override;

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, FilePage)
    friend class Wizard;
  };

} // namespace

#endif
