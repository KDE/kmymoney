/***************************************************************************
                          mymoneyqifreader.cpp
                             -------------------
    begin                : Mon Jan 27 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyqifreader.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QFile>
#include <QStringList>
#include <QTimer>
#include <QRegExp>
#include <QBuffer>
#include <QByteArray>
#include <QInputDialog>
#include <QDir>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kmessagebox.h>
#include <kconfig.h>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include "kjobwidgets.h"
#include "kio/job.h"

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneyexception.h"
#include "kmymoneysettings.h"

#include "mymoneystatement.h"

// define this to debug the code. Using external filters
// while debugging did not work too good for me, so I added
// this code.
// #define DEBUG_IMPORT

#ifdef DEBUG_IMPORT
#ifdef __GNUC__
#warning "DEBUG_IMPORT defined --> external filter not available!!!!!!!"
#endif
#endif

class MyMoneyQifReader::Private
{
public:
  Private() :
      accountType(eMyMoney::Account::Type::Checkings),
      firstTransaction(true),
      mapCategories(true),
      transactionType(MyMoneyQifReader::QifEntryTypeE::EntryUnknown)
  {}

  const QString accountTypeToQif(eMyMoney::Account::Type type) const;

  /**
   * finalize the current statement and add it to the statement list
   */
  void finishStatement();

  bool isTransfer(QString& name, const QString& leftDelim, const QString& rightDelim);

  /**
   * Converts the QIF specific N-record of investment transactions into
   * a category name
   */
  const QString typeToAccountName(const QString& type) const;

  /**
   * Converts the QIF reconcile state to the KMyMoney reconcile state
   */
  eMyMoney::Split::State reconcileState(const QString& state) const;

  /**
    */
  void fixMultiLineMemo(QString& memo) const;

public:
  /**
   * the statement that is currently collected/processed
   */
  MyMoneyStatement st;
  /**
   * the list of all statements to be sent to MyMoneyStatementReader
   */
  QList<MyMoneyStatement> statements;

  /**
   * a list of already used hashes in this file
   */
  QMap<QString, bool> m_hashMap;

  QString st_AccountName;
  QString st_AccountId;
  eMyMoney::Account::Type accountType;
  bool     firstTransaction;
  bool     mapCategories;
  MyMoneyQifReader::QifEntryTypeE  transactionType;
};

void MyMoneyQifReader::Private::fixMultiLineMemo(QString& memo) const
{
  memo.replace("\\n", "\n");
}

void MyMoneyQifReader::Private::finishStatement()
{
  // in case we have collected any data in the statement, we keep it
  if ((st.m_listTransactions.count() + st.m_listPrices.count() + st.m_listSecurities.count()) > 0) {
    statements += st;
    qDebug("Statement with %d transactions, %d prices and %d securities added to the statement list",
           st.m_listTransactions.count(), st.m_listPrices.count(), st.m_listSecurities.count());
  }
  eMyMoney::Statement::Type type = st.m_eType; //stash type and...
  // start with a fresh statement
  st = MyMoneyStatement();
  st.m_skipCategoryMatching = !mapCategories;
  st.m_eType = type;
}

const QString MyMoneyQifReader::Private::accountTypeToQif(eMyMoney::Account::Type type) const
{
  QString rc = "Bank";

  switch (type) {
    default:
      break;
    case eMyMoney::Account::Type::Cash:
      rc = "Cash";
      break;
    case eMyMoney::Account::Type::CreditCard:
      rc = "CCard";
      break;
    case eMyMoney::Account::Type::Asset:
      rc = "Oth A";
      break;
    case eMyMoney::Account::Type::Liability:
      rc = "Oth L";
      break;
    case eMyMoney::Account::Type::Investment:
      rc = "Port";
      break;
  }
  return rc;
}

const QString MyMoneyQifReader::Private::typeToAccountName(const QString& type) const
{
  if (type == "reinvint")
    return i18nc("Category name", "Reinvested interest");

  if (type == "reinvdiv")
    return i18nc("Category name", "Reinvested dividend");

  if (type == "reinvlg")
    return i18nc("Category name", "Reinvested dividend (long term)");

  if (type == "reinvsh")
    return i18nc("Category name", "Reinvested dividend (short term)");

  if (type == "div")
    return i18nc("Category name", "Dividend");

  if (type == "intinc")
    return i18nc("Category name", "Interest");

  if (type == "cgshort")
    return i18nc("Category name", "Capital Gain (short term)");

  if (type == "cgmid")
    return i18nc("Category name", "Capital Gain (mid term)");

  if (type == "cglong")
    return i18nc("Category name", "Capital Gain (long term)");

  if (type == "rtrncap")
    return i18nc("Category name", "Returned capital");

  if (type == "miscinc")
    return i18nc("Category name", "Miscellaneous income");

  if (type == "miscexp")
    return i18nc("Category name", "Miscellaneous expense");

  if (type == "sell" || type == "buy")
    return i18nc("Category name", "Investment fees");

  return i18n("Unknown QIF type %1", type);
}

bool MyMoneyQifReader::Private::isTransfer(QString& tmp, const QString& leftDelim, const QString& rightDelim)
{
  // it's a transfer, extract the account name
  // I've seen entries like this
  //
  // S[Mehrwertsteuer]/_VATCode_N_I          (The '/' is the Quicken class symbol)
  //
  // so extracting is a bit more complex and we use a regexp for it
  QRegExp exp(QString("\\%1(.*)\\%2(.*)").arg(leftDelim, rightDelim));

  bool rc;
  if ((rc = (exp.indexIn(tmp) != -1)) == true) {
    tmp = exp.cap(1) + exp.cap(2);
    tmp = tmp.trimmed();
  }
  return rc;
}

eMyMoney::Split::State MyMoneyQifReader::Private::reconcileState(const QString& state) const
{
  if (state == "X" || state == "R")       // Reconciled
    return eMyMoney::Split::State::Reconciled;

  if (state == "*")                     // Cleared
    return eMyMoney::Split::State::Cleared;

  return eMyMoney::Split::State::NotReconciled;
}


MyMoneyQifReader::MyMoneyQifReader() :
    d(new Private),
    m_file(nullptr),
    m_extractedLine(0),
    m_autoCreatePayee(true),
    m_pos(0),
    m_linenumber(0),
    m_ft(nullptr)
{
  m_skipAccount = false;
  m_transactionsProcessed =
    m_transactionsSkipped = 0;
  m_progressCallback = 0;
  m_file = 0;
  m_entryType = EntryUnknown;
  m_processingData = false;
  m_userAbort = false;
  m_warnedInvestment = false;
  m_warnedSecurity = false;
  m_warnedPrice = false;

  connect(&m_filter, SIGNAL(bytesWritten(qint64)), this, SLOT(slotSendDataToFilter()));
  connect(&m_filter, SIGNAL(readyReadStandardOutput()), this, SLOT(slotReceivedDataFromFilter()));
  connect(&m_filter, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotImportFinished()));
  connect(&m_filter, SIGNAL(readyReadStandardError()), this, SLOT(slotReceivedErrorFromFilter()));
}

MyMoneyQifReader::~MyMoneyQifReader()
{
  delete m_file;
  delete d;
}

void MyMoneyQifReader::setCategoryMapping(bool map)
{
  d->mapCategories = map;
}

void MyMoneyQifReader::setURL(const QUrl &url)
{
  m_url = url;
}

void MyMoneyQifReader::setProfile(const QString& profile)
{
  m_qifProfile.loadProfile("Profile-" + profile);
}

void MyMoneyQifReader::slotSendDataToFilter()
{
  long len;

  if (m_file->atEnd()) {
    m_filter.closeWriteChannel();
  } else {
    len = m_file->read(m_buffer, sizeof(m_buffer));
    if (len == -1) {
      qWarning("Failed to read block from QIF import file");
      m_filter.closeWriteChannel();
      m_filter.kill();
    } else {
      m_filter.write(m_buffer, len);
    }
  }
}

void MyMoneyQifReader::slotReceivedErrorFromFilter()
{
  qWarning("%s", qPrintable(QString(m_filter.readAllStandardError())));
}

void MyMoneyQifReader::slotReceivedDataFromFilter()
{
  parseReceivedData(m_filter.readAllStandardOutput());
}

