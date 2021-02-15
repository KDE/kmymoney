/*
 * SPDX-FileCopyrightText: 2007-2019 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KACCOUNTSVIEW_H
#define KACCOUNTSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class MyMoneyMoney;
class MyMoneyAccount;
class MyMoneyObject;
class SelectedObjects;

namespace eMenu { enum class Action; }
namespace KMyMoneyPlugin { class OnlinePlugin; }

template <class Key, class Value> class QMap;

/**
  * This class implements the accounts hierarchical and iconic 'view'.
  */

class KAccountsViewPrivate;
class KAccountsView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KAccountsView(QWidget *parent = nullptr);
  ~KAccountsView();

  void executeCustomAction(eView::Action action) override;
  void refresh();

public Q_SLOTS:
  void slotNetWorthChanged(const MyMoneyMoney &netWorth, bool isApproximate);

  void slotSelectByVariant(const QVariantList& variant, eView::Intent intent) override;

  void slotSettingsChanged() override;
  void updateActions(const SelectedObjects& selections) override;

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
