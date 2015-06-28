/***************************************************************************
                         knewinvestmentwizard  -  description
                            -------------------
   begin                : Sat Dec 4 2004
   copyright            : (C) 2004 by Thomas Baumgart
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

#ifndef KNEWINVESTMENTWIZARD_H
#define KNEWINVESTMENTWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewinvestmentwizarddecl.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"

/**
  * This class contains the implementation of the new investment wizard.
  *
  * @author Thomas Baumgart
  */

class KNewInvestmentWizardDecl : public QWizard, public Ui::KNewInvestmentWizardDecl
{
public:
  KNewInvestmentWizardDecl(QWidget *parent) : QWizard(parent) {
    setupUi(this);
  }
};
class KNewInvestmentWizard : public KNewInvestmentWizardDecl
{
  Q_OBJECT
public:
  /**
    * Use this constructor for the creation of a new investment
    */
  explicit KNewInvestmentWizard(QWidget *parent = 0);

  /**
    * Use this constructor for the modification of an existing investment
    */
  explicit KNewInvestmentWizard(const MyMoneyAccount& acc, QWidget *parent = 0);

  /**
    * Use this constructor for the modification of an existing security
    */
  explicit KNewInvestmentWizard(const MyMoneySecurity& sec, QWidget *parent = 0);

  ~KNewInvestmentWizard();

  /**
   * This method sets the name in the name widget.
   */
  void setName(const QString& name);

  /**
    * Depending on the constructor used, this method either
    * creates all necessary objects for the investment or updates
    * them.
    *
    * @param parentId id of parent account for the investment
    */
  void createObjects(const QString& parentId);

  const MyMoneyAccount& account() const {
    return m_account;
  }

protected slots:
  void slotCheckForExistingSymbol(const QString&);
  void slotHelp();

private:
  void init1();
  void init2();

private:
  MyMoneyAccount    m_account;
  MyMoneySecurity   m_security;
  bool              m_createAccount;
};

#endif
