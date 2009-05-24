/***************************************************************************
                          knewfiledlg.cpp
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



// ----------------------------------------------------------------------------
// QT Includes

#include <qpixmap.h>
#include <qlineedit.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewfiledlg.h"

KNewFileDlg::KNewFileDlg(QWidget *parent, const char *name, const QString& title)
  : KNewFileDlgDecl(parent)
{
    setModal( true );
  init(title);
}

KNewFileDlg::KNewFileDlg(QString userName, QString userStreet,
  QString userTown, QString userCounty, QString userPostcode, QString userTelephone,
  QString userEmail, QWidget *parent, const char *name, const QString& title)
  : KNewFileDlgDecl(parent)
{
    setModal( true );
  userNameEdit->setText(userName);
  streetEdit->setText(userStreet);
  townEdit->setText(userTown);
  countyEdit->setText(userCounty);
  postcodeEdit->setText(userPostcode);
  telephoneEdit->setText(userTelephone);
  emailEdit->setText(userEmail);

  init(title);
}

void KNewFileDlg::init(const QString& title)
{
  bool showLoadButton = false;
  okBtn->setGuiItem(KStandardGuiItem::ok());
  cancelBtn->setGuiItem(KStandardGuiItem::cancel());

  if (!title.isEmpty())
    setCaption(title);

  KABC::StdAddressBook *ab = static_cast<KABC::StdAddressBook*>
    ( KABC::StdAddressBook::self() );
  if ( ab && !ab->whoAmI().isEmpty() )
      showLoadButton = true;

  if(!showLoadButton)
    kabcBtn->hide();

  userNameEdit->setFocus();

  connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
  connect(okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(kabcBtn, SIGNAL(clicked()), this, SLOT(loadFromKABC()));
}

KNewFileDlg::~KNewFileDlg(){
}

void KNewFileDlg::okClicked()
{
  userNameText = userNameEdit->text();
  userStreetText = streetEdit->text();
  userTownText = townEdit->text();
  userCountyText = countyEdit->text();
  userPostcodeText = postcodeEdit->text();
  userTelephoneText = telephoneEdit->text();
  userEmailText = emailEdit->text();

  accept();
}

void KNewFileDlg::loadFromKABC(void)
{
  KABC::StdAddressBook *ab = static_cast<KABC::StdAddressBook*>
    ( KABC::StdAddressBook::self() );
  if ( !ab )
    return;

  KABC::Addressee addr = ab->whoAmI();
  if ( addr.isEmpty() ) {
    KMessageBox::sorry(this, i18n("Unable to load data, because no contact has been associated with the owner of the standard addressbook."), i18n("Addressbook import"));
    return;
  }

  userNameEdit->setText( addr.formattedName() );
  emailEdit->setText( addr.preferredEmail() );

  KABC::PhoneNumber phone = addr.phoneNumber( KABC::PhoneNumber::Home );
  telephoneEdit->setText( phone.number() );

  KABC::Address a = addr.address( KABC::Address::Home );
  countyEdit->setText( a.country() + " / " + a.region() );
  postcodeEdit->setText( a.postalCode() );
  townEdit->setText( a.locality() );
  streetEdit->setText( a.street() );
}

#include "knewfiledlg.moc"
