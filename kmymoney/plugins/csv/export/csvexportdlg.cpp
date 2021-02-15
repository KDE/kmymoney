/*
 * SPDX-FileCopyrightText: 2013-2014 Allan Anderson <agander93@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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

#include <KMessageBox>
#include <KGuiItem>
#include <KStandardGuiItem>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "icons/icons.h"
#include "mymoneyenums.h"

using namespace Icons;

CsvExportDlg::CsvExportDlg(QWidget *parent) : QDialog(parent), ui(new Ui::CsvExportDlg)
{
  ui->setupUi(this);
  m_fieldDelimiterCharList << "," << ";" << "\t";
  ui->m_separatorComboBox->setCurrentIndex(-1);

  // Set (almost) all the last used options
  readConfig();

  loadAccounts();

  // load button icons
  KGuiItem::assign(ui->m_qbuttonCancel, KStandardGuiItem::cancel());

  KGuiItem okButtonItem(i18n("&Export"),
                        Icons::get(Icon::DocumentExport),
                        i18n("Start operation"),
                        i18n("Use this to start the export operation"));
  KGuiItem::assign(ui->m_qbuttonOk, okButtonItem);

  KGuiItem browseButtonItem(i18n("&Browse..."),
                            Icons::get(Icon::DocumentOpen),
                            i18n("Select filename"),
                            i18n("Use this to select a filename to export to"));
  KGuiItem::assign(ui->m_qbuttonBrowse, browseButtonItem);

  // connect the buttons to their functionality
  connect(ui->m_qbuttonBrowse, SIGNAL(clicked()), this, SLOT(slotBrowse()));
  connect(ui->m_qbuttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(ui->m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

  // connect the change signals to the check slot and perform initial check
  connect(ui->m_qlineeditFile, SIGNAL(editingFinished()), this, SLOT(checkData()));
  connect(ui->m_radioButtonAccount, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(ui->m_radioButtonCategories, SIGNAL(toggled(bool)), this, SLOT(checkData()));
  connect(ui->m_accountComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(checkData(QString)));
  connect(ui->m_separatorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(separator(int)));
  connect(ui->m_separatorComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(checkData()));

  checkData(QString());
}

CsvExportDlg::~CsvExportDlg()
{
}

void CsvExportDlg::slotBrowse()
{
  QString newName(QFileDialog::getSaveFileName(this, QString(), QString(), QLatin1String("*.CSV")));
  if (newName.indexOf('.') == -1)
    newName += QLatin1String(".csv");
  if (!newName.isEmpty())
    ui->m_qlineeditFile->setText(newName);
}

void CsvExportDlg::slotOkClicked()
{
  // Make sure we save the last used settings for use next time,
  writeConfig();
  accept();
}

void CsvExportDlg::separator(int separatorIndex)
{
  m_separator = m_fieldDelimiterCharList[separatorIndex];
}

void CsvExportDlg::readConfig()
{
  KSharedConfig::Ptr config = KSharedConfig::openConfig(QStandardPaths::locate(QStandardPaths::ConfigLocation, QLatin1String("csvexporterrc")));
  KConfigGroup conf = config->group("Last Use Settings");
  ui->m_qlineeditFile->setText(conf.readEntry("CsvExportDlg_LastFile"));
  ui->m_radioButtonAccount->setChecked(conf.readEntry("CsvExportDlg_AccountOpt", true));
  ui->m_radioButtonCategories->setChecked(conf.readEntry("CsvExportDlg_CatOpt", true));
  ui->m_kmymoneydateStart->setDate(conf.readEntry("CsvExportDlg_StartDate", QDate()));
  ui->m_kmymoneydateEnd->setDate(conf.readEntry("CsvExportDlg_EndDate", QDate()));
}

void CsvExportDlg::writeConfig()
{
  KSharedConfigPtr config = KSharedConfig::openConfig(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/csvexporterrc"));
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("CsvExportDlg_LastFile", ui->m_qlineeditFile->text());
  grp.writeEntry("CsvExportDlg_AccountOpt", ui->m_radioButtonAccount->isChecked());
  grp.writeEntry("CsvExportDlg_CatOpt", ui->m_radioButtonCategories->isChecked());
  grp.writeEntry("CsvExportDlg_StartDate", QDateTime(ui->m_kmymoneydateStart->date()));
  grp.writeEntry("CsvExportDlg_EndDate", QDateTime(ui->m_kmymoneydateEnd->date()));
  grp.writeEntry("CsvExportDlg_separatorIndex", ui->m_separatorComboBox->currentIndex());
  config->sync();
}

void CsvExportDlg::checkData(const QString& accountName)
{
  bool  okEnabled = false;

  if (!ui->m_qlineeditFile->text().isEmpty()) {
    auto strFile(ui->m_qlineeditFile->text());
    if (!strFile.endsWith(QLatin1String(".csv"), Qt::CaseInsensitive))
      strFile.append(QLatin1String(".csv"));
    ui->m_qlineeditFile->setText(strFile);
  }
  QDate earliestDate(QDate(2500, 01, 01));
  QDate latestDate(QDate(1900, 01, 01));
  QList<MyMoneyTransaction> listTrans;
  QList<MyMoneyTransaction>::Iterator itTrans;
  MyMoneyAccount account;
  MyMoneyFile* file = MyMoneyFile::instance();

  if (!accountName.isEmpty()) {
    account = file->accountByName(accountName);
    m_accountId = account.id();
    MyMoneyAccount accnt;
    if (account.accountType() == eMyMoney::Account::Type::Investment) {
      //  If this is Investment account, we need child account.
      foreach (const auto sAccount, account.accountList()) {
        accnt = file->account(sAccount);
        MyMoneyTransactionFilter filter(accnt.id());
        listTrans = file->transactionList(filter);
        if (!listTrans.isEmpty()) {
          if (listTrans[0].postDate() < earliestDate) {
            earliestDate = listTrans[0].postDate();
          }
          latestDate = listTrans[listTrans.count() - 1].postDate();
        }
      }
    } else {  //Checking, etc.
      MyMoneyTransactionFilter filter(account.id());
      listTrans = file->transactionList(filter);
      if (listTrans.isEmpty()) {
        KMessageBox::sorry(0, i18n("There are no entries in this account.\n"),
                           i18n("Invalid account"));
        return;
      }
      earliestDate = listTrans[0].postDate();
      latestDate = listTrans[listTrans.count() - 1].postDate();
    }
    ui->m_kmymoneydateStart->setDate(earliestDate);
    ui->m_kmymoneydateEnd->setDate(latestDate);
    ui->m_accountComboBox->setCompletedText(accnt.id());
  }

  if (!ui->m_qlineeditFile->text().isEmpty()
      && !ui->m_accountComboBox->currentText().isEmpty()
      && ui->m_kmymoneydateStart->date() <= ui->m_kmymoneydateEnd->date()
      && (ui->m_radioButtonAccount->isChecked() || ui->m_radioButtonCategories->isChecked())
      && (ui->m_separatorComboBox->currentIndex() >= 0)) {
    okEnabled = true;
  }
  ui->m_qbuttonOk->setEnabled(okEnabled);
}

void CsvExportDlg::loadAccounts()
{
  QStringList lst = getAccounts();
  for (int i = 0; i < lst.count(); i++) {
    ui->m_accountComboBox->addItem(lst[i]);
  }
  ui->m_accountComboBox->setCurrentIndex(-1);
}

QStringList CsvExportDlg::getAccounts()
{
  QStringList list;
  MyMoneyFile* file = MyMoneyFile::instance();
  // Get a list of all accounts
  QList<MyMoneyAccount> accounts;
  file->accountList(accounts);
  QList<MyMoneyAccount>::const_iterator it_account = accounts.constBegin();
  m_idList.clear();
  while (it_account != accounts.constEnd()) {
    MyMoneyAccount account((*it_account).id(), (*it_account));
    if (!account.isClosed()) {
      eMyMoney::Account::Type accntType = account.accountType();
      eMyMoney::Account::Type accntGroup = account.accountGroup();
      if ((accntGroup == eMyMoney::Account::Type::Liability)  || ((accntGroup == eMyMoney::Account::Type::Asset) && (accntType != eMyMoney::Account::Type::Stock))) {  //  ie Asset or Liability types
        list << account.name();
        m_idList << account.id();
      }
    }
    ++it_account;
  }
  qSort(list.begin(), list.end(), caseInsensitiveLessThan);
  return list;
}

void CsvExportDlg::slotStatusProgressBar(int current, int total)
{
  if (total == -1 && current == -1) {     // reset
    ui->progressBar->setValue(ui->progressBar->maximum());
  } else if (total != 0) {                // init
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(0);
    ui->progressBar->show();
  } else {                                // update
    ui->progressBar->setValue(current);
  }
  update();
}

bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
  return s1.toLower() < s2.toLower();
}
