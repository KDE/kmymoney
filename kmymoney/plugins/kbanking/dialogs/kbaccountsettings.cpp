/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2008  Thomas Baumgart ipwizard@users.sourceforge.net        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif

#include "kbaccountsettings.h"

#include <KMessageBox>

#include "mymoneykeyvaluecontainer.h"
#include "mymoneyaccount.h"

#include "ui_kbaccountsettings.h"

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
  QStringList val = kvp.value("kbanking-acc-ref").split("-");
  d->ui.m_mappedBank->setText(val.size() > 0 ? val.at(0) : "");
  d->ui.m_mappedAccount->setText(val.size() > 1 ? val.at(1) : "");
}

void KBAccountSettings::loadKvp(MyMoneyKeyValueContainer& kvp)
{
  kvp.deletePair("kbanking-payee-regexp");
  kvp.deletePair("kbanking-memo-regexp");
  kvp.deletePair("kbanking-payee-exceptions");
  kvp.deletePair("kbanking-txn-download");
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
  kvp["kbanking-statementDate"] = QString("%1").arg(d->ui.m_preferredStatementDate->currentIndex());
}