void MyMoneyQifReader::parseReceivedData(const QByteArray& data)
{
  const char* buff = data.data();
  int len = data.length();

  m_pos += len;
  // signalProgress(m_pos, 0);

  while (len) {
    // process char
    if (*buff == '\n' || *buff == '\r') {
      // found EOL
      if (!m_lineBuffer.isEmpty()) {
        m_qifLines << QString::fromUtf8(m_lineBuffer.trimmed());
      }
      m_lineBuffer = QByteArray();
    } else {
      // collect all others
      m_lineBuffer += (*buff);
    }
    ++buff;
    --len;
  }
}

void MyMoneyQifReader::slotImportFinished()
{
  // check if the last EOL char was missing and add the trailing line
  if (!m_lineBuffer.isEmpty()) {
    m_qifLines << QString::fromUtf8(m_lineBuffer.trimmed());
  }
  qDebug("Read %ld bytes", m_pos);
  QTimer::singleShot(0, this, SLOT(slotProcessData()));
}

void MyMoneyQifReader::slotProcessData()
{
  signalProgress(-1, -1);

  // scan the file and try to determine numeric and date formats
  m_qifProfile.autoDetect(m_qifLines);

  // the detection is accurate for numeric values, but it could be
  // that the dates were too ambiguous so that we have to let the user
  // decide which one to pick.
  QStringList dateFormats;
  m_qifProfile.possibleDateFormats(dateFormats);
  QString format;
  if (dateFormats.count() > 1) {
    bool ok;
    format = QInputDialog::getItem(0, i18n("Date format selection"), i18n("Pick the date format that suits your input file"), dateFormats, 05, false, &ok);
    if (!ok) {
      m_userAbort = true;
    }
  } else
    format = dateFormats.first();

  if (!format.isEmpty()) {
    m_qifProfile.setInputDateFormat(format);
    qDebug("Selected date format: '%s'", qPrintable(format));
  } else {
    // cancel the process because there is probably nothing to work with
    m_userAbort = true;
  }

  signalProgress(0, m_qifLines.count(), i18n("Importing QIF..."));
  QStringList::iterator it;
  for (it = m_qifLines.begin(); m_userAbort == false && it != m_qifLines.end(); ++it) {
    ++m_linenumber;
    // qDebug("Proc: '%s'", (*it).data());
    if ((*it).startsWith('!')) {
      processQifSpecial(*it);
      m_qifEntry.clear();
    } else if (*it == "^") {
      if (m_qifEntry.count() > 0) {
        signalProgress(m_linenumber, 0);
        processQifEntry();
        m_qifEntry.clear();
      }
    } else {
      m_qifEntry += *it;
    }
  }
  d->finishStatement();

  qDebug("%d lines processed", m_linenumber);
  signalProgress(-1, -1);

  emit statementsReady(d->statements);
}

bool MyMoneyQifReader::startImport()
{
  bool rc = false;
  d->st = MyMoneyStatement();
  d->st.m_skipCategoryMatching = !d->mapCategories;
  m_dontAskAgain.clear();
  m_accountTranslation.clear();
  m_userAbort = false;
  m_pos = 0;
  m_linenumber = 0;
  m_filename.clear();
  m_data.clear();

  if (m_url.isEmpty()) {
    return rc;
  } else if (m_url.isLocalFile()) {
    m_filename = m_url.toLocalFile();
  } else {
    m_filename = QDir::tempPath();
    if(!m_filename.endsWith(QDir::separator()))
      m_filename += QDir::separator();
    m_filename += m_url.fileName();
    qDebug() << "Source:" << m_url.toDisplayString() << "Destination:" << m_filename;
    KIO::FileCopyJob *job = KIO::file_copy(m_url, QUrl::fromUserInput(m_filename), -1, KIO::Overwrite);
//    KJobWidgets::setWindow(job, kmymoney);
    if (job->exec() && job->error()) {
      KMessageBox::detailedError(0, i18n("Error while loading file '%1'.", m_url.toDisplayString()),
                                 job->errorString(),
                                 i18n("File access error"));
      return rc;
    }
  }

  m_file = new QFile(m_filename);
  if (m_file->open(QIODevice::ReadOnly)) {

#ifdef DEBUG_IMPORT
    qint64 len;

    while (!m_file->atEnd()) {
      len = m_file->read(m_buffer, sizeof(m_buffer));
      if (len == -1) {
        qWarning("Failed to read block from QIF import file");
      } else {
        parseReceivedData(QByteArray(m_buffer, len));
      }
    }
    QTimer::singleShot(0, this, SLOT(slotImportFinished()));
    rc = true;
#else
    QString program;
    QStringList arguments;
    program.clear();
    arguments.clear();
    // start filter process, use 'cat -' as the default filter
    if (m_qifProfile.filterScriptImport().isEmpty()) {
#ifdef Q_OS_WIN32                   //krazy:exclude=cpp
    // this is the Windows equivalent of 'cat -' but since 'type' does not work with stdin
    // we pass the filename converted to native separators as a parameter
    program = "cmd.exe";
    arguments << "/c";
    arguments << "type";
    arguments << QDir::toNativeSeparators(m_filename);
#else
    program = "cat";
    arguments << "-";
#endif
    } else {
      arguments << m_qifProfile.filterScriptImport().split(' ', QString::KeepEmptyParts);
      program = arguments.takeFirst();
    }
    m_entryType = EntryUnknown;

    m_filter.setProcessChannelMode(QProcess::MergedChannels);
    m_filter.start(program, arguments);
    if (m_filter.waitForStarted()) {
      signalProgress(0, m_file->size(), i18n("Reading QIF..."));
      slotSendDataToFilter();
      rc = true;
//      emit statementsReady(d->statements);
    } else {
      KMessageBox::detailedError(0, i18n("Error while running the filter '%1'.", m_filter.program()),
                                 m_filter.errorString(),
                                 i18n("Filter error"));
    }
#endif
  }
  return rc;
}

