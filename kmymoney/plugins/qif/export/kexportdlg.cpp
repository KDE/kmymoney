/*
    SPDX-FileCopyrightText: 2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Javier Campos Morales <javi_c@ctv.es>
    SPDX-FileCopyrightText: 2001 Felix Rodriguez <frodriguez@mail.wesleyan.edu>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kexportdlg.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QLabel>
#include <QPixmap>
#include <QList>
#include <QUrl>
#include <QPushButton>
#include <QIcon>
#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kmessagebox.h>
#include <KConfigGroup>
#include <KGuiItem>
#include <KStandardGuiItem>
#include <KSharedConfig>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneycategory.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneyutils.h"
#include "models.h"
#include "accountsmodel.h"
#include <icons/icons.h>
#include "mymoneyenums.h"
#include "modelenums.h"
#include "../config/mymoneyqifprofile.h"

using namespace Icons;

KExportDlg::KExportDlg(QWidget *parent)
    : KExportDlgDecl(parent)
{
    // Set (almost) all the last used options
    readConfig();

    loadProfiles(true);
    loadAccounts();

    // load button icons
    KGuiItem::assign(m_qbuttonCancel, KStandardGuiItem::cancel());

    KGuiItem okButtenItem(i18n("&Export"),
                          Icons::get(Icon::DocumentExport),
                          i18n("Start operation"),
                          i18n("Use this to start the export operation"));
    KGuiItem::assign(m_qbuttonOk, okButtenItem);

    KGuiItem browseButtenItem(i18n("&Browse..."),
                              Icons::get(Icon::DocumentOpen),
                              i18n("Select filename"),
                              i18n("Use this to select a filename to export to"));
    KGuiItem::assign(m_qbuttonBrowse, browseButtenItem);

    // connect the buttons to their functionality
    connect(m_qbuttonBrowse, &QAbstractButton::clicked, this, &KExportDlg::slotBrowse);
    connect(m_qbuttonOk, &QAbstractButton::clicked, this, &KExportDlg::slotOkClicked);
    connect(m_qbuttonCancel, &QAbstractButton::clicked, this, &QDialog::reject);

    // connect the change signals to the check slot and perform initial check
    connect(m_qlineeditFile, SIGNAL(editingFinished()), this, SLOT(checkData()));
    connect(m_qcheckboxAccount, SIGNAL(toggled(bool)), this, SLOT(checkData()));
    connect(m_qcheckboxCategories, SIGNAL(toggled(bool)), this, SLOT(checkData()));
    connect(m_accountComboBox, SIGNAL(accountSelected(QString)), this, SLOT(checkData(QString)));
    connect(m_profileComboBox, SIGNAL(activated(int)), this, SLOT(checkData()));
    connect(m_kmymoneydateStart, SIGNAL(dateChanged(QDate)), this, SLOT(checkData()));
    connect(m_kmymoneydateEnd, SIGNAL(dateChanged(QDate)), this, SLOT(checkData()));

    checkData(QString());
}

KExportDlg::~KExportDlg()
{
}

void KExportDlg::slotBrowse()
{
    auto newName(QFileDialog::getSaveFileName(this, QString(), QString(), QLatin1String("*.QIF")));
    if (!newName.endsWith(QLatin1String(".qif"), Qt::CaseInsensitive))
        newName.append(QLatin1String(".qif"));
    if (!newName.isEmpty())
        m_qlineeditFile->setText(newName);
}

void KExportDlg::loadProfiles(const bool selectLast)
{
    QString current = m_profileComboBox->currentText();

    m_profileComboBox->clear();

    QStringList list;
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group("Profiles");

    list = grp.readEntry("profiles", QStringList());
    list.sort();
    if (list.isEmpty()) {
        // in case the list is empty, we need to create the default profile
        MyMoneyQifProfile defaultProfile;
        defaultProfile.setProfileDescription(i18n("The default QIF profile"));
        defaultProfile.setProfileName("Profile-Default");

        list += "Default";
        grp.writeEntry("profiles", list);

        defaultProfile.saveProfile();
    }
    m_profileComboBox->insertItems(0, list);

    if (selectLast == true) {
        grp = config->group("Last Use Settings");
        current = grp.readEntry("KExportDlg_LastProfile");
    }

    m_profileComboBox->setCurrentItem(0);
    if (list.contains(current))
        m_profileComboBox->setCurrentIndex(m_profileComboBox->findText(current, Qt::MatchExactly));
}

void KExportDlg::slotOkClicked()
{
    // Make sure we save the last used settings for use next time,
    writeConfig();
    accept();
}

void KExportDlg::readConfig()
{
    KSharedConfigPtr kconfig = KSharedConfig::openConfig();
    KConfigGroup kgrp = kconfig->group("Last Use Settings");
    m_qlineeditFile->setText(kgrp.readEntry("KExportDlg_LastFile"));
    m_qcheckboxAccount->setChecked(kgrp.readEntry("KExportDlg_AccountOpt", true));
    m_qcheckboxCategories->setChecked(kgrp.readEntry("KExportDlg_CatOpt", true));
    m_kmymoneydateStart->setDate(kgrp.readEntry("KExportDlg_StartDate", QDate()));
    m_kmymoneydateEnd->setDate(kgrp.readEntry("KExportDlg_EndDate", QDate()));
    // m_profileComboBox is loaded in loadProfiles(), so we don't worry here
    // m_accountComboBox is loaded in loadAccounts(), so we don't worry here
}

void KExportDlg::writeConfig()
{
    KSharedConfigPtr kconfig = KSharedConfig::openConfig();
    KConfigGroup grp = kconfig->group("Last Use Settings");
    grp.writeEntry("KExportDlg_LastFile", m_qlineeditFile->text());
    grp.writeEntry("KExportDlg_AccountOpt", m_qcheckboxAccount->isChecked());
    grp.writeEntry("KExportDlg_CatOpt", m_qcheckboxCategories->isChecked());
    grp.writeEntry("KExportDlg_StartDate", QDateTime(m_kmymoneydateStart->date()));
    grp.writeEntry("KExportDlg_EndDate", QDateTime(m_kmymoneydateEnd->date()));
    grp.writeEntry("KExportDlg_LastProfile", m_profileComboBox->currentText());
    kconfig->sync();
}

void KExportDlg::checkData(const QString& accountId)
{
    bool  okEnabled = false;

    if (!m_qlineeditFile->text().isEmpty()) {
        auto strFile(m_qlineeditFile->text());
        if (!strFile.endsWith(QLatin1String(".qif"), Qt::CaseInsensitive))
            strFile.append(QLatin1String(".qif"));
        m_qlineeditFile->setText(strFile);
    }

    MyMoneyAccount account;
    if (!accountId.isEmpty()) {
        MyMoneyFile* file = MyMoneyFile::instance();
        account = file->account(accountId);
        if (m_lastAccount != accountId) {
            MyMoneyTransactionFilter filter(accountId);
            QList<MyMoneyTransaction> list = file->transactionList(filter);
            QList<MyMoneyTransaction>::Iterator it;

            if (!list.isEmpty()) {
                it = list.begin();
                m_kmymoneydateStart->loadDate((*it).postDate());
                it = list.end();
                --it;
                m_kmymoneydateEnd->loadDate((*it).postDate());
            }
            m_lastAccount = accountId;
            m_accountComboBox->setSelected(account.id());
        }
    }

    if (!m_qlineeditFile->text().isEmpty() //
            && !m_accountComboBox->getSelected().isEmpty() //
            && !m_profileComboBox->currentText().isEmpty() //
            && m_kmymoneydateStart->date() <= m_kmymoneydateEnd->date() //
            && (m_qcheckboxAccount->isChecked() || m_qcheckboxCategories->isChecked()))
        okEnabled = true;

    m_qbuttonOk->setEnabled(okEnabled);
}

void KExportDlg::loadAccounts()
{
    auto filterProxyModel = new AccountNamesFilterProxyModel(this);
    filterProxyModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability});
    auto const model = Models::instance()->accountsModel();
    filterProxyModel->setSourceColumns(model->getColumns());
    filterProxyModel->setSourceModel(model);
    filterProxyModel->sort((int)eAccountsModel::Column::Account);
    m_accountComboBox->setModel(filterProxyModel);
}

QString KExportDlg::accountId() const
{
    return m_lastAccount;
}
