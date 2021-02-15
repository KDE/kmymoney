/***************************************************************************
                         knewinvestmentwizard  -  description
                            -------------------
   begin                : Sat Dec 4 2004
   copyright            : (C) 2004 by Thomas Baumgart
   email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                          (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWINVESTMENTWIZARD_H
#define KNEWINVESTMENTWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizard>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;
class MyMoneySecurity;

/**
  * This class contains the implementation of the new investment wizard.
  *
  * @author Thomas Baumgart
  */

class KNewInvestmentWizardPrivate;
class KNewInvestmentWizard : public QWizard
{
  Q_OBJECT
public:
  /**
    * Use this constructor for the creation of a new investment
    */
  explicit KNewInvestmentWizard(QWidget *parent = nullptr);

  /**
    * Use this constructor for the modification of an existing investment
    */
  explicit KNewInvestmentWizard(const MyMoneyAccount& acc, QWidget *parent = nullptr);

  /**
    * Use this constructor for the modification of an existing security
    */
  explicit KNewInvestmentWizard(const MyMoneySecurity& sec, QWidget *parent = nullptr);

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

  /**
    * Create a new investment in a given @p parent investment account
    */
  static void newInvestment(const MyMoneyAccount& parent);
  static void newInvestment(MyMoneyAccount& account, const MyMoneyAccount& parent);

  static void editInvestment(const MyMoneyAccount& parent);

  MyMoneyAccount account() const;

protected Q_SLOTS:
  void slotCheckForExistingSymbol(const QString&);
  void slotHelp();

private:
  Q_DISABLE_COPY(KNewInvestmentWizard)
  Q_DECLARE_PRIVATE(KNewInvestmentWizard)
  const QScopedPointer<KNewInvestmentWizardPrivate> d_ptr;
};

#endif
