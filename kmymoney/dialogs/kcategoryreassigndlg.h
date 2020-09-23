/*
 * Copyright 2007-2008  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KCATEGORYREASSIGNDLG_H
#define KCATEGORYREASSIGNDLG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;
namespace Ui { class KCategoryReassignDlg; }

/**
 *  Implementation of the dialog that lets the user select a payee in order
 *  to re-assign transactions (for instance, if payees are deleted).
 */

class KMM_BASE_DIALOGS_EXPORT KCategoryReassignDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KCategoryReassignDlg)

public:
  explicit KCategoryReassignDlg(QWidget* parent = nullptr);
  ~KCategoryReassignDlg();

  /**
    * This function sets up the dialog, lets the user select a category and returns
    * the id of the selected category in the list of all known income and expense accounts.
    *
    * @param category reference to MyMoneyAccount object of the category to be deleted
    *
    * @return Returns the id of the selected category in the list or QString() if
    *         the dialog was aborted. QString() is also returned if the @a category
    *         does not have an id.
    */
  QString show(const MyMoneyAccount& category);

protected:
  void accept() final override;

private:
  Ui::KCategoryReassignDlg *ui;
};

#endif // KCATEGORYREASSIGNDLG_H
