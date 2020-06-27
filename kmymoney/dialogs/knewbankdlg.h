/*
 * Copyright 2000-2002  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KNEWBANKDLG_H
#define KNEWBANKDLG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyInstitution;
class KJob;

/// This dialog lets the user create or edit an institution
class KNewBankDlgPrivate;
class KMM_BASE_DIALOGS_EXPORT KNewBankDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KNewBankDlg)

public:
  explicit KNewBankDlg(MyMoneyInstitution& institution, QWidget *parent = nullptr);
  ~KNewBankDlg();
  const MyMoneyInstitution& institution();

  static void newInstitution(MyMoneyInstitution& institution);

private Q_SLOTS:
  void okClicked();
  void institutionNameChanged(const QString &);
  void slotUrlChanged(const QString&);
  void slotLoadIcon();
  void slotIconLoaded(KJob* job);
  void killIconLoad();

private:
  KNewBankDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KNewBankDlg)
};

#endif
