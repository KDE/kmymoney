/***************************************************************************
                          knewbankdlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWBANKDLG_H
#define KNEWBANKDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qdialog.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneykeyvaluecontainer.h"

#include "../dialogs/knewbankdlgdecl.h"

/// This dialog lets the user create or edit an institution
class KNewBankDlg : public KNewBankDlgDecl
{
  Q_OBJECT

public:
  KNewBankDlg(MyMoneyInstitution& institution, QWidget *parent = 0, const char *name = 0);
  ~KNewBankDlg();
  const MyMoneyInstitution& institution(void);

protected slots:
  void okClicked();
  void institutionNameChanged( const QString &);

private:
  MyMoneyInstitution m_institution;

};

#endif
