/*
 * Copyright 2002-2004  Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2004-2010  Thomas Baumgart <tbaumgart@kde.org>
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
