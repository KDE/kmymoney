/*
 * Copyright 2005-2009  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KMYMONEYFILEINFODLG_H
#define KMYMONEYFILEINFODLG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KMyMoneyFileInfoDlg; }

/**
  * @author Thomas Baumgart
  */

class KMM_BASE_DIALOGS_EXPORT KMyMoneyFileInfoDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyFileInfoDlg)

public:
  explicit KMyMoneyFileInfoDlg(QWidget *parent = nullptr);
  ~KMyMoneyFileInfoDlg();

private:
  Ui::KMyMoneyFileInfoDlg *ui;
};

#endif
