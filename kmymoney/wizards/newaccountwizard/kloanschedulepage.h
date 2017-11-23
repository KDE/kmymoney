/***************************************************************************
                             kloanschedulepage.h
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

#ifndef KLOANSCHEDULEPAGE_H
#define KLOANSCHEDULEPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class QDate;

namespace NewAccountWizard
{
  class Wizard;

  class LoanSchedulePagePrivate;
  class LoanSchedulePage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(LoanSchedulePage)

  public:
    explicit LoanSchedulePage(Wizard* parent);
    ~LoanSchedulePage() override;

    void enterPage();

    KMyMoneyWizardPage* nextPage() const;

    /**
   * This method returns the due date of the first payment to be recorded.
   */
    QDate firstPaymentDueDate() const;

    QWidget* initialFocusWidget() const override;

  private slots:
    void slotLoadWidgets();
    void slotCreateCategory(const QString& name, QString& id);

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, LoanSchedulePage)
    friend class Wizard;
    friend class AccountSummaryPage;
  };
} // namespace

#endif
