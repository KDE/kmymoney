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

#ifndef WEBOOB_MAPACCOUNT_HPP
#define WEBOOB_MAPACCOUNT_HPP

#include <memory>

#include <QWizard>

#include "../weboob.h"
#include "ui_mapaccount.h"

class WbMapAccountDialog : public QWizard, public Ui::WbMapAccountDialog
{
  Q_OBJECT
public:

  Weboob *weboob;
  explicit WbMapAccountDialog(QWidget *parent = 0);
  virtual ~WbMapAccountDialog();

protected Q_SLOTS:
  void checkNextButton(void);
  void newPage(int id);
  void gotAccounts();
  void gotBackends();

protected:
  bool finishAccountPage(void);
  bool finishLoginPage(void);
  bool finishFiPage(void);

private:

  enum {
    BACKENDS_PAGE = 0,
    ACCOUNTS_PAGE
  };

  struct Private;
  /// \internal d-pointer instance.
  const std::unique_ptr<Private> d;
  const std::unique_ptr<Private> d2;
};


#endif /* WEBOOB_MAPACCOUNT_HPP */
