/***************************************************************************
                             knewuserwizard.h
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
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

#ifndef KNEWUSERWIZARD_H
#define KNEWUSERWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
class QString;

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneywizard.h>
#include <mymoneysecurity.h>
class MyMoneyPayee;
class MyMoneyInstitution;
class MyMoneyAccount;
class MyMoneyMoney;
class MyMoneyTemplate;

/**
  * @author Thomas Baumgart
  */
namespace NewUserWizard {

class IntroPage;
class GeneralPage;
class CurrencyPage;
class AccountPage;
class CategoriesPage;
class PreferencePage;
class FilePage;

/**
  * @author Thomas Baumgart
  *
  * This class implements the new user wizard which is used to gather
  * some initial information from the user who creates a new KMyMoney
  * 'file'.
  */
class Wizard : public KMyMoneyWizard
{
  friend class IntroPage;
  friend class GeneralPage;
  friend class CurrencyPage;
  friend class AccountPage;
  friend class CategoriesPage;
  friend class PreferencePage;
  friend class FilePage;

  Q_OBJECT
public:
  explicit Wizard(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags flags = 0);
  /**
    * Returns the personal information of the user (e.g. name, address, etc.)
    */
  MyMoneyPayee user(void) const;

  /**
    * Returns the URL that the user has chosen to store the file
    */
  QString url(void) const;

  /**
    * Returns the information about an institution if entered by
    * the user. If the name field is empty, then he did not enter
    * such information.
    */
  MyMoneyInstitution institution(void) const;

  /**
    * Returns the information about a checking account if entered by
    * the user. If the name field is empty, then he did not enter
    * such information.
    */
  MyMoneyAccount account(void) const;

  /**
    * Returns the opening balance value provided by the user. not enter
    */
  MyMoneyMoney openingBalance(void) const;

  /**
    * Returns the security to be used as base currency.
    */
  MyMoneySecurity baseCurrency(void) const;

  /**
    * Returns a list of templates including accounts to be created
    */
  QList<MyMoneyTemplate> templates(void) const;

private:
  MyMoneySecurity   m_baseCurrency;
  IntroPage*        m_introPage;
  GeneralPage*      m_generalPage;
  CurrencyPage*     m_currencyPage;
  AccountPage*      m_accountPage;
  CategoriesPage*   m_categoriesPage;
  PreferencePage*   m_preferencePage;
  FilePage*         m_filePage;
};

} // namespace


#endif
