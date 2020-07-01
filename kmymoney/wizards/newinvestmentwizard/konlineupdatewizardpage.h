/***************************************************************************
                         konlineupdatewizardpage  -  description
                            -------------------
   begin                : Sun Jun 27 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
                          (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KONLINEUPDATEWIZARDPAGE_H
#define KONLINEUPDATEWIZARDPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySecurity;

namespace Ui { class KOnlineUpdateWizardPage; }

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */
class KOnlineUpdateWizardPage : public QWizardPage
{
  Q_OBJECT
public:
  explicit KOnlineUpdateWizardPage(QWidget *parent = nullptr);
  ~KOnlineUpdateWizardPage();

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const final override;
  void init2(const MyMoneySecurity& security);

  /**
   * Return whether the m_onlineFactor control is enabled
   */
  bool isOnlineFactorEnabled() const;

public Q_SLOTS:
  void slotCheckPage(const QString&);

private:
  Ui::KOnlineUpdateWizardPage  *ui;
};

#endif
