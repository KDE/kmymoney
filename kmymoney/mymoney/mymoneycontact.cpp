/*
    SPDX-FileCopyrightText: 2014-2015 Cristian One ț <onet.cristian@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>

#include "mymoneycontact.h"

#ifdef ENABLE_ADDRESSBOOK
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
#ifdef ENABLE_ADDRESSBOOK
    KIdentityManagement::IdentityManager im;
    KIdentityManagement::Identity id = im.defaultIdentity();
    return !id.isNull();
#else
    return false;
#endif
}

QString MyMoneyContact::ownerEmail() const
{
#ifdef ENABLE_ADDRESSBOOK
    KIdentityManagement::IdentityManager im;
    KIdentityManagement::Identity id = im.defaultIdentity();
    return id.primaryEmailAddress();
#else
    return QString();
#endif
}

QString MyMoneyContact::ownerFullName() const
{
#ifdef ENABLE_ADDRESSBOOK
    KIdentityManagement::IdentityManager im;
    KIdentityManagement::Identity id = im.defaultIdentity();
    return id.fullName();
#else
    return QString();
#endif
}

void MyMoneyContact::fetchContact(const QString &email)
{
#ifdef ENABLE_ADDRESSBOOK
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
#ifdef ENABLE_ADDRESSBOOK
    const Akonadi::RecursiveItemFetchJob *contactJob = qobject_cast<Akonadi::RecursiveItemFetchJob*>(job);
    Akonadi::Item::List items;
    if (contactJob)
        items = contactJob->items();
    ContactData contactData;
    contactData.email = job->property("MyMoneyContact_email").toString();
    foreach (const Akonadi::Item &item, items) {
        const KContacts::Addressee &contact = item.payload<KContacts::Addressee>();
        if (contact.emails().contains(contactData.email)) {
            KContacts::PhoneNumber phone;
            KContacts::PhoneNumber::List phones = contact.phoneNumbers();
            if (phones.count() == 1)
                phone = phones.first();
            else {
                QList<KContacts::PhoneNumber::Type> typesList = {KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Pref,
                                                                 KContacts::PhoneNumber::Work,
                                                                 KContacts::PhoneNumber::Home | KContacts::PhoneNumber::Pref,
                                                                 KContacts::PhoneNumber::Home
                                                                };
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
                                                             KContacts::Address::Home
                                                            };
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
