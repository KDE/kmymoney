/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2008 Thomas Baumgart ipwizard @users.sourceforge.net
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif

#include "kbaccountsettings.h"

#include <KMessageBox>
#include <KLocalizedString>

#include "mymoneykeyvaluecontainer.h"
#include "mymoneyaccount.h"

#include "ui_kbaccountsettings.h"

#include <aqbanking/version.h>

struct KBAccountSettings::Private {
    Ui::KBAccountSettings ui;
};

KBAccountSettings::KBAccountSettings(const MyMoneyAccount& /*acc*/,
                                     QWidget* parent) :
    QWidget(parent),
    d(new Private)
{
    d->ui.setupUi(this);
}

KBAccountSettings::~KBAccountSettings()
{
    delete d;
}

void KBAccountSettings::loadUi(const MyMoneyKeyValueContainer& kvp)
{
    d->ui.m_usePayeeAsIsButton->setChecked(true);
    d->ui.m_transactionDownload->setChecked(kvp.value("kbanking-txn-download") != "no");
    d->ui.m_preferredStatementDate->setCurrentIndex(kvp.value("kbanking-statementDate").toInt());
    if (!kvp.value("kbanking-payee-regexp").isEmpty()) {
        d->ui.m_extractPayeeButton->setChecked(true);
        d->ui.m_payeeRegExpEdit->setText(kvp.value("kbanking-payee-regexp"));
        d->ui.m_memoRegExpEdit->setText(kvp.value("kbanking-memo-regexp"));
        d->ui.m_payeeExceptions->clear();
        d->ui.m_payeeExceptions->insertStringList(kvp.value("kbanking-payee-exceptions").split(';', QString::SkipEmptyParts));
    }
    d->ui.m_removeLineBreaksFromMemo->setChecked(kvp.value("kbanking-memo-removelinebreaks").compare(QLatin1String("no")));

    d->ui.m_includePayeeDetails->setChecked(kvp.value("kbanking-memo-includepayeedetails").compare(QLatin1String("no")));
    // don't present the option to the user if it is not available
#if QT_VERSION_CHECK(AQBANKING_VERSION_MAJOR, AQBANKING_VERSION_MINOR, AQBANKING_VERSION_PATCHLEVEL) <= QT_VERSION_CHECK(6, 2, 0)
    d->ui.m_includePayeeDetails->hide();
#endif
}

void KBAccountSettings::loadKvp(MyMoneyKeyValueContainer& kvp)
{
    kvp.deletePair("kbanking-payee-regexp");
    kvp.deletePair("kbanking-memo-regexp");
    kvp.deletePair("kbanking-payee-exceptions");
    kvp.deletePair("kbanking-txn-download");
    kvp.deletePair("kbanking-memo-removelinebreaks");
    kvp.deletePair("kbanking-memo-includepayeedetails");
    // The key "kbanking-jobexec" is not used since version 4.8 anymore
    kvp.deletePair("kbanking-jobexec");

    if (d->ui.m_extractPayeeButton->isChecked()
            && !d->ui.m_payeeRegExpEdit->text().isEmpty()
            && !d->ui.m_memoRegExpEdit->text().isEmpty()) {
        kvp["kbanking-payee-regexp"] = d->ui.m_payeeRegExpEdit->text();
        kvp["kbanking-memo-regexp"] = d->ui.m_memoRegExpEdit->text();
        kvp["kbanking-payee-exceptions"] = d->ui.m_payeeExceptions->items().join(";");
    } else if (d->ui.m_extractPayeeButton->isChecked()) {
        KMessageBox::information(0, i18n("You selected to extract the payee from the memo field but did not supply a regular expression for payee and memo extraction. The option will not be activated."), i18n("Missing information"));
    }
    if (!d->ui.m_transactionDownload->isChecked())
        kvp["kbanking-txn-download"] = "no";

    // remove linebreaks, default is on
    if (!d->ui.m_removeLineBreaksFromMemo->isChecked())
        kvp["kbanking-memo-removelinebreaks"] = "no";

    // include payee details, default is no
    if (!d->ui.m_includePayeeDetails->isChecked())
        kvp["kbanking-memo-includepayeedetails"] = "no";

    kvp["kbanking-statementDate"] = QString("%1").arg(d->ui.m_preferredStatementDate->currentIndex());
}
