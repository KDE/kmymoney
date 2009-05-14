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
#include <mymoneyaccount.h>
#include <mymoneysecurity.h>

/**
  * This class contains the implementation of the new investment wizard.
  *
  * @author Thomas Baumgart
  */

class KNewInvestmentWizardDecl : public QDialog, public Ui::KNewInvestmentWizardDecl
{
public:
  KNewInvestmentWizardDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};
class KNewInvestmentWizard : public KNewInvestmentWizardDecl
{
  Q_OBJECT
public:
  /**
    * Use this constructor for the creation of a new investment
    */
  KNewInvestmentWizard( QWidget *parent = 0 );

  /**
    * Use this constructor for the modification of an existing investment
    */
  KNewInvestmentWizard( const MyMoneyAccount& acc, QWidget *parent = 0 );

  /**
    * Use this constructor for the modification of an existing security
    */
  KNewInvestmentWizard( const MyMoneySecurity& sec, QWidget *parent = 0, const char *name = 0 );

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

  const MyMoneyAccount& account(void) const { return m_account; }

protected slots:
  void next(void);
  void slotCheckPage(void);
  void slotCheckPage(const QString&);
  void slotCheckForExistingSymbol(const QString&);
  void slotSourceChanged(bool);
  void slotHelp(void);

private:
  void init1(void);
  void init2(void);

private:
  MyMoneyAccount    m_account;
  MyMoneySecurity   m_security;
  bool              m_createAccount;
};

#endif
