/*
 * SPDX-FileCopyrightText: 2007-2011 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
