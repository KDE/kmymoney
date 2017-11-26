/***************************************************************************
                             kcreditcardschedulepage.h
                             -------------------
    begin                : Tue Sep 25 2007
    copyright            : (C) 2007 Thomas Baumgart
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

#ifndef KCREDITCARDSCHEDULE_H
#define KCREDITCARDSCHEDULE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

namespace NewAccountWizard
{
  class Wizard;

  class CreditCardSchedulePagePrivate;
  class CreditCardSchedulePage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(CreditCardSchedulePage)

  public:
    explicit CreditCardSchedulePage(Wizard* parent);
    ~CreditCardSchedulePage() override;

    KMyMoneyWizardPage* nextPage() const override;
    virtual bool isComplete() const override;
    void enterPage() override;

    QWidget* initialFocusWidget() const override;

  private Q_SLOTS:
    void slotLoadWidgets();

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, CreditCardSchedulePage)
    friend class Wizard;
    friend class AccountSummaryPage;
  };

} // namespace

#endif