void MyMoneyQifReader::processQifSpecial(const QString& _line)
{
  QString line = _line.mid(1);   // get rid of exclamation mark
  if (line.left(5).toLower() == QString("type:")) {
    line = line.mid(5);

    // exportable accounts
    if (line.toLower() == "ccard" || KMyMoneySettings::qifCreditCard().toLower().contains(line.toLower())) {
      d->accountType = eMyMoney::Account::Type::CreditCard;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if (line.toLower() == "bank" || KMyMoneySettings::qifBank().toLower().contains(line.toLower())) {
      d->accountType = eMyMoney::Account::Type::Checkings;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if (line.toLower() == "cash" || KMyMoneySettings::qifCash().toLower().contains(line.toLower())) {
      d->accountType = eMyMoney::Account::Type::Cash;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if (line.toLower() == "oth a" || KMyMoneySettings::qifAsset().toLower().contains(line.toLower())) {
      d->accountType = eMyMoney::Account::Type::Asset;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if (line.toLower() == "oth l" || line.toLower() == i18nc("QIF tag for liability account", "Oth L").toLower()) {
      d->accountType = eMyMoney::Account::Type::Liability;
      d->firstTransaction = true;
      d->transactionType = m_entryType = EntryTransaction;

    } else if (line.toLower() == "invst" || line.toLower() == i18nc("QIF tag for investment account", "Invst").toLower()) {
      d->accountType = eMyMoney::Account::Type::Investment;
      d->transactionType = m_entryType = EntryInvestmentTransaction;

    } else if (line.toLower() == "invoice" || KMyMoneySettings::qifInvoice().toLower().contains(line.toLower())) {
      m_entryType = EntrySkip;

    } else if (line.toLower() == "tax") {
      m_entryType = EntrySkip;

    } else if (line.toLower() == "bill") {
      m_entryType = EntrySkip;

      // exportable lists
    } else if (line.toLower() == "cat" || line.toLower() == i18nc("QIF tag for category", "Cat").toLower()) {
      m_entryType = EntryCategory;

    } else if (line.toLower() == "security" || line.toLower() == i18nc("QIF tag for security", "Security").toLower()) {
      m_entryType = EntrySecurity;

    } else if (line.toLower() == "prices" || line.toLower() == i18nc("QIF tag for prices", "Prices").toLower()) {
      m_entryType = EntryPrice;

    } else if (line.toLower() == "payee") {
      m_entryType = EntryPayee;

    } else if (line.toLower() == "memorized") {
      m_entryType = EntryMemorizedTransaction;

    } else if (line.toLower() == "class" || line.toLower() == i18nc("QIF tag for a class", "Class").toLower()) {
      m_entryType = EntryClass;

    } else if (line.toLower() == "budget") {
      m_entryType = EntrySkip;

    } else if (line.toLower() == "invitem") {
      m_entryType = EntrySkip;

    } else if (line.toLower() == "template") {
      m_entryType = EntrySkip;

    } else {
      qWarning("Unknown type code '%s' in QIF file on line %d", qPrintable(line), m_linenumber);
      m_entryType = EntrySkip;
    }

    // option headers
  } else if (line.toLower() == "account") {
    m_entryType = EntryAccount;

  } else if (line.toLower() == "option:autoswitch") {
    m_entryType = EntryAccount;

  } else if (line.toLower() == "clear:autoswitch") {
    m_entryType = d->transactionType;
  }
}

void MyMoneyQifReader::processQifEntry()
{
  // This method processes a 'QIF Entry' which is everything between two caret
  // signs
  //
  try {
    switch (m_entryType) {
      case EntryCategory:
        processCategoryEntry();
        break;

      case EntryUnknown:
        qDebug() << "Line " << m_linenumber << ": Warning: Found an entry without a type being specified. Checking assumed.";
        processTransactionEntry();
        break;

      case EntryTransaction:
        processTransactionEntry();
        break;

      case EntryInvestmentTransaction:
        processInvestmentTransactionEntry();
        break;

      case EntryAccount:
        processAccountEntry();
        break;

      case EntrySecurity:
        processSecurityEntry();
        break;

      case EntryPrice:
        processPriceEntry();
        break;

      case EntryPayee:
        processPayeeEntry();
        break;

      case EntryClass:
        qDebug() << "Line " << m_linenumber << ": Classes are not yet supported!";
        break;

      case EntryMemorizedTransaction:
        qDebug() << "Line " << m_linenumber << ": Memorized transactions are not yet implemented!";
        break;

      case EntrySkip:
        break;

      default:
        qDebug() << "Line " << m_linenumber << ": EntryType " << m_entryType << " not yet implemented!";
        break;
    }
  } catch (const MyMoneyException &e) {
    if (QString::fromLatin1(e.what()).contains("USERABORT")) {
      qDebug() << "Line " << m_linenumber << ": Unhandled error: " << e.what();
    } else {
      m_userAbort = true;
    }
  }
}

const QString MyMoneyQifReader::extractLine(const QChar& id, int cnt)
{
  QStringList::ConstIterator it;

  m_extractedLine = -1;
  for (it = m_qifEntry.constBegin(); it != m_qifEntry.constEnd(); ++it) {
    ++m_extractedLine;
    if ((*it)[0] == id) {
      if (cnt-- == 1) {
        return (*it).mid(1);
      }
    }
  }
  m_extractedLine = -1;
  return QString();
}

bool MyMoneyQifReader::extractSplits(QList<qSplit>& listqSplits) const
{
//     *** With apologies to QString MyMoneyQifReader::extractLine ***

  QStringList::ConstIterator it;
  bool ret = false;
  bool memoPresent = false;
  int neededCount = 0;
  qSplit q;

  for (it = m_qifEntry.constBegin(); it != m_qifEntry.constEnd(); ++it) {
    if (((*it)[0] == 'S') || ((*it)[0] == '$') || ((*it)[0] == 'E')) {
      memoPresent = false;  //                      in case no memo line in this split
      if ((*it)[0] == 'E') {
        q.m_strMemo = (*it).mid(1);  //             'E' = Memo
        d->fixMultiLineMemo(q.m_strMemo);
        memoPresent = true;  //                     This transaction contains memo
      } else if ((*it)[0] == 'S') {
        q.m_strCategoryName = (*it).mid(1);  //   'S' = CategoryName
        neededCount ++;
      } else if ((*it)[0] == '$') {
        q.m_amount = (*it).mid(1);  //            '$' = Amount
        neededCount ++;
      }
      if (neededCount > 1) {  //                         CategoryName & Amount essential
        listqSplits += q;  //                       Add valid split
        if (!memoPresent) {  //                     If no memo, clear previous
          q.m_strMemo.clear();
        }
        q = qSplit();  //                               Start new split
        neededCount = 0;
        ret = true;
      }
    }
  }
  return ret;
}

#if 0
void MyMoneyQifReader::processMSAccountEntry(const eMyMoney::Account::Type accountType)
{
  if (extractLine('P').toLower() == m_qifProfile.openingBalanceText().toLower()) {
    m_account = MyMoneyAccount();
    m_account.setAccountType(accountType);
    QString txt = extractLine('T');
    MyMoneyMoney balance = m_qifProfile.value('T', txt);

    QDate date = m_qifProfile.date(extractLine('D'));
    m_account.setOpeningDate(date);

    QString name = extractLine('L');
    if (name.left(1) == m_qifProfile.accountDelimiter().left(1)) {
      name = name.mid(1, name.length() - 2);
    }
    d->st_AccountName = name;
    m_account.setName(name);
    selectOrCreateAccount(Select, m_account, balance);
    d->st.m_accountId = m_account.id();
    if (! balance.isZero()) {
      MyMoneyFile* file = MyMoneyFile::instance();
      QString openingtxid = file->openingBalanceTransaction(m_account);
      MyMoneyFileTransaction ft;
      if (! openingtxid.isEmpty()) {
        MyMoneyTransaction openingtx = file->transaction(openingtxid);
        MyMoneySplit split = openingtx.splitByAccount(m_account.id());

        if (split.shares() != balance) {
          const MyMoneySecurity& sec = file->security(m_account.currencyId());
          if (KMessageBox::questionYesNo(
                KMyMoneyUtils::mainWindow(),
                i18n("The %1 account currently has an opening balance of %2. This QIF file reports an opening balance of %3. Would you like to overwrite the current balance with the one from the QIF file?", m_account.name(), split.shares().formatMoney(m_account, sec), balance.formatMoney(m_account, sec)),
                i18n("Overwrite opening balance"),
                KStandardGuiItem::yes(),
                KStandardGuiItem::no(),
                "OverwriteOpeningBalance")
              == KMessageBox::Yes) {
            file->removeTransaction(openingtx);
            m_account.setOpeningDate(date);
            file->createOpeningBalanceTransaction(m_account, balance);
          }
        }

      } else {
        // Add an opening balance
        m_account.setOpeningDate(date);
        file->createOpeningBalanceTransaction(m_account, balance);
      }
      ft.commit();
    }

  } else {
    // for some unknown reason, Quicken 2001 generates the following (somewhat
    // misleading) sequence of lines:
    //
    //  1: !Account
    //  2: NAT&T Universal
    //  3: DAT&T Univers(...xxxx) [CLOSED]
    //  4: TCCard
    //  5: ^
    //  6: !Type:CCard
    //  7: !Account
    //  8: NCFCU Visa
    //  9: DRick's CFCU Visa card (...xxxx)
    // 10: TCCard
    // 11: ^
    // 12: !Type:CCard
    // 13: D1/ 4' 1
    //
    // Lines 1-5 are processed via processQifEntry() and processAccountEntry()
    // Then Quicken issues line 6 but since the account does not carry any
    // transaction does not write an end delimiter. Arrrgh! So we end up with
    // a QIF entry comprising of lines 6-11 and end up in this routine. Actually,
    // lines 7-11 are the leadin for the next account. So we check here if
    // the !Type:xxx record also contains an !Account line and process the
    // entry as required.
    //
    // (Ace) I think a better solution here is to handle exclamation point
    // lines separately from entries.  In the above case:
    // Line 1 would set the mode to "account entries".
    // Lines 2-5 would be interpreted as an account entry.  This would set m_account.
    // Line 6 would set the mode to "cc transaction entries".
    // Line 7 would immediately set the mode to "account entries" again
    // Lines 8-11 would be interpreted as an account entry.  This would set m_account.
    // Line 12 would set the mode to "cc transaction entries"
    // Lines 13+ would be interpreted as cc transaction entries, and life is good
    int exclamationCnt = 1;
    QString category;
    do {
      category = extractLine('!', exclamationCnt++);
    } while (!category.isEmpty() && category != "Account");

    // we have such a weird empty account
    if (category == "Account") {
      processAccountEntry();
    } else {
      selectOrCreateAccount(Select, m_account);

      d->st_AccountName = m_account.name();
      d->st.m_strAccountName = m_account.name();
      d->st.m_accountId = m_account.id();
      d->st.m_strAccountNumber = m_account.id();
      m_account.setNumber(m_account.id());
      if (m_entryType == EntryInvestmentTransaction)
        processInvestmentTransactionEntry();
      else
        processTransactionEntry();
    }
  }
}
#endif

void MyMoneyQifReader::processPayeeEntry()
{
  // TODO
}

void MyMoneyQifReader::processCategoryEntry()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount account = MyMoneyAccount();
  account.setName(extractLine('N'));
  account.setDescription(extractLine('D'));

  MyMoneyAccount parentAccount;
  //The extractline routine will more than likely return 'empty',
  // so also have to test that either the 'I' or 'E' was detected
  //and set up accounts accordingly.
  if ((!extractLine('I').isEmpty()) || (m_extractedLine != -1)) {
    account.setAccountType(eMyMoney::Account::Type::Income);
    parentAccount = file->income();
  } else if ((!extractLine('E').isEmpty()) || (m_extractedLine != -1)) {
    account.setAccountType(eMyMoney::Account::Type::Expense);
    parentAccount = file->expense();
  }

  // check if we can find the account already in the file
  auto acc = findAccount(account, MyMoneyAccount());

  // if not, we just create it
  if (acc.id().isEmpty()) {
    MyMoneyAccount brokerage;
    file->createAccount(account, parentAccount, brokerage, MyMoneyMoney());
  }
}

MyMoneyAccount MyMoneyQifReader::findAccount(const MyMoneyAccount& acc, const MyMoneyAccount& parent) const
{
  static MyMoneyAccount nullAccount;

  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyAccount> parents;
  try {
    // search by id
    if (!acc.id().isEmpty()) {
      return file->account(acc.id());
    }
    // collect the parents. in case parent does not have an id, we scan the all top-level accounts
    if (parent.id().isEmpty()) {
      parents << file->asset();
      parents << file->liability();
      parents << file->income();
      parents << file->expense();
      parents << file->equity();
    } else {
      parents << parent;
    }
    QList<MyMoneyAccount>::const_iterator it_p;
    for (it_p = parents.constBegin(); it_p != parents.constEnd(); ++it_p) {
      MyMoneyAccount parentAccount = *it_p;
      // search by name (allow hierarchy)
      int pos;
      // check for ':' in the name and use it as separator for a hierarchy
      QString name = acc.name();
      bool notFound = false;
      while ((pos = name.indexOf(MyMoneyFile::AccountSeparator)) != -1) {
        QString part = name.left(pos);
        QString remainder = name.mid(pos + 1);
        const auto existingAccount = file->subAccountByName(parentAccount, part);
        // if account has not been found, continue with next top level parent
        if (existingAccount.id().isEmpty()) {
          notFound = true;
          break;
        }
        parentAccount = existingAccount;
        name = remainder;
      }
      if (notFound)
        continue;
      const auto existingAccount = file->subAccountByName(parentAccount, name);
      if (!existingAccount.id().isEmpty()) {
        if (acc.accountType() != eMyMoney::Account::Type::Unknown) {
          if (acc.accountType() != existingAccount.accountType())
            continue;
        }
        return existingAccount;
      }
    }
  } catch (const MyMoneyException &e) {
    KMessageBox::error(0, i18n("Unable to find account: %1", QString::fromLatin1(e.what())));
  }
  return nullAccount;
}

const QString MyMoneyQifReader::transferAccount(const QString& name, bool useBrokerage)
{
  QString accountId;
  QStringList tmpEntry = m_qifEntry;   // keep temp copies
  MyMoneyAccount tmpAccount = m_account;

  m_qifEntry.clear();               // and construct a temp entry to create/search the account
  m_qifEntry << QString("N%1").arg(name);
  m_qifEntry << QString("Tunknown");
  m_qifEntry << QString("D%1").arg(i18n("Autogenerated by QIF importer"));
  accountId = processAccountEntry(false);

  // in case we found a reference to an investment account, we need
  // to switch to the brokerage account instead.
  MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
  if (useBrokerage && (acc.accountType() == eMyMoney::Account::Type::Investment)) {
    m_qifEntry.clear();               // and construct a temp entry to create/search the account
    m_qifEntry << QString("N%1").arg(acc.brokerageName());
    m_qifEntry << QString("Tunknown");
    m_qifEntry << QString("D%1").arg(i18n("Autogenerated by QIF importer"));
    accountId = processAccountEntry(false);
  }
  m_qifEntry = tmpEntry;               // restore local copies
  m_account = tmpAccount;

  return accountId;
}

void MyMoneyQifReader::createOpeningBalance(eMyMoney::Account::Type accType)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // if we don't have a name for the current account we need to extract the name from the L-record
  if (m_account.name().isEmpty()) {
    QString name = extractLine('L');
    if (name.isEmpty()) {
      name = i18n("QIF imported, no account name supplied");
    }
    auto b = d->isTransfer(name, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1));
    Q_UNUSED(b)
    QStringList entry = m_qifEntry;   // keep a temp copy
    m_qifEntry.clear();               // and construct a temp entry to create/search the account
    m_qifEntry << QString("N%1").arg(name);
    m_qifEntry << QString("T%1").arg(d->accountTypeToQif(accType));
    m_qifEntry << QString("D%1").arg(i18n("Autogenerated by QIF importer"));
    processAccountEntry();
    m_qifEntry = entry;               // restore local copy
  }

  MyMoneyFileTransaction ft;
  try {
    bool needCreate = true;

    MyMoneyAccount acc = m_account;
    // in case we're dealing with an investment account, we better use
    // the accompanying brokerage account for the opening balance
    acc = file->accountByName(m_account.brokerageName());

    // check if we already have an opening balance transaction
    QString tid = file->openingBalanceTransaction(acc);
    MyMoneyTransaction ot;
    if (!tid.isEmpty()) {
      ot = file->transaction(tid);
      MyMoneySplit s0 = ot.splitByAccount(acc.id());
      // if the value is the same, we can silently skip this transaction
      if (s0.shares() == m_qifProfile.value('T', extractLine('T'))) {
        needCreate = false;
      }
      if (needCreate) {
        // in case we create it anyway, we issue a warning to the user to check it manually
        KMessageBox::sorry(0, QString("<qt>%1</qt>").arg(i18n("KMyMoney has imported a second opening balance transaction into account <b>%1</b> which differs from the one found already on file. Please correct this manually once the import is done.", acc.name())), i18n("Opening balance problem"));
      }
    }

    if (needCreate) {
      acc.setOpeningDate(m_qifProfile.date(extractLine('D')));
      file->modifyAccount(acc);
      MyMoneyTransaction t = file->createOpeningBalanceTransaction(acc, m_qifProfile.value('T', extractLine('T')));
      if (!t.id().isEmpty()) {
        t.setImported();
        file->modifyTransaction(t);
      }
      ft.commit();
    }

    // make sure to use the updated version of the account
    if (m_account.id() == acc.id())
      m_account = acc;

    // remember which account we created
    d->st.m_accountId = m_account.id();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedError(nullptr,
                               i18n("Error while creating opening balance transaction"),
                               e.what(),
                               i18n("File access error"));
  }
}

void MyMoneyQifReader::processTransactionEntry()
{
  ++m_transactionsProcessed;
  // in case the user selected to skip the account or the account
  // was not found we skip this transaction
  /*
    if(m_account.id().isEmpty()) {
      m_transactionsSkipped++;
      return;
    }
  */
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyStatement::Split s1;
  MyMoneyStatement::Transaction tr;
  QString tmp;
  QString accountId;
  int pos;
  QString payee = extractLine('P');
  unsigned long h;

  h = MyMoneyTransaction::hash(m_qifEntry.join(";"));

  QString hashBase;
  hashBase.sprintf("%s-%07lx", qPrintable(m_qifProfile.date(extractLine('D')).toString(Qt::ISODate)), h);
  int idx = 1;
  QString hash;
  for (;;) {
    hash = QString("%1-%2").arg(hashBase).arg(idx);
    QMap<QString, bool>::const_iterator it;
    it = d->m_hashMap.constFind(hash);
    if (it == d->m_hashMap.constEnd()) {
      d->m_hashMap[hash] = true;
      break;
    }
    ++idx;
  }
  tr.m_strBankID = hash;

  if (d->firstTransaction) {
    // check if this is an opening balance transaction and process it out of the statement
    if (!payee.isEmpty() && ((payee.toLower() == "opening balance") || KMyMoneySettings::qifOpeningBalance().toLower().contains(payee.toLower()))) {
      createOpeningBalance(d->accountType);
      d->firstTransaction = false;
      return;
    }
  }

  // Process general transaction data

  if (d->st.m_accountId.isEmpty())
    d->st.m_accountId = m_account.id();

  s1.m_accountId = d->st.m_accountId;
  switch (d->accountType) {
  case eMyMoney::Account::Type::Checkings:
    d->st.m_eType=eMyMoney::Statement::Type::Checkings;
    break;
  case eMyMoney::Account::Type::Savings:
    d->st.m_eType=eMyMoney::Statement::Type::Savings;
    break;
  case eMyMoney::Account::Type::Investment:
    d->st.m_eType=eMyMoney::Statement::Type::Investment;
    break;
  case eMyMoney::Account::Type::CreditCard:
    d->st.m_eType=eMyMoney::Statement::Type::CreditCard;
    break;
  default:
    d->st.m_eType=eMyMoney::Statement::Type::None;
    break;
  }

  tr.m_datePosted = (m_qifProfile.date(extractLine('D')));
  if (!tr.m_datePosted.isValid()) {
    int rc = KMessageBox::warningContinueCancel(0,
             i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
                  "date profile setting of \"%2\".\n\nPressing \"Continue\" will "
                  "assign todays date to the transaction. Pressing \"Cancel\" will abort "
                  "the import operation. You can then restart the import and select a different "
                  "QIF profile or create a new one.", extractLine('D'), m_qifProfile.inputDateFormat()),
             i18n("Invalid date format"));
    switch (rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = (QDate::currentDate());
        break;

      case KMessageBox::Cancel:
        throw MYMONEYEXCEPTION_CSTRING("USERABORT");
        break;
    }
  }

  tmp = extractLine('L');
  pos = tmp.lastIndexOf("--");
  if (tmp.left(1) == m_qifProfile.accountDelimiter().left(1)) {
    // it's a transfer, so we wipe the memo
//   tmp = "";         why??
//    st.m_strAccountName = tmp;
  } else if (pos != -1) {
//    what's this?
//    t.setValue("Dialog", tmp.mid(pos+2));
    tmp = tmp.left(pos);
  }
//  t.setMemo(tmp);

  // Assign the "#" field to the transaction's bank id
  // This is the custom KMM extension to QIF for a unique ID
  tmp = extractLine('#');
  if (!tmp.isEmpty()) {
    tr.m_strBankID = QString("ID %1").arg(tmp);
  }

#if 0
  // Collect data for the account's split
  s1.m_accountId = m_account.id();
  tmp = extractLine('S');
  pos = tmp.findRev("--");
  if (pos != -1) {
    tmp = tmp.left(pos);
  }
  if (tmp.left(1) == m_qifProfile.accountDelimiter().left(1))
    // it's a transfer, extract the account name
    tmp = tmp.mid(1, tmp.length() - 2);
  s1.m_strCategoryName = tmp;
#endif
  // TODO (Ace) Deal with currencies more gracefully.  QIF cannot deal with multiple
  // currencies, so we should assume that transactions imported into a given
  // account are in THAT ACCOUNT's currency.  If one of those involves a transfer
  // to an account with a different currency, value and shares should be
  // different.  (Shares is in the target account's currency, value is in the
  // transaction's)


  s1.m_amount = m_qifProfile.value('T', extractLine('T'));
  tr.m_amount = m_qifProfile.value('T', extractLine('T'));
  tr.m_shares = m_qifProfile.value('T', extractLine('T'));
  tmp = extractLine('N');
  if (!tmp.isEmpty())
    tr.m_strNumber = tmp;

  if (!payee.isEmpty()) {
    tr.m_strPayee = payee;
  }

  tr.m_reconcile = d->reconcileState(extractLine('C'));
  tr.m_strMemo = extractLine('M');
  d->fixMultiLineMemo(tr.m_strMemo);
  s1.m_strMemo = tr.m_strMemo;
  // tr.m_listSplits.append(s1);

  //             split transaction
  //      ****** ensure each field is ******
  //      *   attached to correct split    *
  QList<qSplit> listqSplits;
  if (! extractSplits(listqSplits)) {
    MyMoneyAccount account;
    // use the same values for the second split, but clear the ID and reverse the value
    MyMoneyStatement::Split s2 = s1;
    s2.m_reconcile = tr.m_reconcile;
    s2.m_amount = (-s1.m_amount);
//    s2.clearId();

    // standard transaction
    tmp = extractLine('L');
    if (d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1))) {
      accountId = transferAccount(tmp, false);

    } else {
      /*      pos = tmp.findRev("--");
            if(pos != -1) {
              t.setValue("Dialog", tmp.mid(pos+2));
              tmp = tmp.left(pos);
            }*/

      // it's an expense / income
      tmp = tmp.trimmed();
      accountId = file->checkCategory(tmp, s1.m_amount, s2.m_amount);
    }

    if (!accountId.isEmpty()) {
      try {
        account = file->account(accountId);
        // FIXME: check that the type matches and ask if not

        if (account.accountType() == eMyMoney::Account::Type::Investment) {
          qDebug() << "Line " << m_linenumber << ": Cannot transfer to/from an investment account. Transaction ignored.";
          return;
        }
        if (account.id() == m_account.id()) {
          qDebug() << "Line " << m_linenumber << ": Cannot transfer to the same account. Transfer ignored.";
          accountId.clear();
        }

      } catch (const MyMoneyException &) {
        qDebug() << "Line " << m_linenumber << ": Account with id " << accountId.data() << " not found";
        accountId.clear();
      }
    }

    if (!accountId.isEmpty()) {
      s2.m_accountId = accountId;
      s2.m_strCategoryName = tmp;
      tr.m_listSplits.append(s2);
    }

  } else {
    int   count;
    for (count = 1; count <= listqSplits.count(); ++count) {                     // Use true splits count
      MyMoneyStatement::Split s2 = s1;
      s2.m_amount = (-m_qifProfile.value('$', listqSplits[count-1].m_amount));   // Amount of split
      s2.m_strMemo = listqSplits[count-1].m_strMemo;                             // Memo in split
      tmp = listqSplits[count-1].m_strCategoryName;                              // Category in split

      if (d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1))) {
        accountId = transferAccount(tmp, false);

      } else {
        pos = tmp.lastIndexOf("--");
        if (pos != -1) {
          tmp = tmp.left(pos);
        }
        tmp = tmp.trimmed();
        accountId = file->checkCategory(tmp, s1.m_amount, s2.m_amount);
      }

      if (!accountId.isEmpty()) {
        try {
          MyMoneyAccount account = file->account(accountId);
          // FIXME: check that the type matches and ask if not

          if (account.accountType() == eMyMoney::Account::Type::Investment) {
            qDebug() << "Line " << m_linenumber << ": Cannot convert a split transfer to/from an investment account. Split removed. Total amount adjusted from " << tr.m_amount.formatMoney("", 2) << " to " << (tr.m_amount + s2.m_amount).formatMoney("", 2) << "\n";
            tr.m_amount += s2.m_amount;
            continue;
          }
          if (account.id() == m_account.id()) {
            qDebug() << "Line " << m_linenumber << ": Cannot transfer to the same account. Transfer ignored.";
            accountId.clear();
          }

        } catch (const MyMoneyException &) {
          qDebug() << "Line " << m_linenumber << ": Account with id " << accountId.data() << " not found";
          accountId.clear();
        }
      }
      if (!accountId.isEmpty()) {
        s2.m_accountId = accountId;
        s2.m_strCategoryName = tmp;
        tr.m_listSplits += s2;
        // in case the transaction does not have a memo and we
        // process the first split just copy the memo over
        if (tr.m_listSplits.count() == 1 && tr.m_strMemo.isEmpty())
          tr.m_strMemo = s2.m_strMemo;
      } else {
        // TODO add an option to create a "Unassigned" category
        // for now, we just drop the split which will show up as unbalanced
        // transaction in the KMyMoney ledger view
      }
    }
  }

  // Add the transaction to the statement
  d->st.m_listTransactions += tr;
}

