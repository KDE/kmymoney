/*
    SPDX-FileCopyrightText: 2014-2015 Cristian Oneț <onet.cristian@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYCONTACT_H
#define MYMONEYCONTACT_H

#include <QObject>
#include <QString>
#include <QMetaType>

#include "kmm_mymoney_export.h"

/**
  * POD containing contact data, these fields are retrieved based on an email address.
  */
struct ContactData {
    QString email;
    QString phoneNumber;
    QString street;
    QString city;
    QString state;
    QString locality;
    QString country;
    QString region;
    QString postalCode;
};

Q_DECLARE_METATYPE(ContactData);

class KJob;
/**
  * This class can be used to retrieve contact fields from the address book based on an email
  * address. It's hides the KDE PIM libraries dependency so it can be made optional.
  */
class KMM_MYMONEY_EXPORT MyMoneyContact : public QObject
{
    Q_OBJECT

public:
    explicit MyMoneyContact(QObject *parent);
    /**
      * Properties of the default identity (the current user).
      */
    bool ownerExists() const;
    QString ownerEmail() const;
    QString ownerFullName() const;

public Q_SLOTS:
    /**
      * Use this slot to start retrieving contact data for an email.
      */
    void fetchContact(const QString &email);

Q_SIGNALS:
    /**
      * This signal is emitted when the contact data was retrieved.
      */
    void contactFetched(const ContactData &identity);

private Q_SLOTS:
    void searchContactResult(KJob *job);
};

#endif // MYMONEYCONTACT_H
