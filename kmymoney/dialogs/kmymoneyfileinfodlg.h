/***************************************************************************
                          kmymoneyfileinfodlg.h  -  description
                             -------------------
    begin                : Sun Oct  9 2005
    copyright            : (C) 2005 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYFILEINFODLG_H
#define KMYMONEYFILEINFODLG_H

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

class KMyMoneyFileInfoDlg : public QDialog
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
