/***************************************************************************
                         konlineupdatewizardpage  -  description
                            -------------------
   begin                : Sun Jun 27 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
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

#include <QString>
#include <QWizardPage>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_konlineupdatewizardpagedecl.h"

class MyMoneySecurity;

/**
 * This class implements the Online Update page of the
 * @ref KNewInvestmentWizard.
 */
class KOnlineUpdateWizardPageDecl : public QWizardPage, public Ui::KOnlineUpdateWizardPageDecl
{
public:
  KOnlineUpdateWizardPageDecl(QWidget *parent) : QWizardPage(parent) {
    setupUi(this);
  }
};

class KOnlineUpdateWizardPage : public KOnlineUpdateWizardPageDecl
{
  Q_OBJECT
public:
  explicit KOnlineUpdateWizardPage(QWidget *parent = 0);

  /**
   * Overload the isComplete function to control the Next button
   */
  bool isComplete() const;
  void init2(const MyMoneySecurity& security);

  /**
   * Return whether the m_onlineFactor control is enabled
   */
  bool isOnlineFactorEnabled() const;

public slots:
  void slotCheckPage(const QString&);
  void slotSourceChanged(bool);
};

#endif
