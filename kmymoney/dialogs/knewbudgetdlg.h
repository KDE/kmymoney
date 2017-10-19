/***************************************************************************
                          knewbudgetdlg.h
                             -------------------
    begin                : Wed Jan 18 2006
    copyright            : (C) 2000-2004 by Darren Gould
    email                : darren_gould@gmx.de
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

class KNewBudgetDlg : public QDialog
{
  Q_OBJECT

public:
  KNewBudgetDlg(QWidget* parent);
  ~KNewBudgetDlg();

  QString& getYear();
  QString& getName();

public slots:
  virtual void m_pbCancel_clicked();
  virtual void m_pbOk_clicked();

private:
  struct Private;
  Private* const d;
};

#endif // KNEWBUDGETDLG_H
