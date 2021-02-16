/*
    SPDX-FileCopyrightText: 2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KNEWEQUITYENTRYDLG_H
#define KNEWEQUITYENTRYDLG_H

#include <QDialog>

/**
  *
  * Dialog to allow user to enter all data for a stock or mutual fund investment type.
  *
  * @author Kevin Tambascio
  *
  */

class KNewEquityEntryDlgPrivate;
class KNewEquityEntryDlg : public QDialog
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
