/*
 * Copyright 2002       Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2004  Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2004-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KNEWEQUITYENTRYDLG_H
#define KNEWEQUITYENTRYDLG_H

#include "kmm_base_dialogs_export.h"

#include <QDialog>

/**
  *
  * Dialog to allow user to enter all data for a stock or mutual fund investment type.
  *
  * @author Kevin Tambascio
  *
  */

class KNewEquityEntryDlgPrivate;
class KMM_BASE_DIALOGS_EXPORT KNewEquityEntryDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KNewEquityEntryDlg)

public:
  explicit KNewEquityEntryDlg(QWidget *parent = nullptr);
  virtual ~KNewEquityEntryDlg();

  void setSymbolName(const QString& str);
  QString symbolName() const;

  void setName(const QString& str);
  QString name() const;

  int fraction() const;

protected Q_SLOTS:
  void onOKClicked();
  void slotDataChanged();

private:
  KNewEquityEntryDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KNewEquityEntryDlg)
};

#endif
