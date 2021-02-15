/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PREVIOUSPOSTPONEWIZARDPAGE_H
#define PREVIOUSPOSTPONEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class PreviousPostponeWizardPage; }

/**
 * This class implements the PreviousPostpone page of the
 * @ref KEndingBalanceDlg wizard.
 */

class PreviousPostponeWizardPage : public QWizardPage
{
  Q_OBJECT
  Q_DISABLE_COPY(PreviousPostponeWizardPage)

public:
  explicit PreviousPostponeWizardPage(QWidget *parent = nullptr);
  ~PreviousPostponeWizardPage();

  Ui::PreviousPostponeWizardPage *ui;
};

#endif
