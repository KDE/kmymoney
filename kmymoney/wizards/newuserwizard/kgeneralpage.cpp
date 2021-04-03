/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgeneralpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "userinfo.h"
#include "ui_userinfo.h"

#include "knewuserwizard.h"
#include "knewuserwizard_p.h"

#include "mymoneycontact.h"
#include "kcurrencypage.h"

class KMyMoneyWizardPage;

namespace NewUserWizard
{
class GeneralPagePrivate : public WizardPagePrivate<Wizard>
{
    Q_DISABLE_COPY(GeneralPagePrivate)

public:
    GeneralPagePrivate(QObject* parent) :
        WizardPagePrivate<Wizard>(parent),
        m_contact(nullptr)
    {
    }

    MyMoneyContact *m_contact;
};

GeneralPage::GeneralPage(Wizard* wizard) :
    UserInfo(wizard),
    WizardPage<Wizard>(*new GeneralPagePrivate(wizard), stepCount++, this, wizard)
{
    Q_D(GeneralPage);
    d->m_contact = new MyMoneyContact(this);

    ui->m_loadAddressButton->setEnabled(d->m_contact->ownerExists());
    connect(ui->m_loadAddressButton, &QAbstractButton::clicked, this, &GeneralPage::slotLoadFromAddressBook);
}

GeneralPage::~GeneralPage()
{
}

void GeneralPage::enterPage()
{
    ui->m_userNameEdit->setFocus();
}


void GeneralPage::slotLoadFromAddressBook()
{
    Q_D(GeneralPage);
    ui->m_userNameEdit->setText(d->m_contact->ownerFullName());
    ui->m_emailEdit->setText(d->m_contact->ownerEmail());
    if (ui->m_emailEdit->text().isEmpty()) {
        KMessageBox::sorry(this, i18n("Unable to load data, because no contact has been associated with the owner of the standard address book."), i18n("Address book import"));
        return;
    }
    ui->m_loadAddressButton->setEnabled(false);
    connect(d->m_contact, &MyMoneyContact::contactFetched, this, &GeneralPage::slotContactFetched);
    d->m_contact->fetchContact(ui->m_emailEdit->text());
}

void GeneralPage::slotContactFetched(const ContactData &identity)
{
    ui->m_loadAddressButton->setEnabled(true);
    if (identity.email.isEmpty())
        return;
    ui->m_telephoneEdit->setText(identity.phoneNumber);
    QString sep;
    if (!identity.country.isEmpty() && !identity.region.isEmpty())
        sep = " / ";
    ui->m_countyEdit->setText(QString("%1%2%3").arg(identity.country, sep, identity.region));
    ui->m_postcodeEdit->setText(identity.postalCode);
    ui->m_townEdit->setText(identity.locality);
    ui->m_streetEdit->setText(identity.street);
}

KMyMoneyWizardPage* GeneralPage::nextPage() const
{
    Q_D(const GeneralPage);
    return d->m_wizard->d_func()->m_currencyPage;
}

}
