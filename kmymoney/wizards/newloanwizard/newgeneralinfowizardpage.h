/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NEWGENERALINFOWIZARDPAGE_H
#define NEWGENERALINFOWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class NewGeneralInfoWizardPage; }

/**
 * This class implements the New General Info page of the
 * @ref KNewLoanWizard.
 */

class NewGeneralInfoWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit NewGeneralInfoWizardPage(QWidget *parent = nullptr);
  ~NewGeneralInfoWizardPage();

private:
  Ui::NewGeneralInfoWizardPage *ui;
};

#endif
