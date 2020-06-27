/*
 * Copyright 2007-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KCONFIRMMANUALENTERDLG_H
#define KCONFIRMMANUALENTERDLG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySchedule;
class MyMoneyTransaction;

namespace Ui { class KConfirmManualEnterDlg; }

class KMM_BASE_DIALOGS_EXPORT KConfirmManualEnterDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KConfirmManualEnterDlg)

public:
  explicit KConfirmManualEnterDlg(const MyMoneySchedule& schedule, QWidget* parent = nullptr);
  ~KConfirmManualEnterDlg();

  typedef enum {
    UseOriginal = 0,
    ModifyOnce,
    ModifyAlways
  } Action;

  /**
    * setup the dialog for the difference between the original transaction
    * @a to and the transaction to be entered @a tn.
    */
  void loadTransactions(const MyMoneyTransaction& to, const MyMoneyTransaction& tn);

  /**
    * Returns information about what to do with the transaction
    */
  Action action() const;

private:
  Ui::KConfirmManualEnterDlg *ui;
};

#endif // KCONFIRMMANUALENTERDLG_H
