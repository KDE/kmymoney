/***************************************************************************
                             kaccountssview.h
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef KACCOUNTSVIEW_H
#define KACCOUNTSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QListWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneyutils.h>
#include "accountsmodel.h"

#include "ui_kaccountsviewdecl.h"

class KMyMoneyApp;
/**
  * This class implements the accounts hierarchical and iconic 'view'.
  */
class KAccountsView : public QWidget, public Ui::KAccountsViewDecl
{
  Q_OBJECT

public:
  explicit KAccountsView(KMyMoneyApp *kmymoney, KMyMoneyView *kmymoneyview);
  virtual ~KAccountsView();

  KRecursiveFilterProxyModel    *getProxyModel();
  QList<AccountsModel::Columns> *getProxyColumns();
  bool                          isLoaded();

public slots:
  void slotLoadAccounts();

protected:
  void loadAccounts();

  // for now it contains the implementation from show()
  virtual void showEvent(QShowEvent * event);

  enum accountViewRole {
    reconcileRole = Qt::UserRole + 1
  };

  void loadIconGroups();

protected slots:
  void slotNetWorthChanged(const MyMoneyMoney &);
  void slotExpandCollapse();
  void slotUnusedIncomeExpenseAccountHidden();

private:
  KMyMoneyApp                         *m_kmymoney;
  KMyMoneyView                        *m_kmymoneyview;

  /** Initializes page and sets its load status to initialized
   */
  void init();

  bool                                m_haveUnusedCategories;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload;

  /**
    * This member holds the load state of page
    */
  bool                                m_needLoad;

  AccountsViewFilterProxyModel        *m_filterProxyModel;
};

#endif
