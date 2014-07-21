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

#ifdef KMM_ADDRESSBOOK_FOUND
#include <KPIMIdentities/IdentityManager>
#include <KPIMIdentities/Identity>
#include <AkonadiCore/RecursiveItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/Collection>
#include <KABC/Addressee>
#endif

MyMoneyContact::MyMoneyContact(QObject *parent) : QObject(parent)
{
}

bool MyMoneyContact::ownerExists() const
{
#ifdef KMM_ADDRESSBOOK_FOUND
  KPIMIdentities::IdentityManager im;
  KPIMIdentities::Identity id = im.defaultIdentity();
  return !id.isNull();
#else
  return false;
#endif
}

QString MyMoneyContact::ownerEmail() const
{
#ifdef KMM_ADDRESSBOOK_FOUND
  KPIMIdentities::IdentityManager im;
  KPIMIdentities::Identity id = im.defaultIdentity();
  return id.primaryEmailAddress();
#else
  return QString();
#endif
}

QString MyMoneyContact::ownerFullName() const
{
#ifdef KMM_ADDRESSBOOK_FOUND
  KPIMIdentities::IdentityManager im;
  KPIMIdentities::Identity id = im.defaultIdentity();
  return id.fullName();
#else
  return QString();
#endif
}

void MyMoneyContact::fetchContact(const QString &email)
{
  // reset the identity
  m_contact = ContactData();
  m_contact.email = email;

#ifdef KMM_ADDRESSBOOK_FOUND
  // fetch the contact data
  Akonadi::RecursiveItemFetchJob *job = new Akonadi::RecursiveItemFetchJob(Akonadi::Collection::root(), QStringList() << KABC::Addressee::mimeType());
  job->fetchScope().fetchFullPayload();
  job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
  connect(job, SIGNAL(result(KJob*)), this, SLOT(searchContactResult(KJob*)));
  job->start();
#else
  emit contactFetched(m_contact);
#endif
}

void MyMoneyContact::searchContactResult(KJob *job)
{
#ifdef KMM_ADDRESSBOOK_FOUND
  const Akonadi::RecursiveItemFetchJob *contactJob = qobject_cast<Akonadi::RecursiveItemFetchJob*>(job);
  Akonadi::Item::List items;
  if (contactJob)
    items = contactJob->items();
  foreach (const Akonadi::Item &item, items) {
    const KABC::Addressee &contact = item.payload<KABC::Addressee>();
    if (contact.emails().contains(m_contact.email)) {
      KABC::PhoneNumber phone = contact.phoneNumber(KABC::PhoneNumber::Home);
      m_contact.phoneNumber = phone.number();
      const KABC::Address &address = contact.address(KABC::Address::Home);
      m_contact.street = address.street();
      m_contact.locality = address.locality();
      m_contact.country = address.country();
      m_contact.region = address.region();
      m_contact.postalCode = address.postalCode();
      break;
    }
  }
  emit contactFetched(m_contact);
#else
  Q_UNUSED(job);
#endif
}
