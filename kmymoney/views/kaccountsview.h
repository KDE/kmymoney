/***************************************************************************
                             kaccountssview.h
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017, 2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountsviewbase.h"

class MyMoneyMoney;
class MyMoneyAccount;
class MyMoneyObject;

namespace eMenu { enum class Action; }
namespace KMyMoneyPlugin { class OnlinePlugin; }

template <class Key, class Value> class QMap;

/**
  * This class implements the accounts hierarchical and iconic 'view'.
  */

class KAccountsViewPrivate;
class KAccountsView : public KMyMoneyAccountsViewBase
{
  Q_OBJECT

public:
  explicit KAccountsView(QWidget *parent = nullptr);
  ~KAccountsView();

  void executeCustomAction(eView::Action action) override;
  void refresh();
  void updateActions(const MyMoneyObject &obj);

public Q_SLOTS:
  void slotNetWorthChanged(const MyMoneyMoney &);
  void slotShowAccountMenu(const MyMoneyAccount& acc);

  void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent) override;
  void slotSelectByVariant(const QVariantList& variant, eView::Intent intent) override;

protected:
  void showEvent(QShowEvent * event) override;

private:
  Q_DECLARE_PRIVATE(KAccountsView)

private Q_SLOTS:
  void slotUnusedIncomeExpenseAccountHidden();
  void slotNewAccount();
  void slotEditAccount();
  void slotDeleteAccount();
  void slotCloseAccount();
  void slotReopenAccount();
  void slotChartAccountBalance();
  void slotNewCategory();
  void slotNewPayee(const QString& nameBase, QString& id);
  void slotAccountMapOnline();
  void slotAccountUnmapOnline();
  void slotAccountUpdateOnline();
  void slotAccountUpdateOnlineAll();
};

#endif
