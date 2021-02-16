/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, PreferencePage)
    friend class Wizard;
  };
} // namespace

#endif
