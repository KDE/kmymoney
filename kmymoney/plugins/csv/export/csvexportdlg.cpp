/*
    SPDX-FileCopyrightText: 2013-2014 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "csvexportdlg.h"
#include "ui_csvexportdlg.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QList>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardPaths>
#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KGuiItem>
#include <KStandardGuiItem>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Headers

#include "accountsmodel.h"
#include "icons.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"

using namespace Icons;

class CsvExportDlgPrivate
{
public:
    CsvExportDlgPrivate(CsvExportDlg* qq)
        : q(qq)
        , ui(new Ui::CsvExportDlg)
        , accountsModel(new AccountNamesFilterProxyModel(qq))
    {
        ui->setupUi(qq);
        ui->infoMessage->hide();
        ui->m_accountComboBox->setSplitActionVisible(false);
        loadSeparator();
        loadAccounts();
        loadIcons();
        readConfig();
    }

    ~CsvExportDlgPrivate() = default;

    void loadSeparator()
    {
        // load separator combobox
        QList<QPair<KLocalizedString, QString>> separatorList = {
            {ki18nc("@item:inlistbox CSV separator", "Comma (,)"), ","},
            {ki18nc("@item:inlistbox CSV separator", "Semicolon (;)"), ";"},
            {ki18nc("@item:inlistbox CSV separator", "Tab (\\t)"), "\t"},
        };
        for (const auto& separatorItem : separatorList) {
            ui->m_separatorComboBox->addItem(separatorItem.first.toString(), separatorItem.second);
        }
    }

    void loadAccounts()
    {
        accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type>{
            eMyMoney::Account::Type::Asset,
            eMyMoney::Account::Type::Liability,
        });
        accountsModel->setHideEquityAccounts(true);
        accountsModel->setSourceModel(MyMoneyFile::instance()->accountsModel());
        accountsModel->sort(AccountsModel::Column::AccountName);
        ui->m_accountComboBox->setModel(accountsModel);
        ui->m_accountComboBox->clearSelection();
    }

    void loadIcons()
    {
        // load button icons
        KGuiItem::assign(ui->m_qbuttonCancel, KStandardGuiItem::cancel());

        KGuiItem okButtonItem(i18n("&Export"), Icons::get(Icon::DocumentExport), i18n("Start operation"), i18n("Use this to start the export operation"));
        KGuiItem::assign(ui->m_qbuttonOk, okButtonItem);

        KGuiItem browseButtonItem(i18n("&Browse..."),
                                  Icons::get(Icon::DocumentOpen),
                                  i18n("Select filename"),
                                  i18n("Use this to select a filename to export to"));
        KGuiItem::assign(ui->m_qbuttonBrowse, browseButtonItem);
    }

    void readConfig()
    {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::ConfigLocation, QLatin1String("csvexporterrc")));
        KConfigGroup conf = config->group("Last Use Settings");
        ui->m_qlineeditFile->setText(conf.readEntry("CsvExportDlg_LastFile"));
        ui->m_radioButtonAccount->setChecked(conf.readEntry("CsvExportDlg_AccountOpt", true));
        ui->m_radioButtonCategories->setChecked(conf.readEntry("CsvExportDlg_CatOpt", true));
        ui->m_kmymoneydateStart->setDate(conf.readEntry("CsvExportDlg_StartDate", QDate()));
        ui->m_kmymoneydateEnd->setDate(conf.readEntry("CsvExportDlg_EndDate", QDate()));
        ui->m_separatorComboBox->setCurrentIndex(conf.readEntry("CsvExportDlg_separatorIndex", -1));
    }

    void writeConfig()
    {
        KSharedConfigPtr config = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/csvexporterrc"));
        KConfigGroup grp = config->group("Last Use Settings");
        grp.writeEntry("CsvExportDlg_LastFile", ui->m_qlineeditFile->text());
        grp.writeEntry("CsvExportDlg_AccountOpt", ui->m_radioButtonAccount->isChecked());
        grp.writeEntry("CsvExportDlg_CatOpt", ui->m_radioButtonCategories->isChecked());
        grp.writeEntry("CsvExportDlg_StartDate", ui->m_kmymoneydateStart->date().startOfDay());
        grp.writeEntry("CsvExportDlg_EndDate", ui->m_kmymoneydateEnd->date().startOfDay());
        grp.writeEntry("CsvExportDlg_separatorIndex", ui->m_separatorComboBox->currentIndex());
        config->sync();
    }

    /**
     * This method checks whether all data is correct to enable
     * the 'Export' button. The enable state of the 'Export' button
     * is updated appropriately.
     *
     * If the parameter @p account is not empty, then it is assumed
     * a new account is selected and the date fields will be loaded
     * with the date of the first and last transaction within this
     * account.
     *
     * @param account The name of the selected account.
     */
    void checkData()
    {
        // make sure file has correct file type
        if (!ui->m_qlineeditFile->text().isEmpty()) {
            auto strFile(ui->m_qlineeditFile->text());
            if (!strFile.endsWith(QLatin1String(".csv"), Qt::CaseInsensitive))
                strFile.append(QLatin1String(".csv"));
            ui->m_qlineeditFile->setText(strFile);
        }

        bool okEnabled = (!ui->m_qlineeditFile->text().isEmpty() && !ui->m_accountComboBox->getSelected().isEmpty()
                          && ui->m_kmymoneydateStart->date() <= ui->m_kmymoneydateEnd->date()
                          && (ui->m_radioButtonAccount->isChecked() || ui->m_radioButtonCategories->isChecked())
                          && (ui->m_separatorComboBox->currentIndex() >= 0) && ui->infoMessage->isHidden());
        ui->m_qbuttonOk->setEnabled(okEnabled);
    }

    void checkData(const QString& accountId)
    {
        if (!accountId.isEmpty()) {
            QDate earliestDate(QDate(2500, 01, 01));
            QDate latestDate(QDate(1900, 01, 01));
            QList<MyMoneyTransaction> transactions;
            MyMoneyAccount account;
            MyMoneyFile* file = MyMoneyFile::instance();
            bool foundTransactions(false);

            account = file->account(accountId);

            if (account.accountType() == eMyMoney::Account::Type::Investment) {
                //  If this is Investment account, we need to check all child accounts.
                for (const auto& stockAccount : account.accountList()) {
                    MyMoneyTransactionFilter filter(stockAccount);
                    file->transactionList(transactions, filter);
                    if (!transactions.isEmpty()) {
                        foundTransactions = true;
                        if (transactions[0].postDate() < earliestDate) {
                            earliestDate = transactions[0].postDate();
                        }
                        if (transactions[transactions.count() - 1].postDate() > latestDate) {
                            latestDate = transactions[transactions.count() - 1].postDate();
                        }
                    }
                }
            } else { // all other account types
                MyMoneyTransactionFilter filter(account.id());
                file->transactionList(transactions, filter);
                if (!transactions.isEmpty()) {
                    foundTransactions = true;
                    earliestDate = transactions[0].postDate();
                    latestDate = transactions[transactions.count() - 1].postDate();
                }
            }
            ui->m_kmymoneydateStart->setDate(earliestDate);
            ui->m_kmymoneydateEnd->setDate(latestDate);

            if (ui->infoMessage->isVisible() && foundTransactions) {
                ui->infoMessage->animatedHide();

            } else if (!ui->infoMessage->isVisible() && !foundTransactions) {
                ui->infoMessage->setText(i18nc("@info Hint in CSV export dialog", "Current selected account does not contain any transactions."));
                ui->infoMessage->animatedShow();
            }
        }
        checkData();
    }

    CsvExportDlg* q;
    Ui::CsvExportDlg* ui;
    AccountNamesFilterProxyModel* accountsModel;
};