void MyMoneyQifReader::processInvestmentTransactionEntry()
{
//   qDebug() << "Investment Transaction:" << m_qifEntry.count() << " lines";
  /*
  Items for Investment Accounts
  Field   Indicator Explanation
  D   Date
  N   Action
  Y   Security (NAME, not symbol)
  I   Price
  Q   Quantity (number of shares or split ratio)
  T   Transaction amount
  C   Cleared status
  P   Text in the first line for transfers and reminders (Payee)
  M   Memo
  O   Commission
  L   Account for the transfer
  $   Amount transferred
  ^   End of the entry

  It will be presumed all transactions are to the associated cash account, if
  one exists, unless otherwise noted by the 'L' field.

  Expense/Income categories will be automatically generated, "_Dividend",
  "_InterestIncome", etc.

  */

  MyMoneyStatement::Transaction tr;
  d->st.m_eType = eMyMoney::Statement::Type::Investment;

//  t.setCommodity(m_account.currencyId());
  // 'D' field: Date
  QDate date = m_qifProfile.date(extractLine('D'));
  if (date.isValid())
    tr.m_datePosted = date;
  else {
    int rc = KMessageBox::warningContinueCancel(0,
             i18n("The date entry \"%1\" read from the file cannot be interpreted through the current "
                  "date profile setting of \"%2\".\n\nPressing \"Continue\" will "
                  "assign todays date to the transaction. Pressing \"Cancel\" will abort "
                  "the import operation. You can then restart the import and select a different "
                  "QIF profile or create a new one.", extractLine('D'), m_qifProfile.inputDateFormat()),
             i18n("Invalid date format"));
    switch (rc) {
      case KMessageBox::Continue:
        tr.m_datePosted = QDate::currentDate();
        break;

      case KMessageBox::Cancel:
        throw MYMONEYEXCEPTION_CSTRING("USERABORT");
        break;
    }
  }

  // 'M' field: Memo
  QString memo = extractLine('M');
  d->fixMultiLineMemo(memo);
  tr.m_strMemo = memo;
  unsigned long h;

  h = MyMoneyTransaction::hash(m_qifEntry.join(";"));

  QString hashBase;
  hashBase.sprintf("%s-%07lx", qPrintable(m_qifProfile.date(extractLine('D')).toString(Qt::ISODate)), h);
  int idx = 1;
  QString hash;
  for (;;) {
    hash = QString("%1-%2").arg(hashBase).arg(idx);
    QMap<QString, bool>::const_iterator it;
    it = d->m_hashMap.constFind(hash);
    if (it == d->m_hashMap.constEnd()) {
      d->m_hashMap[hash] = true;
      break;
    }
    ++idx;
  }
  tr.m_strBankID = hash;

  // '#' field: BankID
  QString tmp = extractLine('#');
  if (! tmp.isEmpty())
    tr.m_strBankID = QString("ID %1").arg(tmp);

  // Reconciliation flag
  tr.m_reconcile = d->reconcileState(extractLine('C'));

  // 'O' field: Fees
  tr.m_fees = m_qifProfile.value('T', extractLine('O'));
  // 'T' field: Amount
  MyMoneyMoney amount = m_qifProfile.value('T', extractLine('T'));
  tr.m_amount = amount;

  MyMoneyStatement::Price price;

  price.m_date = date;
  price.m_strSecurity = extractLine('Y');
  price.m_amount = m_qifProfile.value('T', extractLine('I'));

#if 0 // we must check for that later, because certain activities don't need a security
  // 'Y' field: Security name

  QString securityname = extractLine('Y').toLower();
  if (securityname.isEmpty()) {
    qDebug() << "Line " << m_linenumber << ": Investment transaction without a security is not supported.";
    return;
  }
  tr.m_strSecurity = securityname;
#endif

#if 0

  // For now, we let the statement reader take care of that.

  // The big problem here is that the Y field is not the SYMBOL, it's the NAME.
  // The name is not very unique, because people could have used slightly different
  // abbreviations or ordered words differently, etc.
  //
  // If there is a perfect name match with a subordinate stock account, great.
  // More likely, we have to rely on the QIF file containing !Type:Security
  // records, which tell us the mapping from name to symbol.
  //
  // Therefore, generally it is not recommended to import a QIF file containing
  // investment transactions but NOT containing security records.

  QString securitysymbol = m_investmentMap[securityname];

  // the correct account is the stock account which matches two criteria:
  // (1) it is a sub-account of the selected investment account, and either
  // (2a) the security name of the transaction matches the name of the security, OR
  // (2b) the security name of the transaction maps to a symbol which matches the symbol of the security

  // search through each subordinate account
  bool found = false;
  MyMoneyAccount thisaccount = m_account;
  QStringList accounts = thisaccount.accountList();
  QStringList::const_iterator it_account = accounts.begin();
  while (!found && it_account != accounts.end()) {
    QString currencyid = file->account(*it_account).currencyId();
    MyMoneySecurity security = file->security(currencyid);
    QString symbol = security.tradingSymbol().toLower();
    QString name = security.name().toLower();

    if (securityname == name || securitysymbol == symbol) {
      d->st_AccountId = *it_account;
      s1.m_accountId = *it_account;
      thisaccount = file->account(*it_account);
      found = true;

#if 0
      // update the price, while we're here.  in the future, this should be
      // an option
      QString basecurrencyid = file->baseCurrency().id();
      MyMoneyPrice price = file->price(currencyid, basecurrencyid, t_in.m_datePosted, true);
      if (!price.isValid()) {
        MyMoneyPrice newprice(currencyid, basecurrencyid, t_in.m_datePosted, t_in.m_moneyAmount / t_in.m_dShares, i18n("Statement Importer"));
        file->addPrice(newprice);
      }
#endif
    }

    ++it_account;
  }

  if (!found) {
    qDebug() << "Line " << m_linenumber << ": Security " << securityname << " not found in this account.  Transaction ignored.";

    // If the security is not known, notify the user
    // TODO (Ace) A "SelectOrCreateAccount" interface for investments
    KMessageBox::information(0, i18n("This investment account does not contain the \"%1\" security.  "
                                     "Transactions involving this security will be ignored.", securityname),
                             i18n("Security not found"),
                             QString("MissingSecurity%1").arg(securityname.trimmed()));
    return;
  }
#endif

  // 'Y' field: Security
  tr.m_strSecurity = extractLine('Y');

  // 'Q' field: Quantity
  MyMoneyMoney quantity = m_qifProfile.value('T', extractLine('Q'));

  // 'N' field: Action
  QString action = extractLine('N').toLower();

  // remove trailing X, which seems to have no purpose (?!)
  bool xAction = false;
  if (action.endsWith('x')) {
    action = action.left(action.length() - 1);
    xAction = true;
  }

  tmp = extractLine('L');
  // if the action ends in an X, the L-Record contains the asset account
  // to which the dividend should be transferred. In the other cases, it
  // may contain a category that identifies the income category for the
  // dividend payment
  if ((xAction == true)
      || (d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1)) == true)) {
    tmp = tmp.remove(QRegExp("[\\[\\]]"));                 //  xAction != true so ignore any'[ and ]'
    if (!tmp.isEmpty()) {                                  // use 'L' record name
      tr.m_strBrokerageAccount = tmp;
      transferAccount(tmp);                                // make sure the account exists
    } else {
      tr.m_strBrokerageAccount = m_account.brokerageName();// use brokerage account
      transferAccount(m_account.brokerageName());          // make sure the account exists
    }
  } else {
    tmp = tmp.remove(QRegExp("[\\[\\]]"));                 //  xAction != true so ignore any'[ and ]'
    tr.m_strInterestCategory = tmp;
    tr.m_strBrokerageAccount = m_account.brokerageName();
  }

  // Whether to create a cash split for the other side of the value
  QString accountname; //= extractLine('L');
  if (action == "reinvint" || action == "reinvdiv" || action == "reinvlg" || action == "reinvsh") {
    d->st.m_listPrices += price;
    tr.m_shares = quantity;
    tr.m_eAction = (eMyMoney::Transaction::Action::ReinvestDividend);
    tr.m_price = m_qifProfile.value('I', extractLine('I'));

    tr.m_strInterestCategory = extractLine('L');
    if (tr.m_strInterestCategory.isEmpty()) {
      tr.m_strInterestCategory = d->typeToAccountName(action);
    }
  } else if (action == "div" || action == "cgshort" || action == "cgmid" || action == "cglong" || action == "rtrncap") {
    tr.m_eAction = (eMyMoney::Transaction::Action::CashDividend);

    // make sure, we have valid category. Either taken from the L-Record above,
    // or derived from the action code
    if (tr.m_strInterestCategory.isEmpty()) {
      tr.m_strInterestCategory = d->typeToAccountName(action);
    }

    // For historic reasons (coming from the OFX importer) the statement
    // reader expects the dividend with a reverse sign. So we just do that.
    tr.m_amount -= tr.m_fees;

    // We need an extra split which will be the zero-amount investment split
    // that serves to mark this transaction as a cash dividend and note which
    // stock account it belongs to.
    MyMoneyStatement::Split s2;
    s2.m_amount = MyMoneyMoney();
    s2.m_strCategoryName = extractLine('Y');
    tr.m_listSplits.append(s2);
  } else if (action == "intinc" || action == "miscinc" || action == "miscexp") {
    tr.m_eAction = (eMyMoney::Transaction::Action::Interest);
    if (action == "miscexp")
      tr.m_eAction = (eMyMoney::Transaction::Action::Fees);

    // make sure, we have a valid category. Either taken from the L-Record above,
    // or derived from the action code
    if (tr.m_strInterestCategory.isEmpty()) {
      tr.m_strInterestCategory = d->typeToAccountName(action);
    }

    if (action == "intinc") {
      MyMoneyMoney priceValue = m_qifProfile.value('I', extractLine('I'));
      tr.m_amount -= tr.m_fees;
      if ((!quantity.isZero()) && (!priceValue.isZero()))
        tr.m_amount = -(quantity * priceValue);
    } else
      // For historic reasons (coming from the OFX importer) the statement
      // reader expects the dividend with a reverse sign. So we just do that.
      if (action != "miscexp")
        tr.m_amount = -(amount - tr.m_fees);

    if (tr.m_strMemo.isEmpty())
      tr.m_strMemo = (QString("%1 %2").arg(extractLine('Y')).arg(d->typeToAccountName(action))).trimmed();
  } else if (action == "xin" || action == "xout") {
    QString payee = extractLine('P');
    if (!payee.isEmpty() && ((payee.toLower() == "opening balance") || KMyMoneySettings::qifOpeningBalance().toLower().contains(payee.toLower()))) {
      createOpeningBalance(eMyMoney::Account::Type::Investment);
      return;
    }

    tr.m_eAction = (eMyMoney::Transaction::Action::None);
    MyMoneyStatement::Split s2;
    tmp = extractLine('L');
    if (d->isTransfer(tmp, m_qifProfile.accountDelimiter().left(1), m_qifProfile.accountDelimiter().mid(1, 1))) {
      s2.m_accountId = transferAccount(tmp);
      s2.m_strCategoryName = tmp;
    } else {
      s2.m_strCategoryName = extractLine('L');
      if (tr.m_strInterestCategory.isEmpty()) {
        s2.m_strCategoryName = d->typeToAccountName(action);
      }
    }

    if (action == "xout")
      tr.m_amount = -tr.m_amount;

    s2.m_amount = -tr.m_amount;
    tr.m_listSplits.append(s2);
  } else if (action == "buy") {
    d->st.m_listPrices += price;
    tr.m_price = m_qifProfile.value('I', extractLine('I'));
    tr.m_shares = quantity;
    tr.m_amount = -amount;
    tr.m_eAction = (eMyMoney::Transaction::Action::Buy);
  } else if (action == "sell") {
    d->st.m_listPrices += price;
    tr.m_price = m_qifProfile.value('I', extractLine('I'));
    tr.m_shares = -quantity;
    tr.m_amount = amount;
    tr.m_eAction = (eMyMoney::Transaction::Action::Sell);
  } else if (action == "shrsin") {
    tr.m_shares = quantity;
    tr.m_eAction = (eMyMoney::Transaction::Action::Shrsin);
  } else if (action == "shrsout") {
    tr.m_shares = -quantity;
    tr.m_eAction = (eMyMoney::Transaction::Action::Shrsout);
  } else if (action == "stksplit") {
    MyMoneyMoney splitfactor = (quantity / MyMoneyMoney(10, 1)).reduce();

    // Stock splits not supported
//     qDebug() << "Line " << m_linenumber << ": Stock split not supported (date=" << date << " security=" << securityname << " factor=" << splitfactor.toString() << ")";

//    s1.setShares(splitfactor);
//    s1.setValue(0);
//   s1.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares));

//     return;
  } else {
    // Unsupported action type
    qDebug() << "Line " << m_linenumber << ": Unsupported transaction action (" << action << ")";
    return;
  }
  d->st.m_strAccountName = accountname;  //  accountname appears not to get set
  d->st.m_listTransactions += tr;

  /*************************************************************************
   *
   * These transactions are natively supported by KMyMoney
   *
   *************************************************************************/
  /*
  D1/ 3' 5
  NShrsIn
  YGENERAL MOTORS CORP 52BR1
  I20
  Q200
  U4,000.00
  T4,000.00
  M200 shares added to account @ $20/share
  ^
  */
  /*
  ^
  D1/14' 5
  NShrsOut
  YTEMPLETON GROWTH 97GJ0
  Q50
  90  ^
  */
  /*
  D1/28' 5
  NBuy
  YGENERAL MOTORS CORP 52BR1
  I24.35
  Q100
  U2,435.00
  T2,435.00
  ^
  */
  /*
  D1/ 5' 5
  NSell
  YUnited Vanguard
  I8.41
  Q50
  U420.50
  T420.50
  ^
  */
  /*
  D1/ 7' 5
  NReinvDiv
  YFRANKLIN INCOME 97GM2
  I38
  Q1
  U38.00
  T38.00
  ^
  */
  /*************************************************************************
   *
   * These transactions are all different kinds of income.  (Anything that
   * follows the DNYUT pattern).  They are all handled the same, the only
   * difference is which income account the income is placed into.  By
   * default, it's placed into _xxx where xxx is the right side of the
   * N field.  e.g. NDiv transaction goes into the _Div account
   *
   *************************************************************************/
  /*
  D1/10' 5
  NDiv
  YTEMPLETON GROWTH 97GJ0
  U10.00
  T10.00
  ^
  */
  /*
  D1/10' 5
  NIntInc
  YTEMPLETON GROWTH 97GJ0
  U20.00
  T20.00
  ^
  */
  /*
  D1/10' 5
  NCGShort
  YTEMPLETON GROWTH 97GJ0
  U111.00
  T111.00
  ^
  */
  /*
  D1/10' 5
  NCGLong
  YTEMPLETON GROWTH 97GJ0
  U333.00
  T333.00
  ^
  */
  /*
  D1/10' 5
  NCGMid
  YTEMPLETON GROWTH 97GJ0
  U222.00
  T222.00
  ^
  */
  /*
  D2/ 2' 5
  NRtrnCap
  YFRANKLIN INCOME 97GM2
  U1,234.00
  T1,234.00
  ^
  */
  /*************************************************************************
   *
   * These transactions deal with miscellaneous activity that KMyMoney
   * does not support, but may support in the future.
   *
   *************************************************************************/
  /*   Note the Q field is the split ratio per 10 shares, so Q12.5 is a
        12.5:10 split, otherwise known as 5:4.
  D1/14' 5
  NStkSplit
  YIBM
  Q12.5
  ^
  */
  /*************************************************************************
   *
   * These transactions deal with short positions and options, which are
   * not supported at all by KMyMoney.  They will be ignored for now.
   * There may be a way to hack around this, by creating a new security
   * "IBM_Short".
   *
   *************************************************************************/
  /*
  D1/21' 5
  NShtSell
  YIBM
  I92.38
  Q100
  U9,238.00
  T9,238.00
  ^
  */
  /*
  D1/28' 5
  NCvrShrt
  YIBM
  I92.89
  Q100
  U9,339.00
  T9,339.00
  O50.00
  ^
  */
  /*
  D6/ 1' 5
  NVest
  YIBM Option
  Q20
  ^
  */
  /*
  D6/ 8' 5
  NExercise
  YIBM Option
  I60.952381
  Q20
  MFrom IBM Option Grant 6/1/2004
  ^
  */
  /*
  D6/ 1'14
  NExpire
  YIBM Option
  Q5
  ^
  */
  /*************************************************************************
   *
   * These transactions do not have an associated investment ("Y" field)
   * so presumably they are only valid for the cash account.  Once I
   * understand how these are really implemented, they can probably be
   * handled without much trouble.
   *
   *************************************************************************/
  /*
  D1/14' 5
  NCash
  U-100.00
  T-100.00
  LBank Chrg
  ^
  */
  /*
  D1/15' 5
  NXOut
  U500.00
  T500.00
  L[CU Savings]
  $500.00
  ^
  */
  /*
  D1/28' 5
  NXIn
  U1,000.00
  T1,000.00
  L[CU Checking]
  $1,000.00
  ^
  */
  /*
  D1/25' 5
  NMargInt
  U25.00
  T25.00
  ^
  */
}

