/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
 * SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
 * (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
 */

#ifndef ACCOUNTSETTINGS_H
#define ACCOUNTSETTINGS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;
class MyMoneyKeyValueContainer;

class AccountSettingsPrivate;
class AccountSettings: public QWidget
{
  Q_OBJECT

public:
  explicit AccountSettings(const MyMoneyAccount& acc, QWidget* parent);
  ~AccountSettings();

  void loadUi(const MyMoneyKeyValueContainer& kvp);

  void loadKvp(MyMoneyKeyValueContainer& kvp);
private:

  Q_DECLARE_PRIVATE(AccountSettings)
  AccountSettingsPrivate * const d_ptr;
};


#endif
