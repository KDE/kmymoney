/***************************************************************************
                          knewfiledlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
#include "ui_editpersonaldatadlg.h"

class EditPersonalDataDlgPrivate
{
  Q_DISABLE_COPY(EditPersonalDataDlgPrivate)
  Q_DECLARE_PUBLIC(EditPersonalDataDlg)

public:
  EditPersonalDataDlgPrivate(EditPersonalDataDlg *qq) :
    q_ptr(qq),
    ui(new Ui::EditPersonalDataDlg)
  {
  }

  ~EditPersonalDataDlgPrivate()
  {
    delete m_contact;
    delete ui;
  }

  void init(const QString& title)
  {
    Q_Q(EditPersonalDataDlg);
    m_contact = new MyMoneyContact(q);
    ui->setupUi(q);
    q->setModal(true);

    if (!title.isEmpty())
      q->setWindowTitle(title);

    ui->kabcBtn->setEnabled(m_contact->ownerExists());
    ui->userNameEdit->setFocus();

    q->connect(ui->buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);
    q->connect(ui->buttonBox, &QDialogButtonBox::accepted, q, &EditPersonalDataDlg::okClicked);
    q->connect(ui->kabcBtn, &QAbstractButton::clicked, q, &EditPersonalDataDlg::loadFromAddressBook);
  }

  EditPersonalDataDlg      *q_ptr;
  Ui::EditPersonalDataDlg  *ui;
  MyMoneyContact           *m_contact;
  QString                   userNameText;
  QString                   userStreetText;
  QString                   userTownText;
  QString                   userCountyText;
  QString                   userPostcodeText;
  QString                   userTelephoneText;
  QString                   userEmailText;
};


EditPersonalDataDlg::EditPersonalDataDlg(QWidget *parent, const QString& title) :
  QDialog(parent),
  d_ptr(new EditPersonalDataDlgPrivate(this))
{
  Q_D(EditPersonalDataDlg);
  d->init(title);
}

EditPersonalDataDlg::EditPersonalDataDlg(QString userName,
                                         QString userStreet,
                                         QString userTown,
                                         QString userCounty,
                                         QString userPostcode,
                                         QString userTelephone,
                                         QString userEmail,
                                         QWidget *parent,
                                         const QString& title) :
  QDialog(parent),
  d_ptr(new EditPersonalDataDlgPrivate(this))
{
  Q_D(EditPersonalDataDlg);
  d->init(title);
  d->ui->userNameEdit->setText(userName);
  d->ui->streetEdit->setText(userStreet);
  d->ui->townEdit->setText(userTown);
  d->ui->countyEdit->setText(userCounty);
  d->ui->postcodeEdit->setText(userPostcode);
  d->ui->telephoneEdit->setText(userTelephone);
  d->ui->emailEdit->setText(userEmail);
}

QString EditPersonalDataDlg::userName() const
{
  Q_D(const EditPersonalDataDlg);
  return d->userNameText;
}

QString EditPersonalDataDlg::userStreet() const
{
  Q_D(const EditPersonalDataDlg);
  return d->userStreetText;
}

QString EditPersonalDataDlg::userTown() const
{
  Q_D(const EditPersonalDataDlg);
  return d->userTownText;
}

QString EditPersonalDataDlg::userCountry() const
{
  Q_D(const EditPersonalDataDlg);
  return d->userCountyText;
}

QString EditPersonalDataDlg::userPostcode() const
{
  Q_D(const EditPersonalDataDlg);
  return d->userPostcodeText;
}

QString EditPersonalDataDlg::userTelephone() const
{
  Q_D(const EditPersonalDataDlg);
  return d->userTelephoneText;
}

QString EditPersonalDataDlg::userEmail() const
{
  Q_D(const EditPersonalDataDlg);
  return d->userEmailText;
}

EditPersonalDataDlg::~EditPersonalDataDlg()
{
  Q_D(EditPersonalDataDlg);
  delete d;
}

void EditPersonalDataDlg::okClicked()
{
  Q_D(EditPersonalDataDlg);
  d->userNameText = d->ui->userNameEdit->text();
  d->userStreetText = d->ui->streetEdit->text();
  d->userTownText = d->ui->townEdit->text();
  d->userCountyText = d->ui->countyEdit->text();
  d->userPostcodeText = d->ui->postcodeEdit->text();
  d->userTelephoneText = d->ui->telephoneEdit->text();
  d->userEmailText = d->ui->emailEdit->text();

  accept();
}

void EditPersonalDataDlg::loadFromAddressBook()
{
  Q_D(EditPersonalDataDlg);
  d->ui->userNameEdit->setText(d->m_contact->ownerFullName());
  d->ui->emailEdit->setText(d->m_contact->ownerEmail());
  if (d->ui->emailEdit->text().isEmpty()) {
    KMessageBox::sorry(this, i18n("Unable to load data, because no contact has been associated with the owner of the standard address book."), i18n("Address book import"));
    return;
  }
  d->ui->kabcBtn->setEnabled(false);
  connect(d->m_contact, &MyMoneyContact::contactFetched, this, &EditPersonalDataDlg::slotContactFetched);
  d->m_contact->fetchContact(d->ui->emailEdit->text());
}

void EditPersonalDataDlg::slotContactFetched(const ContactData &identity)
{
  Q_D(EditPersonalDataDlg);
  d->ui->telephoneEdit->setText(identity.phoneNumber);
  QString sep;
  if (!identity.country.isEmpty() && !identity.region.isEmpty())
    sep = " / ";
  d->ui->countyEdit->setText(QString("%1%2%3").arg(identity.country, sep, identity.region));
  d->ui->postcodeEdit->setText(identity.postalCode);
  d->ui->townEdit->setText(identity.locality);
  d->ui->streetEdit->setText(identity.street);
  d->ui->kabcBtn->setEnabled(true);
}

