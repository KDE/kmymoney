/*
 * SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KNEWINSTITUTIONDLG_H
#define KNEWINSTITUTIONDLG_H

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
class KNewInstitutionDlgPrivate;
class KMM_BASE_DIALOGS_EXPORT KNewInstitutionDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KNewInstitutionDlg)

public:
  explicit KNewInstitutionDlg(MyMoneyInstitution& institution, QWidget *parent = nullptr);
  ~KNewInstitutionDlg();
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
  KNewInstitutionDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KNewInstitutionDlg)
};

#endif
