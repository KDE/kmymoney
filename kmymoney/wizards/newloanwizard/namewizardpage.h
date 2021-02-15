/***************************************************************************
                         namewizardpage  -  description
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

#ifndef NAMEWIZARDPAGE_H
#define NAMEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class NameWizardPage; }

/**
 * This class implements the Name page of the
 * @ref KNewLoanWizard.
 */

class NameWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit NameWizardPage(QWidget *parent = nullptr);
  ~NameWizardPage();

  /**
   * Some things on this page depend on previous pages for correct
   * initialization, so overload initializePage()
   */
  void initializePage() final override;

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const final override;

  Ui::NameWizardPage *ui;
};

#endif