const QString MyMoneyQifReader::findOrCreateIncomeAccount(const QString& searchname)
{
  QString result;

  MyMoneyFile *file = MyMoneyFile::instance();

  // First, try to find this account as an income account
  MyMoneyAccount acc = file->income();
  QStringList list = acc.accountList();
  QStringList::ConstIterator it_accid = list.constBegin();
  while (it_accid != list.constEnd()) {
    acc = file->account(*it_accid);
    if (acc.name() == searchname) {
      result = *it_accid;
      break;
    }
    ++it_accid;
  }

  // If we did not find the account, now we must create one.
  if (result.isEmpty()) {
    MyMoneyAccount newAccount;
    newAccount.setName(searchname);
    newAccount.setAccountType(eMyMoney::Account::Type::Income);
    MyMoneyAccount income = file->income();
    MyMoneyFileTransaction ft;
    file->addAccount(newAccount, income);
    ft.commit();
    result = newAccount.id();
  }

  return result;
}

// TODO (Ace) Combine this and the previous function

const QString MyMoneyQifReader::findOrCreateExpenseAccount(const QString& searchname)
{
  QString result;

  MyMoneyFile *file = MyMoneyFile::instance();

  // First, try to find this account as an income account
  MyMoneyAccount acc = file->expense();
  QStringList list = acc.accountList();
  QStringList::ConstIterator it_accid = list.constBegin();
  while (it_accid != list.constEnd()) {
    acc = file->account(*it_accid);
    if (acc.name() == searchname) {
      result = *it_accid;
      break;
    }
    ++it_accid;
  }

  // If we did not find the account, now we must create one.
  if (result.isEmpty()) {
    MyMoneyAccount newAccount;
    newAccount.setName(searchname);
    newAccount.setAccountType(eMyMoney::Account::Type::Expense);
    MyMoneyFileTransaction ft;
    MyMoneyAccount expense = file->expense();
    file->addAccount(newAccount, expense);
    ft.commit();
    result = newAccount.id();
  }

  return result;
}

