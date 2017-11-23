/***************************************************************************
                             kbrokeragepage.h
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

#ifndef KBROKERAGEPAGE_H
#define KBROKERAGEPAGE_H

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

  class BrokeragePagePrivate;
  class BrokeragePage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(BrokeragePage)

  public:
    explicit BrokeragePage(Wizard* parent);
    ~BrokeragePage() override;

    KMyMoneyWizardPage* nextPage() const override;
    void enterPage() override;

    QWidget* initialFocusWidget() const override;

  private slots:
    void slotLoadWidgets();

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, BrokeragePage)
    friend class Wizard;
    friend class AccountSummaryPage;
  };
} // namespace

#endif