CsvExportDlg::CsvExportDlg(QWidget* parent)
    : QDialog(parent)
    , d_ptr(new CsvExportDlgPrivate(this))
{
    Q_D(CsvExportDlg);

    // connect the buttons to their functionality
    connect(d->ui->m_qbuttonBrowse, &QPushButton::clicked, this, [&]() {
        Q_D(CsvExportDlg);
        QString fileName(QFileDialog::getSaveFileName(this, QString(), QString(), QLatin1String("*.CSV")));
        if (!fileName.isEmpty()) {
            d->ui->m_qlineeditFile->setText(fileName);
            d->checkData();
        }
    });

    connect(d->ui->m_qbuttonOk, &QPushButton::clicked, this, [&]() {
        Q_D(CsvExportDlg);
        d->writeConfig();
        accept();
    });
    connect(d->ui->m_qbuttonCancel, &QPushButton::clicked, this, &QDialog::reject);

    // connect the change signals to the check slot and perform initial check
    connect(d->ui->m_qlineeditFile, &QLineEdit::editingFinished, this, [&] {
        d->checkData();
    });
    connect(d->ui->m_radioButtonAccount, &QRadioButton::toggled, this, [&] {
        d->checkData();
    });
    connect(d->ui->m_radioButtonCategories, &QRadioButton::toggled, this, [&] {
        d->checkData();
    });
    connect(d->ui->m_accountComboBox, &KMyMoneyAccountCombo::accountSelected, this, [&](const QString& accountId) {
        Q_D(CsvExportDlg);
        d->checkData(accountId);
    });

    d->checkData(QString());
}

CsvExportDlg::~CsvExportDlg() noexcept
{
}

QString CsvExportDlg::filename() const
{
    Q_D(const CsvExportDlg);
    return d->ui->m_qlineeditFile->text();
};

QDate CsvExportDlg::startDate() const
{
    Q_D(const CsvExportDlg);
    return d->ui->m_kmymoneydateStart->date();
};

QDate CsvExportDlg::endDate() const
{
    Q_D(const CsvExportDlg);
    return d->ui->m_kmymoneydateEnd->date();
};

bool CsvExportDlg::accountSelected() const
{
    Q_D(const CsvExportDlg);
    return d->ui->m_radioButtonAccount->isChecked();
};

bool CsvExportDlg::categorySelected() const
{
    Q_D(const CsvExportDlg);
    return d->ui->m_radioButtonCategories->isChecked();
};

QString CsvExportDlg::accountId() const
{
    Q_D(const CsvExportDlg);
    return d->ui->m_accountComboBox->getSelected();
};

QString CsvExportDlg::separator() const
{
    Q_D(const CsvExportDlg);
    return d->ui->m_separatorComboBox->itemData(d->ui->m_separatorComboBox->currentIndex()).toString();
};
