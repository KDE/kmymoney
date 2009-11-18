/***************************************************************************
                          kpayeereassigndlg.cpp
                             -------------------
    copyright            : (C) 2005 by Andreas Nicolai <ghorwin@users.sourceforge.net>
                           (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kpayeereassigndlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdialog.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneycombo.h>
#include <kguiutils.h>

KPayeeReassignDlg::KPayeeReassignDlg( QWidget* parent) :
  KPayeeReassignDlgDecl( parent)
{
  buttonOk->setGuiItem(KStandardGuiItem::ok());
  buttonCancel->setGuiItem(KStandardGuiItem::cancel());
  kMandatoryFieldGroup* mandatory = new kMandatoryFieldGroup(this);
  mandatory->add(payeeCombo);
  mandatory->setOkButton(buttonOk);
}

KPayeeReassignDlg::~KPayeeReassignDlg()
{
}

QString KPayeeReassignDlg::show(const QList<MyMoneyPayee>& payeeslist)
{
  if (payeeslist.isEmpty())
   return QString(); // no payee available? nothing can be selected...

  payeeCombo->loadPayees(payeeslist);

  // execute dialog and if aborted, return empty string
  if (this->exec() == QDialog::Rejected)
    return QString();

  // otherwise return index of selected payee
  return payeeCombo->selectedItem();
}


void KPayeeReassignDlg::accept(void)
{
  // force update of payeeCombo
  buttonOk->setFocus();

  if(payeeCombo->selectedItem().isEmpty()) {
    KMessageBox::information(this, i18n("This dialog does not allow to create new payees. Please pick a payee from the list."), i18n("Payee creation"));
  } else {
    KPayeeReassignDlgDecl::accept();
  }
}

#include "kpayeereassigndlg.moc"
