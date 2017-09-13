/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
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

#include "printcheck.h"

#include "config-kmymoney.h"

// QT includes
#include <QAction>
#ifdef ENABLE_WEBENGINE
#include <QWebEngineView>
#else
#include <KDEWebKit/KWebView>
#endif
#include <QtPrintSupport/QPrintDialog>

// KDE includes
#include <KActionCollection>
#include <KPluginInfo>
#include <QStandardPaths>
#include <KLocalizedString>

// KMyMoney includes
#include "mymoneyfile.h"
#include "pluginloader.h"

#include "numbertowords.h"
#include "pluginsettings.h"

struct KMMPrintCheckPlugin::Private {
  QAction* m_action;
  QString  m_checkTemplateHTML;
  QStringList m_printedTransactionIdList;
  KMyMoneyRegister::SelectedTransactions m_transactions;
};

KMMPrintCheckPlugin::KMMPrintCheckPlugin()
    : KMyMoneyPlugin::Plugin(nullptr, "Print check"/*must be the same as X-KDE-PluginInfo-Name*/)
{
  // Tell the host application to load my GUI component
  setComponentName("kmm_printcheck", i18n("Print check"));
  setXMLFile("kmm_printcheck.rc");

  // For ease announce that we have been loaded.
  qDebug("KMyMoney printcheck plugin loaded");

  d = std::unique_ptr<Private>(new Private);

  // Create the actions of this plugin
  QString actionName = i18n("Print check");

  d->m_action = actionCollection()->addAction("transaction_printcheck", this, SLOT(slotPrintCheck()));
  d->m_action->setText(actionName);

  // wait until a transaction is selected before enableing the action
  d->m_action->setEnabled(false);
  d->m_printedTransactionIdList = PluginSettings::printedChecks();
  readCheckTemplate();

  //! @todo Christian: Replace
#if 0
  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(plug(KPluginInfo*)), this, SLOT(slotPlug(KPluginInfo*)));
  connect(KMyMoneyPlugin::PluginLoader::instance(), SIGNAL(configChanged(Plugin*)), this, SLOT(slotUpdateConfig()));
#endif
}

/**
 * @internal Destructor is needed because destructor call of unique_ptr must be in this compile unit
 */
KMMPrintCheckPlugin::~KMMPrintCheckPlugin()
{
}

void KMMPrintCheckPlugin::readCheckTemplate()
{
  QString checkTemplateHTMLPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kmm_printcheck/check_template.html");

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

bool KMMPrintCheckPlugin::canBePrinted(const KMyMoneyRegister::SelectedTransaction & selectedTransaction) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  bool isACheck = file->account(selectedTransaction.split().accountId()).accountType() == MyMoneyAccount::Checkings && selectedTransaction.split().shares().isNegative();

  return isACheck && d->m_printedTransactionIdList.contains(selectedTransaction.transaction().id()) == 0;
}

void KMMPrintCheckPlugin::markAsPrinted(const KMyMoneyRegister::SelectedTransaction & selectedTransaction)
{
  d->m_printedTransactionIdList.append(selectedTransaction.transaction().id());
}

void KMMPrintCheckPlugin::slotPrintCheck()
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
    m_currentPrinter = new QPrinter();
    QPointer<QPrintDialog> dialog = new QPrintDialog(m_currentPrinter);
    dialog->setWindowTitle(QString());
    if (dialog->exec() != QDialog::Accepted) {
      delete m_currentPrinter;
      m_currentPrinter = nullptr;
      continue;
    } else {
      #ifdef ENABLE_WEBENGINE
        htmlPart->page()->print(m_currentPrinter, [=] (bool) {delete m_currentPrinter; m_currentPrinter = nullptr;});
      #else
        htmlPart->print(m_currentPrinter);
      #endif
    }
    delete dialog;

    // mark the transaction as printed
    markAsPrinted(*it);
  }

  PluginSettings::setPrintedChecks(d->m_printedTransactionIdList);
  delete htmlPart;
}

void KMMPrintCheckPlugin::slotTransactionsSelected(const KMyMoneyRegister::SelectedTransactions& transactions)
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

// the plugin loader plugs in a plugin
void KMMPrintCheckPlugin::slotPlug(KPluginInfo *info)
{
  if (info->name() == objectName()) {
    connect(viewInterface(), SIGNAL(transactionsSelected(KMyMoneyRegister::SelectedTransactions)),
            this, SLOT(slotTransactionsSelected(KMyMoneyRegister::SelectedTransactions)));
  }
}

// the plugin's configurations has changed
void KMMPrintCheckPlugin::slotUpdateConfig()
{
  PluginSettings::self()->load();
  // re-read the data because the configuration has changed
  readCheckTemplate();
  d->m_printedTransactionIdList = PluginSettings::printedChecks();
}
