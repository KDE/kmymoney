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
#include <QLineEdit>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <kguiutils.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneymvccombo.h>

/** This lookup table needs to be in sync with KPayeeReassignDlg::OperationType enum */
static const char * labelText[KPayeeReassignDlg::TypeCount] = {
  I18N_NOOP("To be able to merge previous selected payees, please select a payee from the list below or create a new one."),
  I18N_NOOP("The transactions associated with the selected payees need to be re-assigned to a different payee before the selected payees can be deleted. Please select a payee from the list below."),
};

KPayeeReassignDlg::KPayeeReassignDlg(KPayeeReassignDlg::OperationType type, QWidget* parent) :
    KPayeeReassignDlgDecl(parent),
    m_type(type)
{
  kMandatoryFieldGroup* mandatory = new kMandatoryFieldGroup(this);
  mandatory->add(payeeCombo);
  mandatory->setOkButton(buttonBox->button(QDialogButtonBox::Ok));
  textLabel1->setText(i18n(labelText[m_type]));
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

  // allow to return the text (new payee) if type is Merge
  if (m_type == TypeMerge && payeeCombo->selectedItem().isEmpty())
    return payeeCombo->lineEdit()->text();

  // otherwise return index of selected payee
  return payeeCombo->selectedItem();
}


void KPayeeReassignDlg::accept()
{
  // force update of payeeCombo
  buttonBox->button(QDialogButtonBox::Ok)->setFocus();

  if (m_type == TypeDelete && payeeCombo->selectedItem().isEmpty()) {
    KMessageBox::information(this, i18n("This dialog does not allow new payees to be created. Please pick a payee from the list."), i18n("Payee creation"));
  } else {
    KPayeeReassignDlgDecl::accept();
  }
}
