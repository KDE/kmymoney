/***************************************************************************
                          knewbudgetdlg.h
                             -------------------
    begin                : Wed Jan 18 2006
    copyright            : (C) 2000-2004 by Darren Gould
    email                : darren_gould@gmx.de
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

#ifndef KNEWBUDGETDLG_H
#define KNEWBUDGETDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KNewBudgetDlgPrivate;
class KNewBudgetDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KNewBudgetDlg)

public:
  explicit KNewBudgetDlg(QWidget* parent = nullptr);
  ~KNewBudgetDlg();

  QString getYear() const;
  QString getName() const;

public slots:
  void m_pbCancel_clicked();
  void m_pbOk_clicked();

private:
  KNewBudgetDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KNewBudgetDlg)
};

#endif // KNEWBUDGETDLG_H
