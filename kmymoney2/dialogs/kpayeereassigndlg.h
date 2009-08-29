/***************************************************************************
                          kpayeereassigndlg.cpp
                             -------------------
    copyright            : (C) 2005 by Andreas Nicolai
                           (C) 2007 by Thomas Baumgart
    author               : Andreas Nicolai, Thomas Baumgart
    email                : <ghorwin@users.sourceforge.net>
                           <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KPAYEEREASSIGNDLG_H
#define KPAYEEREASSIGNDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <q3valuelist.h>
#include <QCheckBox>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneypayee.h>
#include "ui_kpayeereassigndlgdecl.h"

/**
 *  Implementation of the dialog that lets the user select a payee in order
 *  to re-assign transactions (for instance, if payees are deleted).
 */


class KPayeeReassignDlgDecl : public QDialog, public Ui::KPayeeReassignDlgDecl
{
public:
  KPayeeReassignDlgDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};

class KPayeeReassignDlg : public KPayeeReassignDlgDecl
{
  Q_OBJECT
public:
  /** Default constructor */
  KPayeeReassignDlg( QWidget* parent = 0);

  /** Destructor */
  ~KPayeeReassignDlg();

  /**
    * This function sets up the dialog, lets the user select a payee and returns
    * the id of the selected payee in the payeeslist.
    *
    * @param payeeslist reference to QValueList of MyMoneyPayee objects to be contained in the list
    *
    * @return Returns the id of the selected payee in the list or QString() if
    *         the dialog was aborted. QString() is also returned if the payeeslist is empty.
    */
  QString show(const Q3ValueList<MyMoneyPayee>& payeeslist);

  /**
   * Returns true, if the names of the payees to be deleted should be copied
   * to the selected payee's match list.
   */
  bool addToMatchList(void) const { return m_copyToMatchList->isChecked(); }

protected:
  void accept(void);

};

#endif // KPAYEEREASSIGNDLG_H
