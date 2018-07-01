/*
 * Copyright 2014-2015  Cristian Oneț <onet.cristian@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config-kmymoney.h>

#include "mymoneycontact.h"

#ifdef KMM_ADDRESSBOOK_FOUND
#include <KIdentityManagement/IdentityManager>
#include <KIdentityManagement/Identity>
#include <AkonadiCore/RecursiveItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/Collection>
#include <KContacts/Addressee>
#include <QRegularExpression>
#endif

MyMoneyContact::MyMoneyContact(QObject *parent) : QObject(parent)
{
}

bool MyMoneyContact::ownerExists() const
{
#ifdef KMM_ADDRESSBOOK_FOUND
  KIdentityManagement::IdentityManager im;
  KIdentityManagement::Identity id = im.defaultIdentity();
  return !id.isNull();
#else
  return false;
#endif
}

QString MyMoneyContact::ownerEmail() const
{
#ifdef KMM_ADDRESSBOOK_FOUND
  KIdentityManagement::IdentityManager im;
  KIdentityManagement::Identity id = im.defaultIdentity();
  return id.primaryEmailAddress();
#else
  return QString();
#endif
}

QString MyMoneyContact::ownerFullName() const
{
#ifdef KMM_ADDRESSBOOK_FOUND
  KIdentityManagement::IdentityManager im;
  KIdentityManagement::Identity id = im.defaultIdentity();
  return id.fullName();
#else
  return QString();
#endif
}

void MyMoneyContact::fetchContact(const QString &email)
{
#ifdef KMM_ADDRESSBOOK_FOUND
  QRegularExpression re(".+@.+");
  if (!re.match(email).hasMatch()) {
    ContactData contact;
    emit contactFetched(contact);
  } else {
    // fetch the contact data
    Akonadi::RecursiveItemFetchJob *job = new Akonadi::RecursiveItemFetchJob(Akonadi::Collection::root(), QStringList() << KContacts::Addressee::mimeType());
    job->fetchScope().fetchFullPayload();
    job->fetchScope().setAncestorRetrieval(Akonadi::ItemFetchScope::Parent);
    job->setProperty("MyMoneyContact_email", email);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(searchContactResult(KJob*)));
    job->start();
  }
#else
  Q_UNUSED(email);
  ContactData contact;
  emit contactFetched(contact);
#endif
}

void MyMoneyContact::searchContactResult(KJob *job)
{
#ifdef KMM_ADDRESSBOOK_FOUND
  const Akonadi::RecursiveItemFetchJob *contactJob = qobject_cast<Akonadi::RecursiveItemFetchJob*>(job);
  Akonadi::Item::List items;
  if (contactJob)
    items = contactJob->items();
  ContactData contactData;
  contactData.email = job->property("MyMoneyContact_email").toString();
  foreach (const Akonadi::Item &item, items) {
    const KContacts::Addressee &contact = item.payload<KContacts::Addressee>();
    if (contact.emails().contains(contactData.email)) {
      KContacts::PhoneNumber nullPhone;
      KContacts::PhoneNumber& phone = nullPhone;
      const KContacts::PhoneNumber::List phones = contact.phoneNumbers();
      if (phones.count() == 1)
        phone = phones.first();
      else {
        const QList<KContacts::PhoneNumber::Type> typesList = {KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Pref,
                                                         KContacts::PhoneNumber::Work,
                                                         KContacts::PhoneNumber::Home | KContacts::PhoneNumber::Pref,
                                                         KContacts::PhoneNumber::Home};
        foreach (auto type,  typesList) {
          foreach (auto phn, phones) {
            if (phn.type() & type) {
              phone = phn;
              break;
            }
          }
          if (!phone.isEmpty())
            break;
        }
      }
      if (phone.isEmpty() && !phones.isEmpty())
        phone = phones.first();

      contactData.phoneNumber = phone.number();
      KContacts::Address address;
      KContacts::Address::List addresses = contact.addresses();
      if (addresses.count() == 1)
        address = addresses.first();
      else {
        QList<KContacts::Address::Type> typesList = {KContacts::Address::Work | KContacts::Address::Pref,
                                                     KContacts::Address::Work,
                                                     KContacts::Address::Home | KContacts::Address::Pref,
                                                     KContacts::Address::Home};
        foreach (auto type,  typesList) {
          foreach (auto addr, addresses) {
            if (addr.type() & type) {
              address = addr;
              break;
            }
          }
          if (!address.isEmpty())
            break;
        }
      }
      if (address.isEmpty() && !addresses.isEmpty())
        address = addresses.first();

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
