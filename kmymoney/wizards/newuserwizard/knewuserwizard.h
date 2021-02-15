/***************************************************************************
                             knewuserwizard.h
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWUSERWIZARD_H
#define KNEWUSERWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneywizard.h"
#include "mymoneysecurity.h"
class MyMoneyPayee;
class MyMoneyInstitution;
class MyMoneyAccount;
class MyMoneyMoney;
class MyMoneyTemplate;

/**
  * @author Thomas Baumgart
  */
namespace NewUserWizard
{
  extern int stepCount;

  /**
  * @author Thomas Baumgart
  *
  * This class implements the new user wizard which is used to gather
  * some initial information from the user who creates a new KMyMoney
  * 'file'.
  */
  class WizardPrivate;
  class Wizard : public KMyMoneyWizard
  {
    friend class IntroPage;
    friend class GeneralPage;
    friend class CurrencyPage;
    friend class AccountPage;
    friend class CategoriesPage;
    friend class PreferencePage;

    Q_OBJECT
    Q_DISABLE_COPY(Wizard)

  public:
    explicit Wizard(QWidget *parent = nullptr, bool modal = false, Qt::WindowFlags flags = 0);
    ~Wizard() override;
    /**
    * Returns the personal information of the user (e.g. name, address, etc.)
    */
    MyMoneyPayee user() const;

    /**
    * Returns the information about an institution if entered by
    * the user. If the name field is empty, then he did not enter
    * such information.
    */
    MyMoneyInstitution institution() const;

    /**
    * Returns the information about a checking account if entered by
    * the user. If the name field is empty, then he did not enter
    * such information.
    */
    MyMoneyAccount account() const;

    /**
    * Returns the opening balance value provided by the user. not enter
    */
    MyMoneyMoney openingBalance() const;

    /**
    * Returns the security to be used as base currency.
    */
    MyMoneySecurity baseCurrency() const;

    /**
    * Returns a list of templates including accounts to be created
    */
    QList<MyMoneyTemplate> templates() const;

    /**
    * True if the settings dialog should be launched after the wizard is finished.
    */
    bool startSettingsAfterFinished() const;

  private:
    Q_DECLARE_PRIVATE(Wizard)
  };

} // namespace


#endif
