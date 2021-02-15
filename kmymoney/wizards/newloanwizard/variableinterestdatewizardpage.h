/***************************************************************************
                         variableinterestdatewizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef VARIABLEINTERESTDATEWIZARDPAGE_H
#define VARIABLEINTERESTDATEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class VariableInterestDateWizardPage; }

/**
 * This class implements the Variable Interest Date page of the
 * @ref KNewLoanWizard.
 */

class VariableInterestDateWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit VariableInterestDateWizardPage(QWidget *parent = nullptr);
  ~VariableInterestDateWizardPage();

private:
  Ui::VariableInterestDateWizardPage *ui;
};

#endif
