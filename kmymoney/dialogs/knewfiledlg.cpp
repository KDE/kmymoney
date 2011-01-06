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

#include "knewfiledlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QLabel>

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

#include "ui_knewfiledlgdecl.h"

struct KNewFileDlg::Private {
  Ui::KNewFileDlgDecl ui;
};

KNewFileDlg::KNewFileDlg(QWidget *parent, const QString& title)
    : QDialog(parent), d(new Private)
{
  d->ui.setupUi(this);
  setModal(true);
  init(title);
}

KNewFileDlg::KNewFileDlg(QString userName, QString userStreet,
                         QString userTown, QString userCounty, QString userPostcode, QString userTelephone,
                         QString userEmail, QWidget *parent, const QString& title)
    : QDialog(parent), d(new Private)
{
  d->ui.setupUi(this);
  setModal(true);
  d->ui.userNameEdit->setText(userName);
  d->ui.streetEdit->setText(userStreet);
  d->ui.townEdit->setText(userTown);
  d->ui.countyEdit->setText(userCounty);
  d->ui.postcodeEdit->setText(userPostcode);
  d->ui.telephoneEdit->setText(userTelephone);
  d->ui.emailEdit->setText(userEmail);

  init(title);
}

void KNewFileDlg::init(const QString& title)
{
  bool showLoadButton = false;
  d->ui.okBtn->setGuiItem(KStandardGuiItem::ok());
  d->ui.cancelBtn->setGuiItem(KStandardGuiItem::cancel());

  if (!title.isEmpty())
    setWindowTitle(title);

  KABC::StdAddressBook *ab = static_cast<KABC::StdAddressBook*>
                             (KABC::StdAddressBook::self());
  if (ab && !ab->whoAmI().isEmpty())
    showLoadButton = true;

  if (!showLoadButton)
    d->ui.kabcBtn->hide();

  d->ui.userNameEdit->setFocus();

  connect(d->ui.cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
  connect(d->ui.okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(d->ui.kabcBtn, SIGNAL(clicked()), this, SLOT(loadFromKABC()));
}

KNewFileDlg::~KNewFileDlg()
{
  delete d;
}

void KNewFileDlg::okClicked()
{
  userNameText = d->ui.userNameEdit->text();
  userStreetText = d->ui.streetEdit->text();
  userTownText = d->ui.townEdit->text();
  userCountyText = d->ui.countyEdit->text();
  userPostcodeText = d->ui.postcodeEdit->text();
  userTelephoneText = d->ui.telephoneEdit->text();
  userEmailText = d->ui.emailEdit->text();

  accept();
}

void KNewFileDlg::loadFromKABC(void)
{
  KABC::StdAddressBook *ab = static_cast<KABC::StdAddressBook*>
                             (KABC::StdAddressBook::self());
  if (!ab)
    return;

  KABC::Addressee addr = ab->whoAmI();
  if (addr.isEmpty()) {
    KMessageBox::sorry(this, i18n("Unable to load data, because no contact has been associated with the owner of the standard address book."), i18n("Address book import"));
    return;
  }

  d->ui.userNameEdit->setText(addr.formattedName());
  d->ui.emailEdit->setText(addr.preferredEmail());

  KABC::PhoneNumber phone = addr.phoneNumber(KABC::PhoneNumber::Home);
  d->ui.telephoneEdit->setText(phone.number());

  KABC::Address a = addr.address(KABC::Address::Home);
  d->ui.countyEdit->setText(a.country() + " / " + a.region());
  d->ui.postcodeEdit->setText(a.postalCode());
  d->ui.townEdit->setText(a.locality());
  d->ui.streetEdit->setText(a.street());
}

KPushButton* KNewFileDlg::cancelButton(void)
{
  return d->ui.cancelBtn;
}

#include "knewfiledlg.moc"
