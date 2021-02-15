/***************************************************************************
                             khierarchypage.h
                             -------------------
    begin                : Tue Sep 25 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KHIERARCHYPAGE_H
#define KHIERARCHYPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class MyMoneyAccount;

namespace NewAccountWizard
{
  class Wizard;
  class HierarchyPagePrivate;

  class HierarchyPage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(HierarchyPage)

  public:
    explicit HierarchyPage(Wizard* parent);
    ~HierarchyPage() override;

    void enterPage() override;
    KMyMoneyWizardPage* nextPage() const override;
    QWidget* initialFocusWidget() const override;
    MyMoneyAccount parentAccount() const;

    bool isComplete() const override;

  protected Q_SLOTS:
    void parentAccountChanged();

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, HierarchyPage)
    friend class Wizard;
    friend class AccountSummaryPage;
  };

} // namespace

#endif
