/***************************************************************************
                          kconfirmmanualenterdlg.h
                             -------------------
    begin                : Mon Apr  9 2007
    copyright            : (C) 2007 by Thomas Baumgart
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

#ifndef KCONFIRMMANUALENTERDLG_H
#define KCONFIRMMANUALENTERDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySchedule;
class MyMoneyTransaction;

namespace Ui { class KConfirmManualEnterDlg; }

class KConfirmManualEnterDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KConfirmManualEnterDlg)

public:
  explicit KConfirmManualEnterDlg(const MyMoneySchedule& schedule, QWidget* parent = nullptr);
  ~KConfirmManualEnterDlg();

  typedef enum {
    UseOriginal = 0,
    ModifyOnce,
    ModifyAlways
  } Action;

  /**
    * setup the dialog for the difference between the original transaction
    * @a to and the transaction to be entered @a tn.
    */
  void loadTransactions(const MyMoneyTransaction& to, const MyMoneyTransaction& tn);

  /**
    * Returns information about what to do with the transaction
    */
  Action action() const;

private:
  Ui::KConfirmManualEnterDlg *ui;
};

#endif // KCONFIRMMANUALENTERDLG_H
