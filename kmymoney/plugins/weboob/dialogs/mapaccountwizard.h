/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
 * (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef MAPACCOUNTWIZARD_H
#define MAPACCOUNTWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizard>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class WeboobInterface;

class MapAccountWizardPrivate;
class MapAccountWizard : public QWizard
{
  Q_OBJECT

public:
  explicit MapAccountWizard(QWidget *parent, WeboobInterface *weboob);
  ~MapAccountWizard();

  QString currentBackend() const;
  QString currentAccount() const;

private:
  Q_DECLARE_PRIVATE(MapAccountWizard)
  MapAccountWizardPrivate * const d_ptr;

private Q_SLOTS:
  void slotCheckNextButton(void);
  void slotNewPage(int id);
  void slotGotAccounts();
  void slotGotBackends();
};

#endif
