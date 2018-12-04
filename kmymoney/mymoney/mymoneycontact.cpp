/*
 * Copyright 2014  Cristian Oneț <onet.cristian@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <config-kmymoney.h>

#include "mymoneycontact.h"

#ifdef HAVE_KDEPIMLIBS
#include <KPIMIdentities/IdentityManager>
#include <KPIMIdentities/Identity>
#ifdef HAVE_AKONADI
#include <akonadi/recursiveitemfetchjob.h>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Collection>
#endif
#include <KABC/Addressee>
#endif

MyMoneyContact::MyMoneyContact(QObject *parent) : QObject(parent)
{
}

bool MyMoneyContact::ownerExists() const
{
#ifdef HAVE_KDEPIMLIBS
  KPIMIdentities::IdentityManager im;
  KPIMIdentities::Identity id = im.defaultIdentity();
  return !id.isNull();
#else
  return false;
#endif
}

QString MyMoneyContact::ownerEmail() const
{
#ifdef HAVE_KDEPIMLIBS
  KPIMIdentities::IdentityManager im;
  KPIMIdentities::Identity id = im.defaultIdentity();
  return id.primaryEmailAddress();
#else
  return QString();
#endif
}

QString MyMoneyContact::ownerFullName() const
{
#ifdef HAVE_KDEPIMLIBS
  KPIMIdentities::IdentityManager im;
  KPIMIdentities::Identity id = im.defaultIdentity();
  return id.fullName();
#else
  return QString();
#endif
}

void MyMoneyContact::fetchContact(const QString &email)
{
#if defined(HAVE_KDEPIMLIBS) && defined(HAVE_AKONADI)
  // fetch the contact data
  Akonadi::RecursiveItemFetchJob *job = new Akonadi::RecursiveItemFetchJob(Akonadi::Collection::root(), QStringList() << KABC::Addressee::mimeType());
  job->fetchScope().fetchFullPayload();
  job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
  job->setProperty("MyMoneyContact_email", email);
  connect(job, SIGNAL(result(KJob*)), this, SLOT(searchContactResult(KJob*)));
  job->start();
#else
  ContactData contact;
  contact.email = email;
  emit contactFetched(contact);
#endif
}

void MyMoneyContact::searchContactResult(KJob *job)
{
#if defined(HAVE_KDEPIMLIBS) && defined(HAVE_AKONADI)
  const Akonadi::RecursiveItemFetchJob *contactJob = qobject_cast<Akonadi::RecursiveItemFetchJob*>(job);
  Akonadi::Item::List items;
  if (contactJob)
    items = contactJob->items();
  ContactData contactData;
  contactData.email = job->property("MyMoneyContact_email").toString();
  foreach (const Akonadi::Item &item, items) {
    const KABC::Addressee &contact = item.payload<KABC::Addressee>();
    if (contact.emails().contains(contactData.email)) {
      KABC::PhoneNumber phone = contact.phoneNumber(KABC::PhoneNumber::Home);
      contactData.phoneNumber = phone.number();
      const KABC::Address &address = contact.address(KABC::Address::Home);
      contactData.street = address.street();
      contactData.locality = address.locality();
      contactData.country = address.country();
      contactData.region = address.region();
      contactData.postalCode = address.postalCode();
      break;
    }
  }
  emit contactFetched(contactData);
#else
  Q_UNUSED(job);
#endif
}
