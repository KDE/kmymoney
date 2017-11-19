/***************************************************************************
                             kcategoriespage.h
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

#ifndef KNEWUSERWIZARDCATEGORIESPAGE_H
#define KNEWUSERWIZARDCATEGORIESPAGE_H

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
