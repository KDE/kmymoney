/***************************************************************************
                             kpreferencepage.h
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

#ifndef KPREFERENCEPAGE_H
#define KPREFERENCEPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class KMyMoneyWizardPage;

namespace NewUserWizard
{
  class Wizard;

  /**
  * Wizard page to allow changing the preferences during setup
  *
  * @author Thomas Baumgart
  */
  class PreferencePagePrivate;
  class PreferencePage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(PreferencePage)

  public:
    explicit PreferencePage(Wizard* parent);
    ~PreferencePage() override;

    KMyMoneyWizardPage* nextPage() const override;

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, PreferencePage)
    friend class Wizard;
  };
} // namespace

#endif
