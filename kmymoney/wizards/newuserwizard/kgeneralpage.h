/***************************************************************************
                             kgeneralpage.h
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KGENERALPAGE_H
#define KGENERALPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"
#include "userinfo.h"

class KMyMoneyWizardPage;
struct ContactData;

namespace NewUserWizard
{
  class Wizard;
  /**
  * Wizard page collecting information about the user
  *
  * @author Thomas Baumgart
  */
  class GeneralPagePrivate;
  class GeneralPage : public UserInfo, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(GeneralPage)

  public:
    explicit GeneralPage(Wizard* parent);
    ~GeneralPage() override;

    KMyMoneyWizardPage* nextPage() const override;

    void enterPage() override;

  protected Q_SLOTS:
    void slotLoadFromAddressBook();
    void slotContactFetched(const ContactData &identity);

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, GeneralPage)
    friend class Wizard;
  };
} // namespace

#endif
