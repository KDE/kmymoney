/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WEBOOB_WEBACCOUNTSETTINGS_H
#define WEBOOB_WEBACCOUNTSETTINGS_H

#include <QWidget>

class MyMoneyAccount;
class MyMoneyKeyValueContainer;

class WebAccountSettings: public QWidget
{
public:
  WebAccountSettings(const MyMoneyAccount& acc, QWidget* parent);
  ~WebAccountSettings();

  void loadUi(const MyMoneyKeyValueContainer& kvp);

  void loadKvp(MyMoneyKeyValueContainer& kvp);
private:
  /// \internal d-pointer class.
  struct Private;
  /// \internal d-pointer instance.
  Private* const d;
};


#endif /* WEBOOB_WEBACCOUNTSETTINGS_H */
