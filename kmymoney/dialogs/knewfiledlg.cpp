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
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kstandardguiitem.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneycontact.h"
#include "ui_knewfiledlgdecl.h"

struct KNewFileDlg::Private {
  Private() : m_contact(0) {}
  Ui::KNewFileDlgDecl ui;
  MyMoneyContact *m_contact;
};

KNewFileDlg::KNewFileDlg(QWidget *parent, const QString& title)
    : QDialog(parent), d(new Private)
{
  d->m_contact = new MyMoneyContact(this);
  d->ui.setupUi(this);
  setModal(true);
  init(title);
}

KNewFileDlg::KNewFileDlg(QString userName, QString userStreet,
                         QString userTown, QString userCounty, QString userPostcode, QString userTelephone,
                         QString userEmail, QWidget *parent, const QString& title)
    : QDialog(parent), d(new Private)
{
  d->m_contact = new MyMoneyContact(this);
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
    // TODO: port KF5
  //d->ui.okBtn->setGuiItem(KStandardGuiItem::ok());
  //d->ui.cancelBtn->setGuiItem(KStandardGuiItem::cancel());

  if (!title.isEmpty())
    setWindowTitle(title);

  d->ui.kabcBtn->setEnabled(d->m_contact->ownerExists());
  d->ui.userNameEdit->setFocus();

  connect(d->ui.cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
  connect(d->ui.okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(d->ui.kabcBtn, SIGNAL(clicked()), this, SLOT(loadFromAddressBook()));
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

void KNewFileDlg::loadFromAddressBook(void)
{
  d->ui.userNameEdit->setText(d->m_contact->ownerFullName());
  d->ui.emailEdit->setText(d->m_contact->ownerEmail());
  if (d->ui.emailEdit->text().isEmpty()) {
    KMessageBox::sorry(this, i18n("Unable to load data, because no contact has been associated with the owner of the standard address book."), i18n("Address book import"));
    return;
  }
  d->ui.kabcBtn->setEnabled(false);
  connect(d->m_contact, SIGNAL(contactFetched(ContactData)), this, SLOT(slotContactFetched(ContactData)));
  d->m_contact->fetchContact(d->ui.emailEdit->text());
}

void KNewFileDlg::slotContactFetched(const ContactData &identity)
{
  d->ui.telephoneEdit->setText(identity.phoneNumber);
  QString sep;
  if (!identity.country.isEmpty() && !identity.region.isEmpty())
    sep = " / ";
  d->ui.countyEdit->setText(QString("%1%2%3").arg(identity.country, sep, identity.region));
  d->ui.postcodeEdit->setText(identity.postalCode);
  d->ui.townEdit->setText(identity.locality);
  d->ui.streetEdit->setText(identity.street);
  d->ui.kabcBtn->setEnabled(true);
}

QPushButton* KNewFileDlg::cancelButton(void)
{
  return d->ui.cancelBtn;
}
