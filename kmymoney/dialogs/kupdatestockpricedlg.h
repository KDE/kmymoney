/*
 * SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2004-2010 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KUPDATESTOCKPRICEDLG_H
#define KUPDATESTOCKPRICEDLG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KUpdateStockPriceDlg; }

class QDate;

class MyMoneyMoney;

/**
  * @author Kevin Tambascio
  */

class KMM_BASE_DIALOGS_EXPORT KUpdateStockPriceDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KUpdateStockPriceDlg)

public:
  explicit KUpdateStockPriceDlg(QWidget* parent = nullptr);
  ~KUpdateStockPriceDlg();

  QDate date() const;
  MyMoneyMoney price() const;

  Ui::KUpdateStockPriceDlg *ui;

public Q_SLOTS:
  int exec() override;

protected Q_SLOTS:
  void slotCheckData();
  void slotCheckData(int idx);
};

#endif
