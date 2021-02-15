/***************************************************************************
                         loanattributeswizardpage  -  description
                            -------------------
   begin                : Mon Dec 30 2013
   copyright            : (C) 2013 by Jeremy Whiting
   email                : jpwhiting@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef LOANATTRIBUTESWIZARDPAGE_H
#define LOANATTRIBUTESWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class LoanAttributesWizardPage; }

/**
 * This class implements the Loan Attributes page of the
 * @ref KNewLoanWizard.
 */

class LoanAttributesWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit LoanAttributesWizardPage(QWidget *parent = nullptr);
  ~LoanAttributesWizardPage();

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const final override;

  /**
   * Overload the initializePage function to set widgets based on
   * the inputs from previous pages.
   */
  void initializePage() final override;

  /**
   * Set the institution combobox to the name given
   */
  void setInstitution(const QString &institutionName);

  Ui::LoanAttributesWizardPage *ui;

protected Q_SLOTS:
  void slotNewClicked();
};

#endif
