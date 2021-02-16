/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINSTITUTION_H
#define KINSTITUTION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class MyMoneyInstitution;

namespace NewAccountWizard
{
  class Wizard;

  class InstitutionPagePrivate;
  class InstitutionPage : public QWidget, public WizardPage<Wizard>
  {
    Q_OBJECT
    Q_DISABLE_COPY(InstitutionPage)

  public:
    explicit InstitutionPage(Wizard* parent);
    ~InstitutionPage() override;
    KMyMoneyWizardPage* nextPage() const override;

    QWidget* initialFocusWidget() const override;

    /**
    * Returns the information about an institution if entered by
    * the user. If the id field is empty, then he did not enter
    * such information.
    */
    const MyMoneyInstitution& institution() const;

    void selectExistingInstitution(const QString& id);

  private Q_SLOTS:
    void slotLoadWidgets();
    void slotNewInstitution();
    void slotSelectInstitution(int index);

  private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, InstitutionPage)
    friend class Wizard;
    friend class AccountSummaryPage;
    friend class BrokeragePage;
  };
} // namespace

#endif
