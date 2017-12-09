/***************************************************************************
                          knewaccountdlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
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

#ifndef KNEWACCOUNTDLG_H
#define KNEWACCOUNTDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

class QString;
class QItemSelection;

class MyMoneyMoney;
class MyMoneyAccount;

class KNewAccountDlgPrivate;
class KNewAccountDlg : public QDialog
{  
  Q_OBJECT

public:
  /**
    * This is the constructor of the dialog. The parameters define the environment
    * in which the dialog will be used. Depending on the environment, certain rules
    * apply and will be handled by the dialog.
    *
    * @param account The original data to be used to create the account. In case
    *                of @p isEditing is false, the account id, the parent account id
    *                and the list of all child accounts will be cleared.
    * @param isEditing If @p false, rules for new account creation apply.
    *                  If @p true, rules for account editing apply
    * @param categoryEditor If @p false, rules for asset/liability accounts apply.
    *                       If @p true, rules for income/expense account apply.
    * @param parent Pointer to parent object (passed to QDialog). Default is 0.
    * @param title Caption of the object (passed to QDialog). Default is empty string.
    */
  KNewAccountDlg(const MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent, const QString& title);

  /**
    * This method returns the edited account object.
    */
  MyMoneyAccount account();

  /**
    * This method returns the parent account of the edited account object.
    */
  MyMoneyAccount parentAccount() const;

  MyMoneyMoney openingBalance() const;
  void setOpeningBalance(const MyMoneyMoney& balance);

  void setOpeningBalanceShown(bool shown);
  void setOpeningDateShown(bool shown);

  /**
   * This method adds an additional tab pointed to with @a w to the tab widget.
   * This tab is usually defined by a plugin (eg. online banking). If @a w is
   * zero, this is a NOP. @a name is used as the text to be placed on the tab.
   */
  void addTab(QWidget* w, const QString& name);

  /**
    * Brings up the new category editor and saves the information.
    * The dialog will be preset with the name and parent account.
    *
    * @param account reference of category to be created. The @p name member
    *                should be filled by the caller. The object will be filled
    *                with additional information during the creation process
    *                esp. the @p id member.
    * @param parent reference to parent account (defaults to none)
    */
  static void newCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
   * This method opens the category editor with the data found in @a account. The
   * parent account is preset to @a parent but can be modified. If the user
   * acknowledges, the category is created.
   */
  static void createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);

protected Q_SLOTS:
  void okClicked();
  void slotSelectionChanged(const QItemSelection &current, const QItemSelection &previous);
  void slotAccountTypeChanged(int index);
  void slotVatChanged(bool);
  void slotVatAssignmentChanged(bool);
  void slotNewClicked();
  void slotCheckFinished();
  void slotLoadInstitutions(const QString&);
  void slotAdjustMinBalanceAbsoluteEdit(const QString&);
  void slotAdjustMinBalanceEarlyEdit(const QString&);
  void slotAdjustMaxCreditAbsoluteEdit(const QString&);
  void slotAdjustMaxCreditEarlyEdit(const QString&);
  void slotCheckCurrency(int index);

private:
  Q_DISABLE_COPY(KNewAccountDlg)
  Q_DECLARE_PRIVATE(KNewAccountDlg)
  KNewAccountDlgPrivate* d_ptr;
};

#endif

