/*
    SPDX-FileCopyrightText: 2005-2009 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

namespace Ui {
class KMyMoneyFileInfoDlg;
}

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