const QString MyMoneyQifReader::processAccountEntry(bool resetAccountId)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyAccount account;
  QString tmp;

  account.setName(extractLine('N'));
  //   qDebug("Process account '%s'", account.name().data());

  account.setDescription(extractLine('D'));

  tmp = extractLine('$');
  if (tmp.length() > 0)
    account.setValue("lastStatementBalance", tmp);

  tmp = extractLine('/');
  if (tmp.length() > 0)
    account.setValue("lastStatementDate", m_qifProfile.date(tmp).toString("yyyy-MM-dd"));

  QifEntryTypeE transactionType = EntryTransaction;
  QString type = extractLine('T').toLower().remove(QRegExp("\\s+"));
  if (type == m_qifProfile.profileType().toLower().remove(QRegExp("\\s+"))) {
    account.setAccountType(eMyMoney::Account::Type::Checkings);
  } else if (type == "ccard" || type == "creditcard") {
    account.setAccountType(eMyMoney::Account::Type::CreditCard);
  } else if (type == "cash") {
    account.setAccountType(eMyMoney::Account::Type::Cash);
  } else if (type == "otha") {
    account.setAccountType(eMyMoney::Account::Type::Asset);
  } else if (type == "othl") {
    account.setAccountType(eMyMoney::Account::Type::Liability);
  } else if (type == "invst" || type == "port") {
    account.setAccountType(eMyMoney::Account::Type::Investment);
    transactionType = EntryInvestmentTransaction;
  } else if (type == "mutual") { // stock account w/o umbrella investment account
    account.setAccountType(eMyMoney::Account::Type::Stock);
    transactionType = EntryInvestmentTransaction;
  } else if (type == "unknown") {
    // don't do anything with the type, leave it unknown
  } else {
    account.setAccountType(eMyMoney::Account::Type::Checkings);
    qDebug() << "Line " << m_linenumber << ": Unknown account type '" << type << "', checkings assumed";
  }

  // check if we can find the account already in the file
  auto acc = findAccount(account, MyMoneyAccount());
  if (acc.id().isEmpty()) {
    // in case the account is not found by name and the type is
    // unknown, we have to assume something and create a checking account.
    // this might be wrong, but we have no choice at this point.
    if (account.accountType() == eMyMoney::Account::Type::Unknown)
      account.setAccountType(eMyMoney::Account::Type::Checkings);

    MyMoneyAccount parentAccount;
    MyMoneyAccount brokerage;
    // in case it's a stock account, we need to setup a fix investment account
    if (account.isInvest()) {
      acc.setName(i18n("%1 (Investment)", account.name()));   // use the same name for the investment account
      acc.setDescription(i18n("Autogenerated by QIF importer from type Mutual account entry"));
      acc.setAccountType(eMyMoney::Account::Type::Investment);
      parentAccount = file->asset();
      file->createAccount(acc, parentAccount, brokerage, MyMoneyMoney());
      parentAccount = acc;
      qDebug("We still need to create the stock account in MyMoneyQifReader::processAccountEntry()");
    } else {
      // setup parent according the type of the account
      switch (account.accountGroup()) {
        case eMyMoney::Account::Type::Asset:
        default:
          parentAccount = file->asset();
          break;
        case eMyMoney::Account::Type::Liability:
          parentAccount = file->liability();
          break;
        case eMyMoney::Account::Type::Equity:
          parentAccount = file->equity();
          break;
      }
    }

    // investment accounts will receive a brokerage account, as KMyMoney
    // currently does not allow to store funds in the investment account directly
    // but only create it (not here, but later) if it is needed
    if (account.accountType() == eMyMoney::Account::Type::Investment) {
      brokerage.setName(QString());  //                           brokerage name empty so account not created yet
      brokerage.setAccountType(eMyMoney::Account::Type::Checkings);
      brokerage.setCurrencyId(MyMoneyFile::instance()->baseCurrency().id());
    }
    file->createAccount(account, parentAccount, brokerage, MyMoneyMoney());
    acc = account;
    // qDebug("Account created");
  } else {
    // qDebug("Existing account found");
  }

  if (resetAccountId) {
    // possibly start a new statement
    d->finishStatement();
    m_account = acc;
    d->st.m_accountId = m_account.id();  //                      needed here for account selection
    d->transactionType = transactionType;
  }
  return acc.id();
}

