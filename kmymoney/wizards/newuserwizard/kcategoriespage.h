/***************************************************************************
                             kcategoriespage.h
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

#ifndef KCATEGORIESPAGE_H
#define KCATEGORIESPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"
#include "accounts.h"

class KMyMoneyWizardPage;
class MyMoneyTemplate;

template <typename T> class QList;

namespace NewUserWizard
{
  class Wizard;
  /**
  * Wizard page collecting information about the account templates.
  *
  * @author Thomas Baumgart
  */
  class CategoriesPagePrivate;
  class CategoriesPage : public Accounts, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(CategoriesPage)

  public:
    explicit CategoriesPage(Wizard* parent);
    ~CategoriesPage() override;

    KMyMoneyWizardPage* nextPage() const override;
    QList<MyMoneyTemplate> selectedTemplates() const;

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, CategoriesPage)
    friend class Wizard;
  };
} // namespace

#endif
