/***************************************************************************
                         checkingstatementinfowizardpage.h  -  description
                            -------------------
   begin                : Sun Jul 18 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef CHECKINGSTATEMENTINFOWIZARDPAGE_H
#define CHECKINGSTATEMENTINFOWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class CheckingStatementInfoWizardPage; }

/**
 * This class implements the CheckingStatementInfo page of the
 * @ref KEndingBalanceDlg wizard.
 */

class CheckingStatementInfoWizardPage : public QWizardPage
{
  Q_OBJECT
  Q_DISABLE_COPY(CheckingStatementInfoWizardPage)

public:
  explicit CheckingStatementInfoWizardPage(QWidget *parent = nullptr);
  ~CheckingStatementInfoWizardPage();

  Ui::CheckingStatementInfoWizardPage *ui;
};

#endif