void MyMoneyQifReader::setProgressCallback(void(*callback)(qint64, qint64, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyQifReader::signalProgress(qint64 current, qint64 total, const QString& msg)
{
  if (m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

void MyMoneyQifReader::processPriceEntry()
{
  /*
    !Type:Prices
    "IBM",141 9/16,"10/23/98"
    ^
    !Type:Prices
    "GMW",21.28," 3/17' 5"
    ^
    !Type:Prices
    "GMW",71652181.001,"67/128/ 0"
    ^

    Note that Quicken will often put in a price with a bogus date and number.  We will ignore
    prices with bogus dates.  Hopefully that will catch all of these.

    Also note that prices can be in fractional units, e.g. 141 9/16.

  */

  QStringList::const_iterator it_line = m_qifEntry.constBegin();

  // Make a price for each line
  QRegExp priceExp("\"(.*)\",(.*),\"(.*)\"");
  while (it_line != m_qifEntry.constEnd()) {
    if (priceExp.indexIn(*it_line) != -1) {
      MyMoneyStatement::Price price;
      price.m_strSecurity = priceExp.cap(1);
      QString pricestr = priceExp.cap(2);
      QString datestr = priceExp.cap(3);
      qDebug() << "Price:" << price.m_strSecurity << " / " << pricestr << " / " << datestr;

      // Only add the price if the date is valid.  If invalid, fail silently.  See note above.
      // Also require the price value to not have any slashes.  Old prices will be something like
      // "25 9/16", which we do not support.  So we'll skip the price for now.
      QDate date = m_qifProfile.date(datestr);
      MyMoneyMoney rate(m_qifProfile.value('P', pricestr));
      if (date.isValid() && !rate.isZero()) {
        price.m_amount = rate;
        price.m_date = date;
        d->st.m_listPrices += price;
      }
    }
    ++it_line;
  }
}

void MyMoneyQifReader::processSecurityEntry()
{
  /*
  !Type:Security
  NVANGUARD 500 INDEX
  SVFINX
  TMutual Fund
  ^
  */

  MyMoneyStatement::Security security;
  security.m_strName = extractLine('N');
  security.m_strSymbol = extractLine('S');

  d->st.m_listSecurities += security;
}
