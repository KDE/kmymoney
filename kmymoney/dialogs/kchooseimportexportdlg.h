/*
 * SPDX-FileCopyrightText: 2001-2002 Michael Edwardes <mte@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2001-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KCHOOSEIMPORTEXPORTDLG_H
#define KCHOOSEIMPORTEXPORTDLG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  *@author Michael Edwardes
  */

class KChooseImportExportDlgPrivate;
class KMM_BASE_DIALOGS_EXPORT KChooseImportExportDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KChooseImportExportDlg)

public:
  explicit KChooseImportExportDlg(int type, QWidget *parent = nullptr);
  ~KChooseImportExportDlg();
  QString importExportType() const;

protected Q_SLOTS:
  void slotTypeActivated(const QString& text);

private:
  KChooseImportExportDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KChooseImportExportDlg)
};

#endif
