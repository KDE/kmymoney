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
#include <KPIMIdentities/IdentityManager>
#include <KPIMIdentities/Identity>
#include <akonadi/recursiveitemfetchjob.h>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Collection>
#include <KABC/Addressee>

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

  KPIMIdentities::IdentityManager im;
  KPIMIdentities::Identity id = im.defaultIdentity();
  if (!id.isNull())
    showLoadButton = true;

  if (!showLoadButton)
    d->ui.kabcBtn->hide();

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
  KPIMIdentities::IdentityManager im;
  KPIMIdentities::Identity id = im.defaultIdentity();
  if (id.isNull() || id.primaryEmailAddress().isEmpty()) {
    KMessageBox::sorry(this, i18n("Unable to load data, because no contact has been associated with the owner of the standard address book."), i18n("Address book import"));
    return;
  }

  // Search all contacts for the matching email address
  d->ui.kabcBtn->setEnabled(false);
  Akonadi::RecursiveItemFetchJob *job = new Akonadi::RecursiveItemFetchJob(Akonadi::Collection::root(), QStringList() << KABC::Addressee::mimeType());
  job->fetchScope().fetchFullPayload();
  job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
  connect(job, SIGNAL(result(KJob*)), this, SLOT(searchContactResult(KJob*)));
  job->start();

  d->ui.userNameEdit->setText(id.fullName());
  d->ui.emailEdit->setText(id.primaryEmailAddress());
}

void KNewFileDlg::searchContactResult(KJob *job)
{
  const Akonadi::RecursiveItemFetchJob *contactJob = qobject_cast<Akonadi::RecursiveItemFetchJob*>(job);
  Akonadi::Item::List items;
  if (contactJob)
    items = contactJob->items();
  foreach (const Akonadi::Item &item, items) {
    const KABC::Addressee &contact = item.payload<KABC::Addressee>();
    if (contact.emails().contains(d->ui.emailEdit->text())) {
      KABC::PhoneNumber phone = contact.phoneNumber(KABC::PhoneNumber::Home);
      d->ui.telephoneEdit->setText(phone.number());

      const KABC::Address &address = contact.address(KABC::Address::Home);
      d->ui.countyEdit->setText(address.country() + " / " + address.region());
      d->ui.postcodeEdit->setText(address.postalCode());
      d->ui.townEdit->setText(address.locality());
      d->ui.streetEdit->setText(address.street());
      break;
    }
  }
  d->ui.kabcBtn->setEnabled(true);
}

KPushButton* KNewFileDlg::cancelButton(void)
{
  return d->ui.cancelBtn;
}
