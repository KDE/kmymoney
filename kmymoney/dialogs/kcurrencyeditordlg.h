/*
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KCURRENCYEDITORDLG_H
#define KCURRENCYEDITORDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui
{
class KCurrencyEditorDlg;
}

class MyMoneySecurity;
class KCurrencyEditorDlgPrivate;
class KCurrencyEditorDlg : public QDialog
{
  Q_DISABLE_COPY(KCurrencyEditorDlg)

  Q_OBJECT
public:
  explicit KCurrencyEditorDlg(const MyMoneySecurity &currency, QWidget *parent = nullptr);
  ~KCurrencyEditorDlg();

  MyMoneySecurity currency() const;

private:
  KCurrencyEditorDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KCurrencyEditorDlg)
};

#endif
