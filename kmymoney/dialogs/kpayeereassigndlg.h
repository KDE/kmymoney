/*
    SPDX-FileCopyrightText: 2005 Andreas Nicolai <Andreas.Nicolai@gmx.net>
    SPDX-FileCopyrightText: 2007-2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KPAYEEREASSIGNDLG_H
#define KPAYEEREASSIGNDLG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
 *  Implementation of the dialog that lets the user select a payee in order
 *  to re-assign transactions (for instance, if payees are deleted).
 */

class MyMoneyPayee;

class KPayeeReassignDlgPrivate;
class KMM_BASE_DIALOGS_EXPORT KPayeeReassignDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KPayeeReassignDlg)

public:
  /** Change behavior based on type of operation */
  enum OperationType {
    TypeMerge = 0,
    TypeDelete,
    TypeCount,
  };

  /** Default constructor */
  explicit KPayeeReassignDlg(OperationType type, QWidget* parent = nullptr);

  /** Destructor */
  ~KPayeeReassignDlg();

  /**
    * This function sets up the dialog, lets the user select a payee and returns
    * the id of the selected payee in the payeeslist.
    *
    * @param payeeslist reference to QList of MyMoneyPayee objects to be contained in the list
    *
    * @return Returns the id of the selected payee in the list or QString() if
    *         the dialog was aborted. QString() is also returned if the payeeslist is empty.
    */
  QString show(const QList<MyMoneyPayee>& payeeslist);

  /**
   * Returns true, if the names of the payees to be deleted should be copied
   * to the selected payee's match list.
   */
  bool addToMatchList() const;

protected:
  void accept() override;

private:  
  KPayeeReassignDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KPayeeReassignDlg)
};

#endif // KPAYEEREASSIGNDLG_H
