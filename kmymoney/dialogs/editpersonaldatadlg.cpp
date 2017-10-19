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

#include "editpersonaldatadlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneycontact.h"
#include "ui_editpersonaldatadlgdecl.h"

struct EditPersonalDataDlg::Private {
  Private() : m_contact(0) {}
  Ui::EditPersonalDataDlgDecl ui;
  MyMoneyContact *m_contact;
};

EditPersonalDataDlg::EditPersonalDataDlg(QWidget *parent, const QString& title)
    : QDialog(parent), d(new Private)
{
  d->m_contact = new MyMoneyContact(this);
  d->ui.setupUi(this);
  setModal(true);
  init(title);
}

EditPersonalDataDlg::EditPersonalDataDlg(QString userName, QString userStreet,
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

void EditPersonalDataDlg::init(const QString& title)
{
  if (!title.isEmpty())
    setWindowTitle(title);

  d->ui.kabcBtn->setEnabled(d->m_contact->ownerExists());
  d->ui.userNameEdit->setFocus();

  connect(d->ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(d->ui.buttonBox, SIGNAL(accepted()), this, SLOT(okClicked()));
  connect(d->ui.kabcBtn, SIGNAL(clicked()), this, SLOT(loadFromAddressBook()));
}

EditPersonalDataDlg::~EditPersonalDataDlg()
{
  delete d;
}

void EditPersonalDataDlg::okClicked()
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

void EditPersonalDataDlg::loadFromAddressBook()
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

void EditPersonalDataDlg::slotContactFetched(const ContactData &identity)
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

