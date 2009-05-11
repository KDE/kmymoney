/***************************************************************************
                          knewbankdlg.cpp
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


# include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kguiitem.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/kguiutils.h>

#include "knewbankdlg.h"

KNewBankDlg::KNewBankDlg(MyMoneyInstitution& institution, QWidget *parent, const char *name)
  : KNewBankDlgDecl(parent,name,true), m_institution(institution)
{
  okBtn->setGuiItem(KStandardGuiItem::ok());
  cancelBtn->setGuiItem(KStandardGuiItem::cancel());

  nameEdit->setFocus();
  nameEdit->setText(institution.name());
  cityEdit->setText(institution.city());
  streetEdit->setText(institution.street());
  postcodeEdit->setText(institution.postcode());
  telephoneEdit->setText(institution.telephone());
  bicEdit->setText(institution.value("bic"));
  sortCodeEdit->setText(institution.sortcode());

  connect(okBtn, SIGNAL(clicked()), SLOT(okClicked()));
  connect(cancelBtn, SIGNAL(clicked()), SLOT(reject()));
  connect(nameEdit, SIGNAL(textChanged ( const QString & )), SLOT(institutionNameChanged( const QString &)));
  institutionNameChanged( nameEdit->text());

  kMandatoryFieldGroup* requiredFields = new kMandatoryFieldGroup (this);
  requiredFields->setOkButton(okBtn); // button to be enabled when all fields present
  requiredFields->add(nameEdit);
}

void KNewBankDlg::institutionNameChanged( const QString &_text)
{
  okBtn->setEnabled( !_text.isEmpty() );
}

KNewBankDlg::~KNewBankDlg()
{
}

void KNewBankDlg::okClicked()
{
  if (nameEdit->text().isEmpty()) {
    KMessageBox::information(this, i18n("The institution name field is empty.  Please enter the name."), i18n("Adding New Institution"));
    nameEdit->setFocus();
    return;
  }

  m_institution.setName(nameEdit->text());
  m_institution.setTown(cityEdit->text());
  m_institution.setStreet(streetEdit->text());
  m_institution.setPostcode(postcodeEdit->text());
  m_institution.setTelephone(telephoneEdit->text());
  m_institution.setValue("bic", bicEdit->text());
  m_institution.setSortcode(sortCodeEdit->text());

  accept();
}

const MyMoneyInstitution& KNewBankDlg::institution(void)
{
  return m_institution;
}


#include "knewbankdlg.moc"
