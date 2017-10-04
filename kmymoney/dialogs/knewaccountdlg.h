/***************************************************************************
                          knewaccountdlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>

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

#include <QResizeEvent>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneymoney.h>
#include <kmymoneyedit.h>
#include "accountsproxymodel.h"

#include "ui_knewaccountdlgdecl.h"

namespace reports
{
}

class HierarchyFilterProxyModel : public AccountsProxyModel
{
  Q_OBJECT

public:
  HierarchyFilterProxyModel(QObject *parent = 0);

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  void setCurrentAccountId(const QString &selectedAccountId);
  QModelIndex getSelectedParentAccountIndex() const;

protected:
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;

private:
  QString m_currentAccountId;
};

/**
  * This dialog lets you create/edit an account.
  */

class KNewAccountDlgDecl : public QDialog, public Ui::kNewAccountDlgDecl
{
public:
  KNewAccountDlgDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};

class KNewAccountDlg : public KNewAccountDlgDecl
{
  Q_OBJECT

private:
  MyMoneyAccount m_account;
  MyMoneyAccount m_parentAccount;
  HierarchyFilterProxyModel *m_filterProxyModel;

  bool m_categoryEditor;
  bool m_isEditing;

  void loadVatAccounts();
  void storeKVP(const QString& key, kMyMoneyEdit* widget);
  void storeKVP(const QString& key, KLineEdit* widget);
  void storeKVP(const QString& key, const QString& text, const QString& value);
  void storeKVP(const QString& key, QCheckBox* widget);
  void loadKVP(const QString& key, kMyMoneyEdit* widget);
  void loadKVP(const QString& key, KLineEdit* widget);

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
  KNewAccountDlg(const MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent = 0, const QString& title = QString());

  /**
    * This method returns the edited account object.
    */
  const MyMoneyAccount& account();

  /**
    * This method returns the parent account of the edited account object.
    */
  const MyMoneyAccount& parentAccount();

  void setOpeningBalance(const MyMoneyMoney& balance);

  const MyMoneyMoney openingBalance() const {
    return m_openingBalanceEdit->value();
  };

  void setOpeningBalanceShown(bool shown);
  void setOpeningDateShown(bool shown);

  /**
   * This method adds an additional tab pointed to with @a w to the tab widget.
   * This tab is usually defined by a plugin (eg. online banking). If @a w is
   * zero, this is a NOP. @a name is used as the text to be placed on the tab.
   */
  void addTab(QWidget* w, const QString& name);

protected:
  void displayOnlineBankingStatus();
  void adjustEditWidgets(kMyMoneyEdit* dst, kMyMoneyEdit* src, char mode, int corr);
  void handleOpeningBalanceCheckbox(const QString &currencyId);

protected slots:
  void okClicked();
  void slotSelectionChanged(const QItemSelection &current, const QItemSelection &previous);
  void slotAccountTypeChanged(const QString& type);
  void slotVatChanged(bool);
  void slotVatAssignmentChanged(bool);
  void slotNewClicked();
  void slotCheckFinished();
  void slotLoadInstitutions(const QString&);
  void slotAdjustMinBalanceAbsoluteEdit(const QString&);
  void slotAdjustMinBalanceEarlyEdit(const QString&);
  void slotAdjustMaxCreditAbsoluteEdit(const QString&);
  void slotAdjustMaxCreditEarlyEdit(const QString&);
  void slotCheckCurrency();
};

#endif

