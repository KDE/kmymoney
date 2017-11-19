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

#ifndef KNEWACCOUNTWIZARDCREDITCARDSCHEDULE_H
#define KNEWACCOUNTWIZARDCREDITCARDSCHEDULE_H

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

    KMyMoneyWizardPage* nextPage() const;
    virtual bool isComplete() const;
    void enterPage();

    QWidget* initialFocusWidget() const override;

  private slots:
    void slotLoadWidgets();

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, CreditCardSchedulePage)
    friend class Wizard;
    friend class AccountSummaryPage;
  };

} // namespace

#endif
