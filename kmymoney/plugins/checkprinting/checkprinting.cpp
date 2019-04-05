/***************************************************************************
 *   This file is part of KMyMoney, A Personal Finance Manager by KDE      *
 *                                                                         *
 *   Copyright (C) 2009      Cristian Onet <onet.cristian@gmail.com>       *
 *   Copyright (C) 2019      Thomas Baumgart <tbaumgart@kde.org>           *
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

#include <config-kmymoney.h>
#include "checkprinting.h"

// QT includes
#include <QAction>
#include <QFile>
#include <QDialog>
#ifdef ENABLE_WEBENGINE
#include <QWebEngineView>
#else
#include <KWebView>
#endif
#include <QStandardPaths>

// KDE includes
#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>

// KMyMoney includes
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyinstitution.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyutils.h"
#include "viewinterface.h"
#include "selectedtransactions.h"

#include "numbertowords.h"
#include "pluginsettings.h"
#include "mymoneyenums.h"

#ifdef IS_APPIMAGE
#include <QCoreApplication>
#include <QStandardPaths>
#endif
#include "kmm_printer.h"

struct CheckPrinting::Private {
  QAction* m_action;
  QString  m_checkTemplateHTML;
  QStringList m_printedTransactionIdList;
  KMyMoneyRegister::SelectedTransactions m_transactions;
};

CheckPrinting::CheckPrinting(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "checkprinting"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  Q_UNUSED(args);
  const auto componentName = QLatin1String("checkprinting");
  const auto rcFileName = QLatin1String("checkprinting.rc");
  // Tell the host application to load my GUI component
  setComponentName(componentName, i18nc("It's about printing bank checks", "Check printing"));

#ifdef IS_APPIMAGE
  const QString rcFilePath = QCoreApplication::applicationDirPath() + QLatin1String("/../share/kxmlgui5/") + componentName + QLatin1Char('/') + rcFileName;
  setXMLFile(rcFilePath);

  const QString localRcFilePath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + QLatin1Char('/') + componentName + QLatin1Char('/') + rcFileName;
  setLocalXMLFile(localRcFilePath);
#else
  setXMLFile(rcFileName);
#endif

  // For ease announce that we have been loaded.
  qDebug("Plugins: checkprinting loaded");

  d = std::unique_ptr<Private>(new Private);

  // Create the actions of this plugin
  QString actionName = i18n("Print check");

  d->m_action = actionCollection()->addAction("transaction_checkprinting", this, SLOT(slotPrintCheck()));
  d->m_action->setText(actionName);

  // wait until a transaction is selected before enabling the action
  d->m_action->setEnabled(false);
  d->m_printedTransactionIdList = PluginSettings::printedChecks();
  readCheckTemplate();

  //! @todo Christian: Replace
#if 0
  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(configChanged(Plugin*)), this, SLOT(slotUpdateConfig()));
#endif
}

/**
 * @internal Destructor is needed because destructor call of unique_ptr must be in this compile unit
 */
CheckPrinting::~CheckPrinting()
{
  qDebug("Plugins: checkprinting unloaded");
}

void CheckPrinting::plug()
{
  connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::transactionsSelected, this, &CheckPrinting::slotTransactionsSelected);
}

void CheckPrinting::unplug()
{
  disconnect(viewInterface(), &KMyMoneyPlugin::ViewInterface::transactionsSelected, this, &CheckPrinting::slotTransactionsSelected);
}

void CheckPrinting::readCheckTemplate()
{
  QString checkTemplateHTMLPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "checkprinting/check_template.html");

  if (PluginSettings::checkTemplateFile().isEmpty()) {
    PluginSettings::setCheckTemplateFile(checkTemplateHTMLPath);
    PluginSettings::self()->save();
  }

  QFile checkTemplateHTMLFile(PluginSettings::checkTemplateFile());
  checkTemplateHTMLFile.open(QIODevice::ReadOnly);

  QTextStream stream(&checkTemplateHTMLFile);

  d->m_checkTemplateHTML = stream.readAll();

  checkTemplateHTMLFile.close();
}

bool CheckPrinting::canBePrinted(const KMyMoneyRegister::SelectedTransaction & selectedTransaction) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  bool isACheck = file->account(selectedTransaction.split().accountId()).accountType() == eMyMoney::Account::Type::Checkings && selectedTransaction.split().shares().isNegative();

  return isACheck && d->m_printedTransactionIdList.contains(selectedTransaction.transaction().id()) == 0;
}

void CheckPrinting::markAsPrinted(const KMyMoneyRegister::SelectedTransaction & selectedTransaction)
{
  d->m_printedTransactionIdList.append(selectedTransaction.transaction().id());
}

void CheckPrinting::slotPrintCheck()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyMoneyToWordsConverter converter;
  #ifdef ENABLE_WEBENGINE
  auto htmlPart = new QWebEngineView();
  #else
  auto htmlPart = new KWebView();
  #endif

  KMyMoneyRegister::SelectedTransactions::const_iterator it;
  for (it = d->m_transactions.constBegin(); it != d->m_transactions.constEnd(); ++it) {
    if (!canBePrinted(*it))
      continue; // skip this check since it was already printed

    QString checkHTML = d->m_checkTemplateHTML;
    const MyMoneyAccount account = file->account((*it).split().accountId());
    const MyMoneySecurity currency = file->currency(account.currencyId());
    const MyMoneyInstitution institution = file->institution(file->account((*it).split().accountId()).institutionId());

    // replace the predefined tokens
    // data about the user
    checkHTML.replace("$OWNER_NAME", file->user().name());
    checkHTML.replace("$OWNER_ADDRESS", file->user().address());
    checkHTML.replace("$OWNER_CITY", file->user().city());
    checkHTML.replace("$OWNER_STATE", file->user().state());
    // data about the account institution
    checkHTML.replace("$INSTITUTION_NAME", institution.name());
    checkHTML.replace("$INSTITUTION_STREET", institution.street());
    checkHTML.replace("$INSTITUTION_TELEPHONE", institution.telephone());
    checkHTML.replace("$INSTITUTION_TOWN", institution.town());
    checkHTML.replace("$INSTITUTION_CITY", institution.city());
    checkHTML.replace("$INSTITUTION_POSTCODE", institution.postcode());
    checkHTML.replace("$INSTITUTION_MANAGER", institution.manager());
    // data about the transaction
    checkHTML.replace("$DATE", QLocale().toString((*it).transaction().postDate(), QLocale::ShortFormat));
    checkHTML.replace("$CHECK_NUMBER", (*it).split().number());
    checkHTML.replace("$PAYEE_NAME", file->payee((*it).split().payeeId()).name());
    checkHTML.replace("$PAYEE_ADDRESS", file->payee((*it).split().payeeId()).address());
    checkHTML.replace("$PAYEE_CITY", file->payee((*it).split().payeeId()).city());
    checkHTML.replace("$PAYEE_POSTCODE", file->payee((*it).split().payeeId()).postcode());
    checkHTML.replace("$PAYEE_STATE", file->payee((*it).split().payeeId()).state());
    checkHTML.replace("$AMOUNT_STRING", converter.convert((*it).split().shares().abs(), currency.smallestAccountFraction()));
    checkHTML.replace("$AMOUNT_DECIMAL", MyMoneyUtils::formatMoney((*it).split().shares().abs(), currency));
    checkHTML.replace("$MEMO", (*it).split().memo());

    // print the check
    htmlPart->setHtml(checkHTML, QUrl("file://"));
    auto printer = KMyMoneyPrinter::startPrint();
    if (printer != nullptr) {
      #ifdef ENABLE_WEBENGINE
        htmlPart->page()->print(printer, [=] (bool) {});
      #else
        htmlPart->print(printer);
      #endif
    }

    // mark the transaction as printed
    markAsPrinted(*it);
  }

  PluginSettings::setPrintedChecks(d->m_printedTransactionIdList);
  delete htmlPart;
}

void CheckPrinting::slotTransactionsSelected(const KMyMoneyRegister::SelectedTransactions& transactions)
{
  d->m_transactions = transactions;
  bool actionEnabled = false;
  // enable/disable the action depending if there are transactions selected or not
  // and whether they can be printed or not
  KMyMoneyRegister::SelectedTransactions::const_iterator it;
  for (it = d->m_transactions.constBegin(); it != d->m_transactions.constEnd(); ++it) {
    if (canBePrinted(*it)) {
      actionEnabled = true;
      break;
    }
  }
  d->m_action->setEnabled(actionEnabled);
}

// the plugin's configurations has changed
void CheckPrinting::configurationChanged()
{
  PluginSettings::self()->load();
  // re-read the data because the configuration has changed
  readCheckTemplate();
  d->m_printedTransactionIdList = PluginSettings::printedChecks();
}

K_PLUGIN_FACTORY_WITH_JSON(CheckPrintingFactory, "checkprinting.json", registerPlugin<CheckPrinting>();)

#include "checkprinting.moc"
