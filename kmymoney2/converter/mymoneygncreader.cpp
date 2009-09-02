/***************************************************************************
                      mymoneygncreader  -  description
                         -------------------
begin                : Wed Mar 3 2004
copyright            : (C) 2000-2004 by Michael Edwardes
email                : mte@users.sourceforge.net
                       Javier Campos Morales <javi_c@users.sourceforge.net>
                       Felix Rodriguez <frodriguez@users.sourceforge.net>
                       John C <thetacoturtle@users.sourceforge.net>
                       Thomas Baumgart <ipwizard@users.sourceforge.net>
                       Kevin Tambascio <ktambascio@users.sourceforge.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "mymoneygncreader.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QFile>
#include <QMap>
#include <QObject>
#include <QDateTime>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <kurl.h>
#ifndef _GNCFILEANON
  #include <klocale.h>
  #include <kconfig.h>
  #include <kmessagebox.h>
#endif

// ----------------------------------------------------------------------------
// Third party Includes

// ------------------------------------------------------------Box21----------------
// Project Includes
#ifndef _GNCFILEANON
  #include "config-kmymoney.h"
  #include "storage/imymoneystorage.h"
  #include "kmymoneyutils.h"
  #include "mymoneyfile.h"
  #include "mymoneyprice.h"
  #include "kgncimportoptionsdlg.h"
  #include "kgncpricesourcedlg.h"
  #include "keditscheduledlg.h"
  #include "kmymoneyedit.h"
  #define TRY try {
  #define CATCH } catch (MyMoneyException *e) {
  #define PASS } catch (MyMoneyException *e) { throw e; }
#else
  #include "mymoneymoney.h"
  #include <QTextEdit>
  #define i18n QObject::tr
  #define TRY
  #define CATCH
  #define PASS
  #define MYMONEYEXCEPTION QString
  #define MyMoneyException QString
  #define PACKAGE "KMyMoney"
#endif // _GNCFILEANON

// init static variables
double MyMoneyGncReader::m_fileHideFactor = 0.0;
double GncObject::m_moneyHideFactor;

// user options
void MyMoneyGncReader::setOptions () {
#ifndef _GNCFILEANON
  KGncImportOptionsDlg dlg; // display the dialog to allow the user to set own options
  if (dlg.exec()) {
    // set users input options
    m_dropSuspectSchedules = dlg.scheduleOption();
    m_investmentOption = dlg.investmentOption();
    m_useFinanceQuote = dlg.quoteOption();
    m_useTxNotes = dlg.txNotesOption();
    m_decoder = dlg.decodeOption();
    gncdebug = dlg.generalDebugOption();
    xmldebug = dlg.xmlDebugOption();
    bAnonymize = dlg.anonymizeOption();
  } else {
    // user declined, so set some sensible defaults
    m_dropSuspectSchedules = false;
    // investment option - 0, create investment a/c per stock a/c, 1 = single new investment account, 2 = prompt for each stock
    // option 2 doesn't really work too well at present
    m_investmentOption = 0;
    m_useFinanceQuote = false;
    m_useTxNotes = false;
    m_decoder = 0;
    gncdebug = false; // general debug messages
    xmldebug = false; // xml trace
    bAnonymize = false; // anonymize input
  }
  // no dialog option for the following; it will set base currency, and print actual XML data
  developerDebug = false;
  // set your fave currency here to save getting that enormous dialog each time you run a test
  // especially if you have to scroll down to USD...
  if (developerDebug) m_storage->setValue ("kmm-baseCurrency", "GBP");
#endif // _GNCFILEANON
}

GncObject::GncObject () {
}

// Check that the current element is of a version we are coded for
void GncObject::checkVersion (const QString& elName, const QXmlAttributes& elAttrs, const map_elementVersions& map) {
  TRY
  if (map.contains(elName)) { // if it's not in the map, there's nothing to check
    if (!map[elName].contains(elAttrs.value("version"))) {
      QString em = __func__ +i18n(": Sorry. This importer cannot handle version %1 of element %2"
                          ,elAttrs.value("version"),elName);
      throw new MYMONEYEXCEPTION (em);
    }
  }
  return ;
  PASS
}

// Check if this element is in the current object's sub element list
GncObject *GncObject::isSubElement (const QString& elName, const QXmlAttributes& elAttrs) {
  TRY
  uint i;
  GncObject *next = 0;
  for (i = 0; i < m_subElementListCount; i++) {
    if (elName == m_subElementList[i]) {
      m_state = i;
      next = startSubEl(); // go create the sub object
      if (next != 0) {
        next->initiate(elName, elAttrs); // initialize it
        next->m_elementName = elName;    // save it's name so we can identify the end
      }
      break;
    }
  }
  return (next);
  PASS
}

// Check if this element is in the current object's data element list
bool GncObject::isDataElement (const QString &elName, const QXmlAttributes& elAttrs) {
  TRY
  uint i;
  for (i = 0; i < m_dataElementListCount; i++) {
    if (elName == m_dataElementList[i]) {
      m_state = i;
      dataEl(elAttrs); // go set the pointer so the data can be stored
      return (true);
    }
  }
  m_dataPtr = 0; // we don't need this, so make sure we don't store extraneous data
  return (false);
  PASS
}

// return the variable string, decoded if required
QString GncObject::var (int i) const {
#warning "port to kde4"
#if 0
      	return (pMain->m_decoder == 0
      ? *(m_v.at(i))
      : pMain->m_decoder->toUnicode (*(m_v.at(i))));
#endif
        return m_v[i];
}

void GncObject::adjustHideFactor () {
  m_moneyHideFactor = pMain->m_fileHideFactor * (1.0 + (int)(200.0 * rand()/(RAND_MAX+1.0))) / 100.0;
}

// data anonymizer
QString GncObject::hide (QString data, unsigned int anonClass) {
  TRY
  if (!pMain->bAnonymize) return (data); // no anonymizing required
  // counters used to generate names for anonymizer
  static int nextAccount;
  static int nextEquity;
  static int nextPayee;
  static int nextSched;
  static QMap<QString, QString> anonPayees; // to check for duplicate payee names
  static QMap<QString, QString> anonStocks; // for reference to equities

  QString result (data);
  QMap<QString, QString>::const_iterator it;
  MyMoneyMoney in, mresult;
  switch (anonClass) {
  case ASIS: break;                  // this is not personal data
  case SUPPRESS: result = ""; break; // this is personal and is not essential
  case NXTACC: result = i18n("Account%1").arg(++nextAccount, -6); break; // generate account name
  case NXTEQU:   // generate/return an equity name
    it = anonStocks.find (data);
    if (it == anonStocks.end()) {
      result = i18n("Stock%1").arg(++nextEquity, -6);
      anonStocks.insert (data, result);
    } else {
      result = (*it);
    }
    break;
  case NXTPAY:   // genearet/return a payee name
    it = anonPayees.find (data);
    if (it == anonPayees.end()) {
      result = i18n("Payee%1").arg(++nextPayee, -6);
      anonPayees.insert (data, result);
    } else {
      result = (*it);
    }
    break;
  case NXTSCHD: result = i18n("Schedule%1").arg(++nextSched, -6); break; // generate a schedule name
  case MONEY1:
    in = MyMoneyMoney(data);
    if (data == "-1/0") in = MyMoneyMoney (0); // spurious gnucash data - causes a crash sometimes
    mresult = MyMoneyMoney(m_moneyHideFactor) * in;
    mresult.convert(10000);
    result = mresult.toString();
    break;
  case MONEY2:
    in = MyMoneyMoney(data);
    if (data == "-1/0") in = MyMoneyMoney (0);
    mresult  = MyMoneyMoney(m_moneyHideFactor) * in;
    mresult.convert(10000);
    mresult.setThousandSeparator (' ');
    result = mresult.formatMoney("", 2);
    break;
  }
  return (result);
  PASS
}

// dump current object data values // only called if gncdebug set
void GncObject::debugDump () {
  uint i;
  qDebug () << "Object" << m_elementName;
  for (i = 0; i < m_dataElementListCount; i++) {
    qDebug () << m_dataElementList[i] << "=" << m_v[i];
  }
}
//*****************************************************************
GncFile::GncFile () {
  static const QString subEls[] = {"gnc:book", "gnc:count-data", "gnc:commodity", "price",
                                   "gnc:account", "gnc:transaction", "gnc:template-transactions",
                                   "gnc:schedxaction"
                                  };
  m_subElementList = subEls;
  m_subElementListCount = END_FILE_SELS;
  m_dataElementListCount = 0;
  m_processingTemplates = false;
  m_bookFound = false;
}

GncFile::~GncFile () {}

GncObject *GncFile::startSubEl() {
  TRY
  if (pMain->xmldebug) qDebug ("File start subel m_state %d", m_state);
  GncObject *next = 0;
  switch (m_state) {
  case BOOK:
    if (m_bookFound) throw new MYMONEYEXCEPTION (i18n("This version of the importer cannot handle multi-book files."));
    m_bookFound = true;
    break;
  case COUNT: next = new GncCountData; break;
  case CMDTY: next = new GncCommodity; break;
  case PRICE: next = new GncPrice; break;
  case ACCT:
    // accounts within the template section are ignored
    if (!m_processingTemplates) next = new GncAccount;
    break;
  case TX: next = new GncTransaction (m_processingTemplates); break;
  case TEMPLATES: m_processingTemplates = true; break;
  case SCHEDULES: m_processingTemplates = false; next = new GncSchedule; break;
  default: throw new MYMONEYEXCEPTION ("GncFile rcvd invalid state");
  }
  return (next);
  PASS
}

void GncFile::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("File end subel");
  if (!m_processingTemplates) delete subObj; // template txs must be saved awaiting schedules
  m_dataPtr = 0;
  return ;
}
//****************************************** GncDate *********************************************
GncDate::GncDate () {
  m_subElementListCount = 0;
  static const QString dEls[] = {"ts:date", "gdate"};
  m_dataElementList = dEls;
  m_dataElementListCount = END_Date_DELS;
  static const unsigned int anonClasses[] = {ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
}

GncDate::~GncDate() {}
//*************************************GncCmdtySpec***************************************
GncCmdtySpec::GncCmdtySpec () {
  m_subElementListCount = 0;
  static const QString dEls[] = {"cmdty:space", "cmdty:id"};
  m_dataElementList = dEls;
  m_dataElementListCount = END_CmdtySpec_DELS;
  static const unsigned int anonClasses[] = {ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
}

GncCmdtySpec::~GncCmdtySpec () {}

QString GncCmdtySpec::hide(QString data, unsigned int) {
  // hide equity names, but not currency names
  unsigned int newClass = ASIS;
  switch (m_state) {
  case CMDTYID:
    if (!isCurrency()) newClass = NXTEQU;
  }
  return (GncObject::hide (data, newClass));
}
//************* GncKvp********************************************
GncKvp::GncKvp () {
  m_subElementListCount = END_Kvp_SELS;
  static const QString subEls[] = {"slot"}; // kvp's may be nested
  m_subElementList = subEls;
  m_dataElementListCount = END_Kvp_DELS;
  static const QString dataEls[] = {"slot:key", "slot:value"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
}

GncKvp::~GncKvp () {}

void GncKvp::dataEl (const QXmlAttributes& elAttrs) {
  switch (m_state) {
  case VALUE:
    m_kvpType = elAttrs.value("type");
  }
  m_dataPtr = &(m_v[m_state]);
  if (key().contains ("formula")) {
    m_anonClass = MONEY2;
  } else {
    m_anonClass = ASIS;
  }
  return ;
}

GncObject *GncKvp::startSubEl() {
  if (pMain->xmldebug) qDebug ("Kvp start subel m_state %d", m_state);
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case KVP: next = new GncKvp; break;
  default: throw new MYMONEYEXCEPTION ("GncKvp rcvd invalid m_state ");
  }
  return (next);
  PASS
}

void GncKvp::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Kvp end subel");
  m_kvpList.append (subObj);
  m_dataPtr = 0;
  return ;
}
//*********************************GncLot*********************************************
GncLot::GncLot() {
  m_subElementListCount = 0;
  m_dataElementListCount = 0;
}

GncLot::~GncLot() {}

//*********************************GncCountData***************************************
GncCountData::GncCountData() {
  m_subElementListCount = 0;
  m_dataElementListCount = 0;
  m_v.append (QString()); // only 1 data item
}

GncCountData::~GncCountData () {}

void GncCountData::initiate (const QString&, const QXmlAttributes& elAttrs) {
  m_countType = elAttrs.value ("cd:type");
  m_dataPtr = &(m_v[0]);
  return ;
}

void GncCountData::terminate () {
  int i = m_v[0].toInt();
  if (m_countType == "commodity") {
    pMain->setGncCommodityCount(i); return ;
  }
  if (m_countType == "account") {
    pMain->setGncAccountCount(i); return ;
  }
  if (m_countType == "transaction") {
    pMain->setGncTransactionCount(i); return ;
  }
  if (m_countType == "schedxaction") {
    pMain->setGncScheduleCount(i); return ;
  }
  if (i != 0) {
    if (m_countType == "budget") pMain->setBudgetsFound(true);
    else if (m_countType.left(7) == "gnc:Gnc") pMain->setSmallBusinessFound(true);
    else if (pMain->xmldebug) qDebug ("Unknown count type %s", qPrintable(m_countType));
  }
  return ;
}
//*********************************GncCommodity***************************************
GncCommodity::GncCommodity () {
  m_subElementListCount = 0;
  static const QString dEls[] = {"cmdty:space", "cmdty:id", "cmdty:name", "cmdty:fraction"};
  m_dataElementList = dEls;
  m_dataElementListCount = END_Commodity_DELS;
  static const unsigned int anonClasses[] = {ASIS, NXTEQU, SUPPRESS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
}

GncCommodity::~GncCommodity () {}

void GncCommodity::terminate() {
  TRY
  pMain->convertCommodity (this);
  return ;
  PASS
}
//************* GncPrice********************************************
GncPrice::GncPrice () {
  static const QString subEls[] = {"price:commodity", "price:currency", "price:time"};
  m_subElementList = subEls;
  m_subElementListCount = END_Price_SELS;
  m_dataElementListCount = END_Price_DELS;
  static const QString dataEls[] = {"price:value"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
  m_vpCommodity = NULL;
  m_vpCurrency = NULL;
  m_vpPriceDate = NULL;
}

GncPrice::~GncPrice () {
  delete m_vpCommodity; delete m_vpCurrency; delete m_vpPriceDate;
}

GncObject *GncPrice::startSubEl() {
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case CMDTY: next = new GncCmdtySpec; break;
  case CURR: next = new GncCmdtySpec; break;
  case PRICEDATE: next = new GncDate; break;
  default: throw new MYMONEYEXCEPTION ("GncPrice rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncPrice::endSubEl(GncObject *subObj) {
  TRY
  switch (m_state) {
  case CMDTY: m_vpCommodity = static_cast<GncCmdtySpec *>(subObj); break;
  case CURR: m_vpCurrency = static_cast<GncCmdtySpec *>(subObj); break;
  case PRICEDATE: m_vpPriceDate = static_cast<GncDate *>(subObj); break;
  default: throw new MYMONEYEXCEPTION ("GncPrice rcvd invalid m_state");
  }
  return;
  PASS
}

void GncPrice::terminate() {
  TRY
  pMain->convertPrice (this);
  return ;
  PASS
}
//************* GncAccount********************************************
GncAccount::GncAccount () {
  m_subElementListCount = END_Account_SELS;
  static const QString subEls[] = {"act:commodity", "slot", "act:lots"};
  m_subElementList = subEls;
  m_dataElementListCount = END_Account_DELS;
  static const QString dataEls[] = {"act:id", "act:name", "act:description",
                                    "act:type", "act:parent"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, NXTACC, SUPPRESS, ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
  m_vpCommodity = NULL;
}

GncAccount::~GncAccount () {
  delete m_vpCommodity;
}

GncObject *GncAccount::startSubEl() {
  TRY
  if (pMain->xmldebug) qDebug ("Account start subel m_state %d", m_state);
  GncObject *next = 0;
  switch (m_state) {
  case CMDTY: next = new GncCmdtySpec; break;
  case KVP: next = new GncKvp; break;
  case LOTS: next = new GncLot();
             pMain->setLotsFound(true); // we don't handle lots; just set flag to report
             break;
  default: throw new MYMONEYEXCEPTION ("GncAccount rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncAccount::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Account end subel");
  switch (m_state) {
  case CMDTY: m_vpCommodity = static_cast<GncCmdtySpec *>(subObj); break;
  case KVP: m_kvpList.append (subObj);
  }
  return ;
}

void GncAccount::terminate() {
  TRY
  pMain->convertAccount (this);
  return ;
  PASS
}
//************* GncTransaction********************************************
GncTransaction::GncTransaction (bool processingTemplates) {
  m_subElementListCount = END_Transaction_SELS;
  static const QString subEls[] = {"trn:currency", "trn:date-posted", "trn:date-entered",
    "trn:split", "slot"};
  m_subElementList = subEls;
  m_dataElementListCount = END_Transaction_DELS;
  static const QString dataEls[] = {"trn:id", "trn:num", "trn:description"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, SUPPRESS, NXTPAY};
  m_anonClassList = anonClasses;
  adjustHideFactor();
  m_template = processingTemplates;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
  m_vpCurrency = NULL;
  m_vpDateEntered = m_vpDatePosted = NULL;
}

GncTransaction::~GncTransaction () {
  delete m_vpCurrency; delete m_vpDatePosted; delete m_vpDateEntered;
}

GncObject *GncTransaction::startSubEl() {
  TRY
  if (pMain->xmldebug) qDebug ("Transaction start subel m_state %d", m_state);
  GncObject *next = 0;
  switch (m_state) {
  case CURRCY: next = new GncCmdtySpec; break;
  case POSTED:
  case ENTERED:
    next = new GncDate; break;
  case SPLIT:
    if (isTemplate()) {
      next = new GncTemplateSplit;
    } else {
      next = new GncSplit;
    }
    break;
  case KVP: next = new GncKvp; break;
  default: throw new MYMONEYEXCEPTION ("GncTransaction rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncTransaction::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Transaction end subel");
  switch (m_state) {
  case CURRCY: m_vpCurrency = static_cast<GncCmdtySpec *>(subObj); break;
  case POSTED: m_vpDatePosted = static_cast<GncDate *>(subObj); break;
  case ENTERED: m_vpDateEntered = static_cast<GncDate *>(subObj); break;
  case SPLIT: m_splitList.append (subObj); break;
  case KVP: m_kvpList.append (subObj);
  }
  return ;
}

void GncTransaction::terminate() {
  TRY
  if (isTemplate()) {
    pMain->saveTemplateTransaction(this);
  } else {
    pMain->convertTransaction (this);
  }
  return ;
  PASS
}
//************* GncSplit********************************************
GncSplit::GncSplit () {
  m_subElementListCount = END_Split_SELS;
  static const QString subEls[] = {"split:reconcile-date"};
  m_subElementList = subEls;
  m_dataElementListCount = END_Split_DELS;
  static const QString dataEls[] = {"split:id", "split:memo", "split:reconciled-state", "split:value",
                                    "split:quantity", "split:account"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, SUPPRESS, ASIS, MONEY1, MONEY1, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
  m_vpDateReconciled = NULL;
}

GncSplit::~GncSplit () {
  delete m_vpDateReconciled;
}

GncObject *GncSplit::startSubEl () {
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case RECDATE: next = new GncDate; break;
  default: throw new MYMONEYEXCEPTION ("GncTemplateSplit rcvd invalid m_state ");
  }
  return (next);
  PASS
}

void GncSplit::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Split end subel");
  switch (m_state) {
  case RECDATE: m_vpDateReconciled = static_cast<GncDate *>(subObj); break;
  }
  return ;
}
//************* GncTemplateSplit********************************************
GncTemplateSplit::GncTemplateSplit () {
  m_subElementListCount = END_TemplateSplit_SELS;
  static const QString subEls[] = {"slot"};
  m_subElementList = subEls;
  m_dataElementListCount = END_TemplateSplit_DELS;
  static const QString dataEls[] = {"split:id", "split:memo", "split:reconciled-state", "split:value",
                                    "split:quantity", "split:account"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, SUPPRESS, ASIS, MONEY1, MONEY1, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
}

GncTemplateSplit::~GncTemplateSplit () {}

GncObject *GncTemplateSplit::startSubEl() {
  if (pMain->xmldebug) qDebug ("TemplateSplit start subel m_state %d", m_state);
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case KVP: next = new GncKvp; break;
  default: throw new MYMONEYEXCEPTION ("GncTemplateSplit rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncTemplateSplit::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("TemplateSplit end subel");
  m_kvpList.append (subObj);
  m_dataPtr = 0;
  return ;
}
//************* GncSchedule********************************************
GncSchedule::GncSchedule () {
  m_subElementListCount = END_Schedule_SELS;
  static const QString subEls[] = {"sx:start", "sx:last", "sx:end", "gnc:freqspec", "gnc:recurrence","sx:deferredInstance"};
  m_subElementList = subEls;
  m_dataElementListCount = END_Schedule_DELS;
  static const QString dataEls[] = {"sx:name", "sx:enabled", "sx:autoCreate", "sx:autoCreateNotify",
                                    "sx:autoCreateDays", "sx:advanceCreateDays", "sx:advanceRemindDays",
                                    "sx:instanceCount", "sx:num-occur",
                                    "sx:rem-occur", "sx:templ-acct"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {NXTSCHD, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
  m_vpStartDate = m_vpLastDate = m_vpEndDate = NULL;
  m_vpFreqSpec = NULL;
  m_vpRecurrence.clear();
  m_vpSchedDef = NULL;
}

GncSchedule::~GncSchedule () {
  delete m_vpStartDate; delete m_vpLastDate; delete m_vpEndDate; delete m_vpFreqSpec; delete m_vpSchedDef;
}

GncObject *GncSchedule::startSubEl() {
  if (pMain->xmldebug) qDebug ("Schedule start subel m_state %d", m_state);
  TRY
  GncObject *next = 0;
  switch (m_state) {
  case STARTDATE:
  case LASTDATE:
  case ENDDATE: next = new GncDate; break;
  case FREQ: next = new GncFreqSpec; break;
  case RECURRENCE: next = new GncRecurrence; break;
  case DEFINST: next = new GncSchedDef; break;
  default: throw new MYMONEYEXCEPTION ("GncSchedule rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncSchedule::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Schedule end subel");
  switch (m_state) {
  case STARTDATE: m_vpStartDate = static_cast<GncDate *>(subObj); break;
  case LASTDATE: m_vpLastDate = static_cast<GncDate *>(subObj); break;
  case ENDDATE: m_vpEndDate = static_cast<GncDate *>(subObj); break;
  case FREQ: m_vpFreqSpec = static_cast<GncFreqSpec *>(subObj); break;
  case RECURRENCE: m_vpRecurrence.append(static_cast<GncRecurrence *>(subObj)); break;
  case DEFINST: m_vpSchedDef = static_cast<GncSchedDef *>(subObj); break;
  }
  return ;
}

void GncSchedule::terminate() {
  TRY
  pMain->convertSchedule (this);
  return ;
  PASS
}
//************* GncFreqSpec********************************************
GncFreqSpec::GncFreqSpec () {
  m_subElementListCount = END_FreqSpec_SELS;
  static const QString subEls[] = {"gnc:freqspec"};
  m_subElementList = subEls;
  m_dataElementListCount = END_FreqSpec_DELS;
  static const QString dataEls[] = {"fs:ui_type", "fs:monthly", "fs:daily", "fs:weekly", "fs:interval",
                                    "fs:offset", "fs:day"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, ASIS, ASIS, ASIS, ASIS, ASIS, ASIS      };
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
}

GncFreqSpec::~GncFreqSpec () {}

GncObject *GncFreqSpec::startSubEl() {
  TRY
  if (pMain->xmldebug) qDebug ("FreqSpec start subel m_state %d", m_state);

  GncObject *next = 0;
  switch (m_state) {
  case COMPO: next = new GncFreqSpec; break;
  default: throw new MYMONEYEXCEPTION ("GncFreqSpec rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncFreqSpec::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("FreqSpec end subel");
  switch (m_state) {
  case COMPO: m_fsList.append (subObj); break;
  }
  m_dataPtr = 0;
  return ;
}

void GncFreqSpec::terminate() {
  pMain->convertFreqSpec (this);
  return ;
}
//************* GncRecurrence********************************************
GncRecurrence::GncRecurrence () {
  m_subElementListCount = END_Recurrence_SELS;
  static const QString subEls[] = {"recurrence:start"};
  m_subElementList = subEls;
  m_dataElementListCount = END_Recurrence_DELS;
  static const QString dataEls[] = {"recurrence:mult", "recurrence:period_type"};
  m_dataElementList = dataEls;
  static const unsigned int anonClasses[] = {ASIS, ASIS};
  m_anonClassList = anonClasses;
  for (uint i = 0; i < m_dataElementListCount; i++) m_v.append (QString());
}

GncRecurrence::~GncRecurrence () {
  delete m_vpStartDate;
}

GncObject *GncRecurrence::startSubEl() {
  TRY
  if (pMain->xmldebug) qDebug ("Recurrence start subel m_state %d", m_state);

  GncObject *next = 0;
  switch (m_state) {
  case STARTDATE: next = new GncDate; break;
  default: throw new MYMONEYEXCEPTION ("GncRecurrence rcvd invalid m_state");
  }
  return (next);
  PASS
}

void GncRecurrence::endSubEl(GncObject *subObj) {
  if (pMain->xmldebug) qDebug ("Recurrence end subel");
  switch (m_state) {
  case STARTDATE: m_vpStartDate = static_cast<GncDate *>(subObj); break;
  }
  m_dataPtr = 0;
  return ;
}

void GncRecurrence::terminate() {
  pMain->convertRecurrence (this);
  return ;
}

QString GncRecurrence::getFrequency() const {
  // This function converts a gnucash 2.2 recurrence specification into it's previous equivalent
  // This will all need re-writing when MTE finishes the schedule re-write
  if (periodType() == "once") return("once");
  if ((periodType() == "day") and (mult() == "1")) return("daily");
  if (periodType() == "week") {
    if (mult() == "1") return ("weekly");
    if (mult() == "2") return ("bi_weekly");
    if (mult() == "4") return ("four-weekly");
  }
  if (periodType() == "month") {
    if (mult() == "1") return ("monthly");
    if (mult() == "2") return ("two-monthly");
    if (mult() == "3") return ("quarterly");
    if (mult() == "4") return ("tri_annually");
    if (mult() == "6") return ("semi_yearly");
    if (mult() == "12") return ("yearly");
    if (mult() == "24") return ("two-yearly");
  }
  return ("unknown");
}

//************* GncSchedDef********************************************
GncSchedDef::GncSchedDef () {
  // process ing for this sub-object is undefined at the present time
  m_subElementListCount = 0;
  m_dataElementListCount = 0;
}

GncSchedDef::~GncSchedDef () {}

/************************************************************************************************
                         XML Reader
************************************************************************************************/
void XmlReader::processFile (QIODevice* pDevice) {
  m_source = new QXmlInputSource (pDevice); // set up the Qt XML reader
  m_reader = new QXmlSimpleReader;
  m_reader->setContentHandler (this);
  // go read the file
  if (!m_reader->parse (m_source)) {
    throw new MYMONEYEXCEPTION (i18n("Input file cannot be parsed; may be corrupt\n%1", errorString()));
  }
  delete m_reader;
  delete m_source;
  return ;
}

// XML handling routines
bool XmlReader::startDocument() {
  m_co = new GncFile; // create initial object, push to stack , pass it the 'main' pointer
  m_os.push (m_co);
  m_co->setPm (pMain);
  m_headerFound = false;
#ifdef _GNCFILEANON
  pMain->oStream << "<?xml version=\"1.0\"?>";
  lastType = -1;
  indentCount = 0;
#endif // _GNCFILEANON
   return (true);
}

bool XmlReader::startElement (const QString&, const QString&, const QString& elName ,
                              const QXmlAttributes& elAttrs) {
  try {
    if (pMain->gncdebug) qDebug ("XML start - %s", qPrintable(elName));
#ifdef _GNCFILEANON
    int i;
    QString spaces;
    // anonymizer - write data
    if (elName == "gnc:book" || elName == "gnc:count-data" || elName == "book:id") lastType = -1;
    pMain->oStream << endl;
    switch (lastType) {
      case 0:
        indentCount += 2;
        // tricky fall through here

      case 2:
        spaces.fill (' ', indentCount);
        pMain->oStream << spaces.toLatin1();
        break;
    }
    pMain->oStream << '<' << elName;
    for (i = 0; i < elAttrs.count(); ++i) {
      pMain->oStream << ' ' << elAttrs.qName(i) << '='  << '"' << elAttrs.value(i) << '"';
    }
    pMain->oStream << '>';
    lastType = 0;
#else
    if ((!m_headerFound) && (elName != "gnc-v2"))
      throw new MYMONEYEXCEPTION (i18n("Invalid header for file. Should be 'gnc-v2'"));
    m_headerFound = true;
#endif // _GNCFILEANON
    m_co->checkVersion (elName, elAttrs, pMain->m_versionList);
    // check if this is a sub object element; if so, push stack and initialize
    GncObject *temp = m_co->isSubElement (elName, elAttrs);
    if (temp != 0) {
      m_os.push (temp);
      m_co = m_os.top();
      m_co->setVersion(elAttrs.value("version"));
      m_co->setPm (pMain); // pass the 'main' pointer to the sub object
      // return true;   // removed, as we hit a return true anyway
    }
#if 0
    // check for a data element
    if (m_co->isDataElement (elName, elAttrs))
      return (true);
#endif
    else {
      // reduced the above to
      m_co->isDataElement(elName, elAttrs);
    }
  } catch (MyMoneyException *e) {
#ifndef _GNCFILEANON
    // we can't pass on exceptions here coz the XML reader won't catch them and we just abort
    KMessageBox::error(0, i18n("Import failed:\n\n%1",e->what()), PACKAGE);
    qFatal ("%s", qPrintable(e->what()));
#else
    qFatal ("%s", e->toLatin1());
#endif // _GNCFILEANON
  }
  return true; // to keep compiler happy
}

bool XmlReader::endElement( const QString&, const QString&, const QString&elName ) {
  try {
    if (pMain->xmldebug) qDebug ("XML end - %s", qPrintable(elName));
#ifdef _GNCFILEANON
    QString spaces;
    switch (lastType) {
    case 2:
      indentCount -= 2; spaces.fill (' ', indentCount); pMain->oStream << endl << spaces.toLatin1(); break;
    }
    pMain->oStream << "</" << elName << '>' ;
    lastType = 2;
#endif // _GNCFILEANON
    m_co->resetDataPtr(); // so we don't get extraneous data loaded into the variables
    if (elName == m_co->getElName()) { // check if this is the end of the current object
      if (pMain->gncdebug) m_co->debugDump(); // dump the object data (temp)
      // call the terminate routine, pop the stack, and advise the parent that it's done
      m_co->terminate();
      GncObject *temp = m_co;
      m_os.pop();
      m_co = m_os.top();
      m_co->endSubEl (temp);
    }
    return (true);
  } catch (MyMoneyException *e) {
#ifndef _GNCFILEANON
    // we can't pass on exceptions here coz the XML reader won't catch them and we just abort
    KMessageBox::error(0, i18n("Import failed:\n\n%1",e->what()), PACKAGE);
    qFatal (qPrintable(e->what()));
#else
    qFatal ("%s", e->toLatin1());
#endif // _GNCFILEANON
  }
  return (true); // to keep compiler happy
}

bool XmlReader::characters (const QString &data) {
  if (pMain->xmldebug) qDebug ("XML Data received - %d bytes", data.length());
  QString pData = data.trimmed(); // data may contain line feeds and indentation spaces
  if (!pData.isEmpty()) {
    if (pMain->developerDebug) qDebug ("XML Data - %s", qPrintable(pData));
    m_co->storeData (pData); //go store it
#ifdef _GNCFILEANON
    QString anonData = m_co->getData ();
    if (anonData.isEmpty()) anonData = pData;
    // there must be a Qt standard way of doing the following but I can't ... find it
    anonData.replace ('<', "&lt;");
    anonData.replace ('>', "&gt;");
    anonData.replace ('&', "&amp;");
    pMain->oStream << anonData; // write original data
    lastType = 1;
#endif // _GNCFILEANON
  }
  return (true);
}

bool XmlReader::endDocument() {
#ifdef _GNCFILEANON
  pMain->oStream << endl << endl;
  pMain->oStream << "<!-- Local variables: -->" << endl;
  pMain->oStream << "<!-- mode: xml        -->" << endl;
  pMain->oStream << "<!-- End:             -->" << endl;
#endif // _GNCFILEANON
  return (true);
}

/*******************************************************************************************
                                 Main class for this module
  Controls overall operation of the importer
********************************************************************************************/
//***************** Constructor ***********************
MyMoneyGncReader::MyMoneyGncReader() {
#ifndef _GNCFILEANON
  m_storage = NULL;
#endif // _GNCFILEANON
// to hold gnucash count data (only used for progress bar)
  m_gncCommodityCount = m_gncAccountCount = m_gncTransactionCount = m_gncScheduleCount = 0;
  m_smallBusinessFound = m_budgetsFound = m_lotsFound = false;
  m_commodityCount = m_priceCount = m_accountCount = m_transactionCount = m_templateCount = m_scheduleCount = 0;
  m_decoder = 0;
  // build a list of valid versions
  static const QString versionList[] = {"gnc:book 2.0.0", "gnc:commodity 2.0.0", "gnc:pricedb 1",
                                        "gnc:account 2.0.0", "gnc:transaction 2.0.0", "gnc:schedxaction 1.0.0",
                                        "gnc:schedxaction 2.0.0", // for gnucash 2.2 onward
                                        "gnc:freqspec 1.0.0", "zzz" // zzz = stopper
                                       };
  unsigned int i;
  for (i = 0; versionList[i] != "zzz"; ++i)
    m_versionList[versionList[i].section (' ', 0, 0)].append(versionList[i].section (' ', 1, 1));
}

//***************** Destructor *************************
MyMoneyGncReader::~MyMoneyGncReader() {}

//**************************** Main Entry Point ************************************
#ifndef _GNCFILEANON
void MyMoneyGncReader::readFile(QIODevice* pDevice, IMyMoneySerialize* storage) {

  Q_CHECK_PTR (pDevice);
  Q_CHECK_PTR (storage);

  m_storage = dynamic_cast<IMyMoneyStorage *>(storage);
  qDebug ("Entering gnucash importer");
  setOptions ();
  // get a file anonymization factor from the user
  if (bAnonymize) setFileHideFactor ();
  //m_defaultPayee = createPayee (i18n("Unknown payee"));

  MyMoneyFileTransaction ft;
  m_xr = new XmlReader (this);
  try {
    m_xr->processFile (pDevice);
    terminate (); // do all the wind-up things
    ft.commit();
  } catch (MyMoneyException *e) {
    KMessageBox::error(0, i18n("Import failed:\n\n%1",e->what()), PACKAGE);
    qFatal (qPrintable( e->what()));
  } // end catch
  signalProgress (0, 1, i18n("Import complete")); // switch off progress bar
  delete m_xr;
  qDebug ("Exiting gnucash importer");
  return ;
}
#else
// Control code for the file anonymizer
void MyMoneyGncReader::readFile(QString in, QString out) {
  QFile pDevice (in);
  if (!pDevice.open (QIODevice::ReadOnly)) qFatal ("Can't open input file");
  QFile outFile (out);
  if (!outFile.open (QIODevice::WriteOnly)) qFatal ("Can't open output file");
  oStream.setDevice (&outFile);
  bAnonymize = true;
  // get a file anonymization factor from the user
  setFileHideFactor ();
  m_xr = new XmlReader (this);
  try {
    m_xr->processFile (&pDevice);
  } catch (MyMoneyException *e) {
    qFatal ("%s", e->toLatin1());
  } // end catch
  delete m_xr;
  pDevice.close();
  outFile.close();
  return ;
}

#include <QApplication>
int main (int argc, char ** argv) {
    QApplication a (argc, argv);
    MyMoneyGncReader m;
    QString inFile, outFile;

    if (argc > 0) inFile = a.argv()[1];
    if (argc > 1) outFile = a.argv()[2];
    if (inFile.isEmpty()) {
        inFile = KFileDialog::getOpenFileName("",
                    "Gnucash files(*.nc *)",
                    0);
    }
    if (inFile.isEmpty()) qFatal ("Input file required");
    if (outFile.isEmpty()) outFile = inFile + ".anon";
    m.readFile (inFile, outFile);
    exit (0);
}
#endif // _GNCFILEANON

void MyMoneyGncReader::setFileHideFactor () {
#define MINFILEHIDEF 0.01
#define MAXFILEHIDEF 99.99
    srand (QTime::currentTime().second()); // seed randomizer for anonymize
    m_fileHideFactor = 0.0;
    while (m_fileHideFactor == 0.0) {
      m_fileHideFactor = KInputDialog::getDouble (
        i18n ("Disguise your wealth"),
        i18n ("Each monetary value on your file will be multiplied by a random number between 0.01 and 1.99\n"
                     "with a different value used for each transaction. In addition, to further disguise the true\n"
                             "values, you may enter a number between %1 and %2 which will be applied to all values.\n"
                             "These numbers will not be stored in the file.",MINFILEHIDEF,MAXFILEHIDEF),
                (1.0 + (int)(1000.0 * rand() / (RAND_MAX + 1.0))) / 100.0,
        MINFILEHIDEF, MAXFILEHIDEF, 2);
    }
}
#ifndef _GNCFILEANON

//********************************* convertCommodity *******************************************
void MyMoneyGncReader::convertCommodity (const GncCommodity *gcm) {
  Q_CHECK_PTR (gcm);
  MyMoneySecurity equ;
  if (m_commodityCount == 0) signalProgress (0, m_gncCommodityCount, i18n("Loading commodities..."));
  if (!gcm->isCurrency()) { // currencies should not be present here but...
    equ.setName (gcm->name());
    equ.setTradingSymbol (gcm->id());
    equ.setTradingMarket (gcm->space()); // the 'space' may be market or quote source, dep on what the user did
    // don't set the source here since he may not want quotes
    //equ.setValue ("kmm-online-source", gcm->space()); // we don't know, so use it as both
    equ.setTradingCurrency (""); // not available here, will set from pricedb or transaction
    equ.setSecurityType (MyMoneySecurity::SECURITY_STOCK); // default to it being a stock
    //tell the storage objects we have a new equity object.
    equ.setSmallestAccountFraction(gcm->fraction().toInt());
    m_storage->addSecurity(equ);

    //assign the gnucash id as the key into the map to find our id
    if (gncdebug) qDebug ("mapping, key = %s, id = %s", qPrintable(gcm->id()), qPrintable(equ.id()));
    m_mapEquities[gcm->id().toUtf8()] = equ.id();
  }
  signalProgress (++m_commodityCount, 0);
  return ;
}

//******************************* convertPrice ************************************************
void MyMoneyGncReader::convertPrice (const GncPrice *gpr) {
  Q_CHECK_PTR (gpr);
  // add this to our price history
  if (m_priceCount == 0) signalProgress (0, 1, i18n("Loading prices..."));
  MyMoneyMoney rate = convBadValue (gpr->value());
  if (gpr->commodity()->isCurrency()) {
    MyMoneyPrice exchangeRate (gpr->commodity()->id().toUtf8(), gpr->currency()->id().toUtf8(),
                               gpr->priceDate(), rate, i18n("Imported History"));
    m_storage->addPrice (exchangeRate);
  } else {
    MyMoneySecurity e = m_storage->security(m_mapEquities[gpr->commodity()->id().toUtf8()]);
    if (gncdebug) qDebug () << "Searching map, key = " << gpr->commodity()->id()
         << ", found id =" << e.id().data();
    e.setTradingCurrency (gpr->currency()->id().toUtf8());
    MyMoneyPrice stockPrice(e.id(), gpr->currency()->id().toUtf8(), gpr->priceDate(), rate, i18n("Imported History"));
    m_storage->addPrice (stockPrice);
    m_storage->modifySecurity(e);
  }
  signalProgress (++m_priceCount, 0);
  return ;
}

//*********************************convertAccount ****************************************
void MyMoneyGncReader::convertAccount (const GncAccount* gac) {
  Q_CHECK_PTR (gac);
  TRY
  // we don't care about the GNC root account
  if("ROOT" == gac->type()) {
      m_rootId = gac->id().toUtf8();
      return;
  }

  MyMoneyAccount acc;
  if (m_accountCount == 0) signalProgress (0, m_gncAccountCount, i18n("Loading accounts..."));
  acc.setName(gac->name());

  acc.setDescription(gac->desc());

  QDate currentDate = QDate::currentDate();
  acc.setOpeningDate(currentDate);
  acc.setLastModified(currentDate);
  acc.setLastReconciliationDate(currentDate);
  if (gac->commodity()->isCurrency()) {
    acc.setCurrencyId (gac->commodity()->id().toUtf8());
    m_currencyCount[gac->commodity()->id()]++;
  }

  acc.setParentAccountId (gac->parent().toUtf8());
  // now determine the account type and its parent id
  /* This list taken from
# Feb 2006: A RELAX NG Compact schema for gnucash "v2" XML files.
# Copyright (C) 2006 Joshua Sled <jsled@asynchronous.org>
"NO_TYPE" "BANK" "CASH" "CREDIT" "ASSET" "LIABILITY" "STOCK" "MUTUAL" "CURRENCY"
"INCOME" "EXPENSE" "EQUITY" "RECEIVABLE" "PAYABLE" "CHECKING" "SAVINGS" "MONEYMRKT" "CREDITLINE"
  Some don't seem to be used in practice. Not sure what CREDITLINE s/be converted as.
  */
  if ("BANK" == gac->type() || "CHECKING" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Checkings);
  } else if ("SAVINGS" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Savings);
  } else if ("ASSET" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Asset);
  } else if ("CASH" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Cash);
  } else if ("CURRENCY" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Cash);
  } else if ("STOCK" == gac->type() || "MUTUAL" == gac->type() ) {
    // gnucash allows a 'broker' account to be denominated as type STOCK, but with
    // a currency balance. We do not need to create a stock account for this
    // actually, the latest version of gnc (1.8.8) doesn't seem to allow you to do
    // this any more, though I do have one in my own account...
    if (gac->commodity()->isCurrency()) {
      acc.setAccountType(MyMoneyAccount::Investment);
    } else {
      acc.setAccountType(MyMoneyAccount::Stock);
    }
  } else if ("EQUITY" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Equity);
  } else if ("LIABILITY" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Liability);
  } else if ("CREDIT" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::CreditCard);
  } else if ("INCOME" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Income);
  } else if ("EXPENSE" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Expense);
  } else if ("RECEIVABLE" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Asset);
  } else if ("PAYABLE" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::Liability);
  } else if ("MONEYMRKT" == gac->type()) {
    acc.setAccountType(MyMoneyAccount::MoneyMarket);
  } else { // we have here an account type we can't currently handle
    QString em =
        i18n("Current importer does not recognize GnuCash account type %1",gac->type());
    throw new MYMONEYEXCEPTION (em);
  }
  // if no parent account is present, assign to one of our standard accounts
  if ((acc.parentAccountId().isEmpty()) || (acc.parentAccountId() == m_rootId)) {
    switch (acc.accountGroup()) {
    case MyMoneyAccount::Asset: acc.setParentAccountId (m_storage->asset().id()); break;
    case MyMoneyAccount::Liability: acc.setParentAccountId (m_storage->liability().id()); break;
    case MyMoneyAccount::Income: acc.setParentAccountId (m_storage->income().id()); break;
    case MyMoneyAccount::Expense: acc.setParentAccountId (m_storage->expense().id()); break;
    case MyMoneyAccount::Equity: acc.setParentAccountId (m_storage->equity().id()); break;
    default: break; // not necessary but avoids compiler warnings
    }
  }

  // extra processing for a stock account
  if (acc.accountType() == MyMoneyAccount::Stock) {
    // save the id for later linking to investment account
    m_stockList.append (gac->id());
    // set the equity type
    MyMoneySecurity e = m_storage->security (m_mapEquities[gac->commodity()->id().toUtf8()]);
    if (gncdebug) qDebug ("Acct equity search, key = %s, found id = %s",
                            qPrintable(gac->commodity()->id()), qPrintable(e.id()));
    acc.setCurrencyId (e.id()); // actually, the security id
    if ("MUTUAL" == gac->type()) {
      e.setSecurityType (MyMoneySecurity::SECURITY_MUTUALFUND);
      if (gncdebug) qDebug ("Setting %s to mutual", qPrintable(e.name()));
      m_storage->modifySecurity (e);
    }
    // See if he wants online quotes for this account
    // NB: In gnc, this selection is per account, in KMM, per security
    // This is unlikely to cause problems in practice. If it does,
    // we probably need to introduce a 'pricing basis' in the account class
    QList<GncObject*>::const_iterator kvpi;
    QList<GncObject*> list = gac->m_kvpList;
    GncKvp *k;
    for (kvpi = list.begin(); kvpi != list.end(); ++kvpi) {
      k = static_cast<GncKvp*> (*(kvpi));
      if (k->key().contains("price-source") && k->type() == "string") {
        getPriceSource (e, k->value());
        break;
      } else {
        ++kvpi;
      }
    }
  }

  // check for tax-related status
  QList<GncObject*>::const_iterator kvpi;
  QList<GncObject*> list = gac->m_kvpList;
  GncKvp *k;
  for (kvpi = list.begin(); kvpi != list.end(); ++kvpi) {
    k = static_cast<GncKvp*> (*(kvpi));
    if (k->key().contains("tax-related") && k->type() == "integer" && k->value() == "1") {
      acc.setValue ("Tax", "Yes");
      break;
    }
  }

  // all the details from the file about the account should be known by now.
  // calling addAccount will automatically fill in the account ID.
  m_storage->addAccount(acc);
  m_mapIds[gac->id().toUtf8()] = acc.id(); // to link gnucash id to ours for tx posting

  if (gncdebug)
      qDebug() << "Gnucash account" << gac->id() << "has id of" << acc.id()
               << ", type of" << KMyMoneyUtils::accountTypeToString(acc.accountType())
               << "parent is" << acc.parentAccountId();

  signalProgress (++m_accountCount, 0);
  return ;
  PASS
}

//********************************************** convertTransaction *****************************
void MyMoneyGncReader::convertTransaction (const GncTransaction *gtx) {
  Q_CHECK_PTR (gtx);
  MyMoneyTransaction tx;
  MyMoneySplit split;
  unsigned int i;

  if (m_transactionCount == 0) signalProgress (0, m_gncTransactionCount, i18n("Loading transactions..."));
  // initialize class variables related to transactions
  m_txCommodity = "";
  m_txPayeeId = "";
  m_potentialTransfer = true;
  m_splitList.clear(); m_liabilitySplitList.clear(); m_otherSplitList.clear();
  // payee, dates, commodity
  if (!gtx->desc().isEmpty()) m_txPayeeId = createPayee (gtx->desc());
  tx.setEntryDate (gtx->dateEntered());
  tx.setPostDate (gtx->datePosted());
  m_txDatePosted = tx.postDate(); // save for use in splits
  m_txChequeNo = gtx->no(); // ditto
  tx.setCommodity (gtx->currency().toUtf8());
  m_txCommodity = tx.commodity(); // save in storage, maybe needed for Orphan accounts
  // process splits
  for (i = 0; i < gtx->splitCount(); i++) {
    convertSplit (static_cast<const GncSplit *>(gtx->getSplit (i)));
  }
  // handle the odd case of just one split, which gnc allows,
  // by just duplicating the split
  // of course, we should change the sign but this case has only ever been seen
  // when the balance is zero, and can cause kmm to crash, so...
  if (gtx->splitCount() == 1) {
    convertSplit (static_cast<const GncSplit *>(gtx->getSplit (0)));
  }
  m_splitList += m_liabilitySplitList += m_otherSplitList;
  // the splits are in order in splitList. Link them to the tx. also, determine the
  // action type, and fill in some fields which gnc holds at transaction level
  // first off, is it a transfer (can only have 2 splits?)
  // also, a tx with just 2 splits is shown by GnuCash as non-split
  bool nonSplitTx = true;
  if (m_splitList.count() != 2) {
    m_potentialTransfer = false;
    nonSplitTx = false;
  }
  for (i = 0; i < gtx->kvpCount(); i++ ) {
    const GncKvp *slot = gtx->getKvp(i);
    if (slot->key() == "notes") tx.setMemo(slot->value());
  }
  QList<MyMoneySplit>::iterator it = m_splitList.begin();
  while (!m_splitList.isEmpty()) {
    split = *it;
    // at this point, if m_potentialTransfer is still true, it is actually one!
    if (m_potentialTransfer) split.setAction(MyMoneySplit::ActionTransfer);
    if ((m_useTxNotes) // if use txnotes option is set
         && (nonSplitTx) // and it's a (GnuCash) non-split transaction
         && (!tx.memo().isEmpty())) // and tx notes are present
      split.setMemo(tx.memo());  // use the tx notes as memo
    tx.addSplit(split);
    it = m_splitList.erase(it);
  }
  // memo - set from split - not any more
  //tx.setMemo(txMemo);
  m_storage->addTransaction(tx, true); // all done, add the transaction to storage
  signalProgress (++m_transactionCount, 0);
  return ;
}
//******************************************convertSplit********************************
void MyMoneyGncReader::convertSplit (const GncSplit *gsp) {
  Q_CHECK_PTR (gsp);
  MyMoneySplit split;
  MyMoneyAccount splitAccount;
  // find the kmm account id corresponding to the gnc id
  QString kmmAccountId;
  map_accountIds::const_iterator id = m_mapIds.find(gsp->acct().toUtf8());
  if (id != m_mapIds.end()) {
    kmmAccountId = id.value();
  } else { // for the case where the acs not found (which shouldn't happen?), create an account with gnc name
    kmmAccountId = createOrphanAccount (gsp->acct());
  }
  // find the account pointer and save for later
  splitAccount = m_storage->account (kmmAccountId);
  // print some data so we can maybe identify this split later
  // TODO : prints personal data
  //if (gncdebug) qDebug ("Split data - gncid %s, kmmid %s, memo %s, value %s, recon state %s",
  //                        gsp->acct().toLatin1(), kmmAccountId.data(), gsp->memo().toLatin1(), gsp->value().toLatin1(),
  //                        gsp->recon().toLatin1());
  // payee id
  split.setPayeeId (m_txPayeeId.toUtf8());
  // reconciled state and date
  switch (gsp->recon().at(0).toLatin1()) {
  case 'n':
    split.setReconcileFlag(MyMoneySplit::NotReconciled); break;
  case 'c':
    split.setReconcileFlag(MyMoneySplit::Cleared); break;
  case 'y':
    split.setReconcileFlag(MyMoneySplit::Reconciled); break;
  }
  split.setReconcileDate(gsp->reconDate());
  // memo
  split.setMemo(gsp->memo());
  // accountId
  split.setAccountId (kmmAccountId);
  // cheque no
  split.setNumber (m_txChequeNo);
  // value and quantity
  MyMoneyMoney splitValue (convBadValue (gsp->value()));
  if (gsp->value() == "-1/0") { // treat gnc invalid value as zero
   // it's not quite a consistency check, but easier to treat it as such
   postMessage ("CC", 4, splitAccount.name().toLatin1(), m_txDatePosted.toString(Qt::ISODate).toLatin1());
  }
  MyMoneyMoney splitQuantity(convBadValue(gsp->qty()));
  split.setValue (splitValue);
  // if split currency = tx currency, set shares = value (14/10/05)
  if (splitAccount.currencyId() == m_txCommodity) {
    split.setShares (splitValue);
  } else {
    split.setShares (splitQuantity);
  }

  // in kmm, the first split is important. in this routine we will
  // save the splits in our split list with the priority:
  // 1. assets
  // 2. liabilities
  // 3. others (categories)
  // but keeping each in same order as gnucash
  MyMoneySecurity e;
  MyMoneyMoney price, newPrice(0);

  switch (splitAccount.accountGroup()) {
  case MyMoneyAccount::Asset:
    if (splitAccount.accountType() == MyMoneyAccount::Stock) {
      split.value() == MyMoneyMoney(0) ?
        split.setAction (MyMoneySplit::ActionAddShares) :     // free shares?
        split.setAction (MyMoneySplit::ActionBuyShares);
      m_potentialTransfer = false; // ?
      // add a price history entry
      e = m_storage->security(splitAccount.currencyId());
      // newPrice fix supplied by Phil Longstaff
      price = split.value() / split.shares();
#define NEW_DENOM 10000
      if (!split.shares().isZero())  // patch to fix divide by zero?
        newPrice = MyMoneyMoney ( price.toDouble(), (signed64)NEW_DENOM );
      if (!newPrice.isZero()) {
        TRY
          // we can't use m_storage->security coz security list is not built yet
          m_storage->currency(m_txCommodity);   // will throw exception if not currency
          e.setTradingCurrency (m_txCommodity);
          if (gncdebug) qDebug ("added price for %s, %s date %s",
              qPrintable(e.name()), qPrintable(newPrice.toString()),
              qPrintable(m_txDatePosted.toString(Qt::ISODate)));
          m_storage->modifySecurity(e);
          MyMoneyPrice dealPrice (e.id(), m_txCommodity, m_txDatePosted, newPrice, i18n("Imported Transaction"));
          m_storage->addPrice (dealPrice);
        CATCH // stock transfer; treat like free shares?
          split.setAction (MyMoneySplit::ActionAddShares);
          delete e;
        }
      }
    } else { // not stock
      if (split.value().isNegative()) {
        bool isNumeric = false;
        if (!split.number().isEmpty()) {
          split.number().toLong(&isNumeric);    // No QString.isNumeric()??
        }
        if (isNumeric) {
          split.setAction (MyMoneySplit::ActionCheck);
        } else {
          split.setAction (MyMoneySplit::ActionWithdrawal);
        }
      } else {
        split.setAction (MyMoneySplit::ActionDeposit);
      }
    }
    m_splitList.append(split);
    break;
  case MyMoneyAccount::Liability:
    split.value().isNegative() ?
        split.setAction (MyMoneySplit::ActionWithdrawal) :
        split.setAction (MyMoneySplit::ActionDeposit);
    m_liabilitySplitList.append(split);
    break;
  default:
    m_potentialTransfer = false;
    m_otherSplitList.append (split);
  }
  // backdate the account opening date if necessary
  if (m_txDatePosted < splitAccount.openingDate()) {
    splitAccount.setOpeningDate(m_txDatePosted);
    m_storage->modifyAccount(splitAccount);
  }
  return ;
}
//********************************* convertTemplateTransaction **********************************************
MyMoneyTransaction MyMoneyGncReader::convertTemplateTransaction (const QString& schedName, const GncTransaction *gtx) {

  Q_CHECK_PTR (gtx);
  MyMoneyTransaction tx;
  MyMoneySplit split;
  unsigned int i;
  if (m_templateCount == 0) signalProgress (0, 1, i18n("Loading templates..."));

  // initialize class variables related to transactions
  m_txCommodity = "";
  m_txPayeeId = "";
  m_potentialTransfer = true;
  m_splitList.clear(); m_liabilitySplitList.clear(); m_otherSplitList.clear();

  // payee, dates, commodity
  if (!gtx->desc().isEmpty()) {
    m_txPayeeId = createPayee (gtx->desc());
  } else {
    m_txPayeeId = createPayee (i18n("Unknown payee")); // schedules require a payee tho normal tx's don't. not sure why...
  }
  tx.setEntryDate(gtx->dateEntered());
  tx.setPostDate(gtx->datePosted());
  m_txDatePosted = tx.postDate();
  tx.setCommodity (gtx->currency().toUtf8());
  m_txCommodity = tx.commodity(); // save for possible use in orphan account
  // process splits
  for (i = 0; i < gtx->splitCount(); i++) {
    convertTemplateSplit (schedName, static_cast<const GncTemplateSplit *>(gtx->getSplit (i)));
  }
  // determine the action type for the splits and link them to the template tx
  /*QString negativeActionType, positiveActionType;
  if (!m_splitList.isEmpty()) { // if there are asset splits
    positiveActionType = MyMoneySplit::ActionDeposit;
    negativeActionType = MyMoneySplit::ActionWithdrawal;
  } else { // if there are liability splits
    positiveActionType = MyMoneySplit::ActionWithdrawal;
    negativeActionType = MyMoneySplit::ActionDeposit;
} */
  if (!m_otherSplitList.isEmpty()) m_potentialTransfer = false; // tfrs can occur only between assets and asset/liabilities
  m_splitList += m_liabilitySplitList += m_otherSplitList;
  // the splits are in order in splitList. Transfer them to the tx
  // also, determine the action type. first off, is it a transfer (can only have 2 splits?)
  if (m_splitList.count() != 2) m_potentialTransfer = false;
  // at this point, if m_potentialTransfer is still true, it is actually one!
  QString txMemo = "";
  QList<MyMoneySplit>::iterator it = m_splitList.begin();
  while (!m_splitList.isEmpty()) {
    split = *it;
    if (m_potentialTransfer) {
      split.setAction(MyMoneySplit::ActionTransfer);
    } else {
      if (split.value().isNegative()) {
        //split.setAction (negativeActionType);
        split.setAction (MyMoneySplit::ActionWithdrawal);
      } else {
        //split.setAction (positiveActionType);
        split.setAction (MyMoneySplit::ActionDeposit);
      }
    }
    split.setNumber(gtx->no()); // set cheque no (or equivalent description)
    // Arbitrarily, save the first non-null split memo as the memo for the whole tx
    // I think this is necessary because txs with just 2 splits (the majority)
    // are not viewable as split transactions in kmm so the split memo is not seen
    if ((txMemo.isEmpty()) && (!split.memo().isEmpty())) txMemo = split.memo();
    tx.addSplit(split);
    it = m_splitList.erase(it);
  }
  // memo - set from split
  tx.setMemo (txMemo);
  signalProgress (++m_templateCount, 0);
  return (tx);
}
//********************************* convertTemplateSplit ****************************************************
void MyMoneyGncReader::convertTemplateSplit (const QString& schedName, const GncTemplateSplit *gsp) {
  Q_CHECK_PTR (gsp);
  // convertTemplateSplit
  MyMoneySplit split;
  MyMoneyAccount splitAccount;
  unsigned int i, j;
  bool nonNumericFormula = false;

  // action, value and account will be set from slots
  // reconcile state, always Not since it hasn't even been posted yet (?)
  split.setReconcileFlag(MyMoneySplit::NotReconciled);
  // memo
  split.setMemo(gsp->memo());
  // payee id
  split.setPayeeId (m_txPayeeId.toUtf8());
  // read split slots (KVPs)
  int xactionCount = 0;
  int validSlotCount = 0;
  QString gncAccountId;
  for (i = 0; i < gsp->kvpCount(); i++ ) {
    const GncKvp *slot = gsp->getKvp(i);
    if ((slot->key() == "sched-xaction") && (slot->type() == "frame")) {
      bool bFoundStringCreditFormula = false;
      bool bFoundStringDebitFormula = false;
      bool bFoundGuidAccountId = false;
      QString gncCreditFormula, gncDebitFormula;
      for (j = 0; j < slot->kvpCount(); j++) {
        const GncKvp *subSlot = slot->getKvp (j);
        // again, see comments above. when we have a full specification
        // of all the options available to us, we can no doubt improve on this
        if ((subSlot->key() == "credit-formula") && (subSlot->type() == "string")) {
          gncCreditFormula = subSlot->value();
          bFoundStringCreditFormula = true;
        }
        if ((subSlot->key() == "debit-formula") && (subSlot->type() == "string")) {
          gncDebitFormula = subSlot->value();
          bFoundStringDebitFormula = true;
        }
        if ((subSlot->key() == "account") && (subSlot->type() == "guid")) {
          gncAccountId = subSlot->value();
          bFoundGuidAccountId = true;
        }
      }
      // all data read, now check we have everything
      if ((bFoundStringCreditFormula) && (bFoundStringDebitFormula) && (bFoundGuidAccountId)) {
        if (gncdebug) qDebug ("Found valid slot; credit %s, debit %s, acct %s",
                                qPrintable(gncCreditFormula), qPrintable(gncDebitFormula), qPrintable(gncAccountId));
        validSlotCount++;
      }
      // validate numeric, work out sign
      MyMoneyMoney exFormula (0);
      exFormula.setNegativeMonetarySignPosition (MyMoneyMoney::BeforeQuantityMoney);
      QString numericTest;
      char crdr=0 ;
      if (!gncCreditFormula.isEmpty()) {
        crdr = 'C';
        numericTest = gncCreditFormula;
      } else if (!gncDebitFormula.isEmpty()) {
        crdr = 'D';
        numericTest = gncDebitFormula;
      }
      kMyMoneyMoneyValidator v (0);
      int pos; // useless, but required for validator
      if (v.validate (numericTest, pos) == QValidator::Acceptable) {
        switch (crdr) {
          case 'C':
            exFormula = QString ("-" + numericTest); break;
          case 'D':
            exFormula = numericTest;
        }
      } else {
          if (gncdebug) qDebug ("%s is not numeric", qPrintable(numericTest));
          nonNumericFormula = true;
      }
      split.setValue (exFormula);
      xactionCount++;
    } else {
      postMessage ("SC", 3, schedName.toLatin1(), slot->key().toLatin1(), slot->type().toLatin1());
      m_suspectSchedule = true;
    }
  }
  // report this as untranslatable tx
  if (xactionCount > 1) {
    postMessage ("SC", 4, schedName.toLatin1());
    m_suspectSchedule = true;
  }
  if (validSlotCount == 0) {
    postMessage ("SC", 5, schedName.toLatin1());
    m_suspectSchedule = true;
  }
  if (nonNumericFormula) {
    postMessage ("SC", 6, schedName.toLatin1());
    m_suspectSchedule = true;
  }
  // find the kmm account id corresponding to the gnc id
  QString kmmAccountId;
  map_accountIds::const_iterator id = m_mapIds.find(gncAccountId.toUtf8());
  if (id != m_mapIds.end()) {
    kmmAccountId = id.value();
  } else { // for the case where the acs not found (which shouldn't happen?), create an account with gnc name
    kmmAccountId = createOrphanAccount (gncAccountId);
  }
  splitAccount = m_storage->account (kmmAccountId);
  split.setAccountId (kmmAccountId);
  // if split currency = tx currency, set shares = value (14/10/05)
  if (splitAccount.currencyId() == m_txCommodity) {
    split.setShares (split.value());
  } /* else {  //FIXME: scheduled currency or investment tx needs to be investigated
    split.setShares (splitQuantity);
  }  */
  // add the split to one of the lists
  switch (splitAccount.accountGroup()) {
  case MyMoneyAccount::Asset:
    m_splitList.append (split); break;
  case MyMoneyAccount::Liability:
    m_liabilitySplitList.append (split); break;
  default:
    m_otherSplitList.append (split);
  }
  // backdate the account opening date if necessary
  if (m_txDatePosted < splitAccount.openingDate()) {
    splitAccount.setOpeningDate(m_txDatePosted);
    m_storage->modifyAccount(splitAccount);
  }
  return ;
}
//********************************* convertSchedule  ********************************************************
void MyMoneyGncReader::convertSchedule (const GncSchedule *gsc) {
  TRY
  Q_CHECK_PTR (gsc);
  MyMoneySchedule sc;
  MyMoneyTransaction tx;
  m_suspectSchedule = false;
  QDate startDate, nextDate, lastDate, endDate;  // for date calculations
  QDate today = QDate::currentDate();
  int numOccurs, remOccurs;

  if (m_scheduleCount == 0) signalProgress (0, m_gncScheduleCount, i18n("Loading schedules..."));
  // schedule name
  sc.setName(gsc->name());
  // find the transaction template as stored earlier
  QList<GncTransaction*>::const_iterator itt;
  //GncTransaction *ttx;
  for (itt = m_templateList.begin(); itt != m_templateList.end(); ++itt) {
    // the id to match against is the split:account value in the splits
    if (static_cast<const GncTemplateSplit *>((*itt)->getSplit(0))->acct() == gsc->templId()) break;
  }
  if (itt == m_templateList.end()) {
    throw new MYMONEYEXCEPTION (i18n("Can't find template transaction for schedule %1",sc.name()));
  } else {
    tx = convertTemplateTransaction (sc.name(), *itt);
  }
  tx.clearId();

// define the conversion table for intervals
  struct convIntvl {
    QString gncType; // the gnucash name
    unsigned char interval; // for date calculation
    unsigned int intervalCount;
    MyMoneySchedule::occurrenceE occ; // equivalent occurrence code
    MyMoneySchedule::weekendOptionE wo;
  };
/* other intervals supported by gnc according to Josh Sled's schema (see above)
 "none" "semi_monthly"
 */
  /* some of these type names do not appear in gnucash and are difficult to generate for
  pre 2.2 files.They can be generated for 2.2 however, by GncRecurrence::getFrequency() */
  static convIntvl vi [] = {
                           {"once", 'o', 1, MyMoneySchedule::OCCUR_ONCE, MyMoneySchedule::MoveNothing },
                           {"daily" , 'd', 1, MyMoneySchedule::OCCUR_DAILY, MyMoneySchedule::MoveNothing },
                           //{"daily_mf", 'd', 1, MyMoneySchedule::OCCUR_DAILY, MyMoneySchedule::MoveMonday }, doesn't work, need new freq in kmm
                           {"30-days" , 'd', 30, MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS, MyMoneySchedule::MoveNothing },
                           {"weekly", 'w', 1, MyMoneySchedule::OCCUR_WEEKLY, MyMoneySchedule::MoveNothing },
                           {"bi_weekly", 'w', 2, MyMoneySchedule::OCCUR_EVERYOTHERWEEK, MyMoneySchedule::MoveNothing },
                           {"three-weekly", 'w', 3, MyMoneySchedule::OCCUR_EVERYTHREEWEEKS, MyMoneySchedule::MoveNothing },
                           {"four-weekly", 'w', 4, MyMoneySchedule::OCCUR_EVERYFOURWEEKS,
                           MyMoneySchedule::MoveNothing },
                           {"eight-weekly", 'w', 8, MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS, MyMoneySchedule::MoveNothing },
                           {"monthly", 'm', 1, MyMoneySchedule::OCCUR_MONTHLY, MyMoneySchedule::MoveNothing },
                           {"two-monthly", 'm', 2, MyMoneySchedule::OCCUR_EVERYOTHERMONTH,
                            MyMoneySchedule::MoveNothing },
                           {"quarterly", 'm', 3, MyMoneySchedule::OCCUR_QUARTERLY, MyMoneySchedule::MoveNothing },
                           {"tri_annually", 'm', 4, MyMoneySchedule::OCCUR_EVERYFOURMONTHS, MyMoneySchedule::MoveNothing },
                           {"semi_yearly", 'm', 6, MyMoneySchedule::OCCUR_TWICEYEARLY, MyMoneySchedule::MoveNothing },
                           {"yearly", 'y', 1, MyMoneySchedule::OCCUR_YEARLY, MyMoneySchedule::MoveNothing },
                           {"two-yearly", 'y', 2, MyMoneySchedule::OCCUR_EVERYOTHERYEAR,
                            MyMoneySchedule::MoveNothing },
                           {"zzz", 'y', 1, MyMoneySchedule::OCCUR_YEARLY, MyMoneySchedule::MoveNothing}
                            // zzz = stopper, may cause problems. what else can we do?
                           };

  QString frequency = "unknown"; // set default to unknown frequency
  bool unknownOccurs = false; // may have zero, or more than one frequency/recurrence spec
  QString schedEnabled;
  if (gsc->version() == "2.0.0") {
    if (gsc->m_vpRecurrence.count() != 1) {
      unknownOccurs = true;
    } else {
      const GncRecurrence *gre = gsc->m_vpRecurrence.first();
      //qDebug (QString("Sched %1, pt %2, mu %3, sd %4").arg(gsc->name()).arg(gre->periodType())
       // .arg(gre->mult()).arg(gre->startDate().toString(Qt::ISODate)));
      frequency = gre->getFrequency();
      schedEnabled = gsc->enabled();
    }
    sc.setOccurrence(MyMoneySchedule::OCCUR_ONCE); // FIXME - how to convert
  } else {
    // find this interval
    const GncFreqSpec *fs = gsc->getFreqSpec();
    if (fs == NULL) {
      unknownOccurs = true;
    } else {
      frequency = fs->intervalType();
      if (!fs->m_fsList.isEmpty()) unknownOccurs = true; // nested freqspec
    }
    schedEnabled = 'y'; // earlier versions did not have an enable flag
  }

  int i;
  for (i = 0; vi[i].gncType != "zzz"; i++) {
    if (frequency == vi[i].gncType) break;
  }
  if (vi[i].gncType == "zzz") {
    postMessage ("SC", 1, sc.name().toLatin1(), frequency.toLatin1());
    i = 0; // treat as single occurrence
    m_suspectSchedule = true;
  }
  if (unknownOccurs) {
    postMessage ("SC", 7, sc.name().toLatin1());
    m_suspectSchedule = true;
  }
  // set the occurrence interval, weekend option, start date
  sc.setOccurrence (vi[i].occ);
  sc.setWeekendOption (vi[i].wo);
  sc.setStartDate (gsc->startDate());
  // if a last date was specified, use it, otherwise try to work out the last date
  sc.setLastPayment(gsc->lastDate());
  numOccurs = gsc->numOccurs().toInt();
  if (sc.lastPayment() == QDate()) {
    nextDate = lastDate = gsc->startDate();
    while (nextDate < today) {
      lastDate = nextDate;
      nextDate = incrDate (lastDate, vi[i].interval, vi[i].intervalCount);
    }
    sc.setLastPayment(lastDate);
  }
  // under Tom's new regime, the tx dates are the next due date (I think)
  tx.setPostDate(incrDate(sc.lastPayment(), vi[i].interval, vi[i].intervalCount));
  tx.setEntryDate(incrDate(sc.lastPayment(), vi[i].interval, vi[i].intervalCount));
  // if an end date was specified, use it, otherwise if the input file had a number
  // of occurs remaining, work out the end date
  sc.setEndDate(gsc->endDate());
  remOccurs = gsc->remOccurs().toInt();
  if ((sc.endDate() == QDate()) && (remOccurs > 0)) {
    endDate = sc.lastPayment();
    while (remOccurs-- > 0) {
      endDate = incrDate (endDate, vi[i].interval, vi[i].intervalCount);
    }
    sc.setEndDate(endDate);
  }
  // Check for sched deferred interval. Don't know how/if we can handle it, or even what it means...
  if (gsc->getSchedDef() != NULL) {
    postMessage ("SC", 8, sc.name().toLatin1());
    m_suspectSchedule = true;
  }
  // payment type, options
  sc.setPaymentType((MyMoneySchedule::paymentTypeE)MyMoneySchedule::STYPE_OTHER);
  sc.setFixed (!m_suspectSchedule); // if any probs were found, set it as variable so user will always be prompted
  // we don't currently have a 'disable' option, but just make sure auto-enter is off if not enabled
  //qDebug(QString("%1 and %2").arg(gsc->autoCreate()).arg(schedEnabled));
  //qDebug(QString("autoEnter set to %1").arg(sc.autoEnter()));
  // type
  QString actionType = tx.splits().first().action();
  if (actionType == MyMoneySplit::ActionDeposit) {
    sc.setType((MyMoneySchedule::typeE)MyMoneySchedule::TYPE_DEPOSIT);
  } else if (actionType == MyMoneySplit::ActionTransfer) {
    sc.setType((MyMoneySchedule::typeE)MyMoneySchedule::TYPE_TRANSFER);
  } else {
    sc.setType((MyMoneySchedule::typeE)MyMoneySchedule::TYPE_BILL);
  }
  // finally, set the transaction pointer
  sc.setTransaction(tx);
  //tell the storage objects we have a new schedule object.
  if (m_suspectSchedule && m_dropSuspectSchedules) {
    postMessage ("SC", 2, sc.name().toLatin1());
  } else {
    m_storage->addSchedule(sc);
    if (m_suspectSchedule)
      m_suspectList.append (sc.id());
  }
  signalProgress (++m_scheduleCount, 0);
  return ;
  PASS
}
//********************************* convertFreqSpec  ********************************************************
void MyMoneyGncReader::convertFreqSpec (const GncFreqSpec *) {
  // Nowt to do here at the moment, convertSched only retrieves the interval type
  // but we will probably need to look into the nested freqspec when we properly implement semi-monthly and stuff
  return ;
}
//********************************* convertRecurrence  ********************************************************
void MyMoneyGncReader::convertRecurrence (const GncRecurrence *) {
  return ;
}

//**********************************************************************************************************
//************************************* terminate **********************************************************
void MyMoneyGncReader::terminate () {
  TRY
  // All data has been converted and added to storage
  // this code is just temporary to show us what is in the file.
  if (gncdebug) qDebug("%d accounts found in the GnuCash file", (unsigned int)m_mapIds.count());
  for (map_accountIds::const_iterator it = m_mapIds.begin(); it != m_mapIds.end(); ++it) {
    if (gncdebug) qDebug() << "key ="  << it.key() << "value =" << it.value();
  }
  // first step is to implement the users investment option, now we
  // have all the accounts available
  QList<QString>::iterator stocks;
  for (stocks = m_stockList.begin(); stocks != m_stockList.end(); ++stocks) {
    checkInvestmentOption (*stocks);
  }
  // Next step is to walk the list and assign the parent/child relationship between the objects.
  unsigned int i = 0;
  signalProgress (0, m_accountCount, i18n ("Reorganizing accounts..."));
  QList<MyMoneyAccount> list;
  QList<MyMoneyAccount>::iterator acc;
  m_storage->accountList(list);
  for (acc = list.begin(); acc != list.end(); ++acc) {
    if ((*acc).parentAccountId() == m_storage->asset().id()) {
      MyMoneyAccount assets = m_storage->asset();
      m_storage->addAccount(assets, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main asset account", qPrintable((*acc).id()));
    } else if ((*acc).parentAccountId() == m_storage->liability().id()) {
      MyMoneyAccount liabilities = m_storage->liability();
      m_storage->addAccount(liabilities, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main liability account", qPrintable((*acc).id()));
    } else if ((*acc).parentAccountId() == m_storage->income().id()) {
      MyMoneyAccount incomes = m_storage->income();
      m_storage->addAccount(incomes, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main income account", qPrintable((*acc).id()));
    } else if ((*acc).parentAccountId() == m_storage->expense().id()) {
      MyMoneyAccount expenses = m_storage->expense();
      m_storage->addAccount(expenses, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main expense account", qPrintable((*acc).id()));
    } else if ((*acc).parentAccountId() == m_storage->equity().id()) {
      MyMoneyAccount equity = m_storage->equity();
      m_storage->addAccount(equity, (*acc));
      if (gncdebug) qDebug("Account id %s is a child of the main equity account", qPrintable((*acc).id()));
    } else if ((*acc).parentAccountId() == m_rootId) {
      if (gncdebug) qDebug("Account id %s is a child of root", qPrintable((*acc).id()));
    } else {
      // it is not under one of the main accounts, so find gnucash parent
      QString parentKey = (*acc).parentAccountId();
      if (gncdebug) qDebug ("acc %s, parent %s", qPrintable((*acc).id()),
                              qPrintable((*acc).parentAccountId()));
      map_accountIds::const_iterator id = m_mapIds.find(parentKey);
      if (id != m_mapIds.end()) {
        if (gncdebug)
            qDebug() << "Setting account id" << (*acc).id()
                     << "parent account id to" << id.value();
        MyMoneyAccount parent = m_storage->account(id.value());
        parent = checkConsistency (parent, (*acc));
        m_storage->addAccount (parent, (*acc));
      } else {
        throw new MYMONEYEXCEPTION ("terminate() could not find account id");
      }
    }
    signalProgress (++i, 0);
  } // end for account
  signalProgress (0, 1, (".")); // debug - get rid of reorg message
  // offer the most common account currency as a default
  QString mainCurrency = "";
  unsigned int maxCount = 0;
  QMap<QString, unsigned int>::ConstIterator it;
  for (it = m_currencyCount.constBegin(); it != m_currencyCount.constEnd(); ++it) {
    if (it.value() > maxCount) {
      maxCount = it.value();
      mainCurrency = it.key();
      }
  }

  if (mainCurrency != "") {
    /* fix for qt3.3.4?. According to Qt docs, this should return the enum id of the button pressed, and
       indeed it used to do so. However now it seems to return the index of the button. In this case it doesn't matter,
       since for Yes, the id is 3 and the index is 0, whereas the No button will return 4 or 1. So we test for either Yes case */
    /* and now it seems to have changed again, returning 259 for a Yes??? so use KMessagebox */
    QString question = i18n("Your main currency seems to be %1 (%2); do you want to set this as your base currency?", mainCurrency,m_storage->currency(mainCurrency.toUtf8()).name());
    if(KMessageBox::questionYesNo(0, question, PACKAGE) == KMessageBox::Yes) {
      m_storage->setValue ("kmm-baseCurrency", mainCurrency);
    }
  }
  // now produce the end of job reports - first, work out which ones are required
  m_ccCount = 0, m_orCount = 0, m_scCount = 0;
  int si;
  for (si = 0; si < m_messageList.count(); ++si) {
    if ((*m_messageList.at(si)).source == "CC") m_ccCount++;
    if ((*m_messageList.at(si)).source == "OR") m_orCount++;
    if ((*m_messageList.at(si)).source == "SC") m_scCount++;
  }
  QList<QString> sectionsToReport; // list of sections needing report
  sectionsToReport.append ("MN"); // always build the main section
  if (m_ccCount > 0) sectionsToReport.append ("CC");
  if (m_orCount > 0) sectionsToReport.append ("OR");
  if (m_scCount > 0) sectionsToReport.append ("SC");
  // produce the sections in message boxes
  bool exit = false;
  for (si = 0; (si < sectionsToReport.count()) && !exit; ++si) {
    QString button0Text = i18n("More");
    if (si + 1 == sectionsToReport.count())
      button0Text = i18n("Done"); // last section
    KGuiItem yesItem(button0Text, KIcon(), "", "");
    KGuiItem noItem(i18n("Save Report"), KIcon(), "", "");

    switch(KMessageBox::questionYesNoCancel(0,
           buildReportSection (sectionsToReport[si]),
                                PACKAGE,
                                yesItem, noItem)) {
      case KMessageBox::Yes:
        break;
      case KMessageBox::No:
        exit = writeReportToFile (sectionsToReport);
        break;
      default:
        exit = true;
        break;
    }
  }

  for (si = 0; si < m_suspectList.count(); ++si) {
    MyMoneySchedule sc = m_storage->schedule(m_suspectList[si]);
      KEditScheduleDlg *s;
      switch(KMessageBox::warningYesNo(0, i18n("Problems were encountered in converting schedule '%1'.\nDo you want to review or edit it now?",sc.name()), PACKAGE)) {
      case KMessageBox::Yes:
        s = new KEditScheduleDlg (sc);
        // FIXME: connect newCategory to something useful, so that we
        // can create categories from within the dialog
        if (s->exec())
          m_storage->modifySchedule (s->schedule());
        delete s;
        break;

      default:
        break;
    }
  }
  PASS
}
//************************************ buildReportSection************************************
QString MyMoneyGncReader::buildReportSection (const QString& source) {
  TRY
  QString s = "";
  bool more = false;
  if (source == "MN") {
    s.append (i18n("Found:\n\n"));
    s.append (QString::number(m_commodityCount) + i18n(" commodities (equities)\n"));
    s.append (QString::number(m_priceCount) + i18n(" prices\n"));
    s.append (QString::number(m_accountCount) + i18n(" accounts\n"));
    s.append (QString::number(m_transactionCount) + i18n(" transactions\n"));
    s.append (QString::number(m_scheduleCount) + i18n(" schedules\n"));
    s.append ("\n\n");
    if (m_ccCount == 0) {
      s.append (i18n("No inconsistencies were detected"));
    } else {
      s.append (QString::number(m_ccCount) + i18n(" inconsistencies were detected and corrected\n"));
      more = true;
    }
    if (m_orCount > 0) {
      s.append ("\n\n");
      s.append (QString::number(m_orCount) + i18n(" orphan accounts were created\n"));
      more = true;
    }
    if (m_scCount > 0) {
      s.append ("\n\n");
      s.append (QString::number(m_scCount) + i18n(" possible schedule problems were noted\n"));
      more = true;
    }
    QString unsupported ("");
    QString lineSep ("\n  - ");
    if (m_smallBusinessFound) unsupported.append(lineSep + i18n("Small Business Features (Customers, Invoices, etc.)"));
    if (m_budgetsFound) unsupported.append(lineSep + i18n("Budgets"));
    if (m_lotsFound) unsupported.append(lineSep + i18n("Lots"));
    if (!unsupported.isEmpty()) {
      unsupported.prepend(i18n("The following features found in your file are not currently supported:"));
      s.append(unsupported);
    }
    if (more) s.append (i18n("\n\nPress More for further information"));
  } else { // we need to retrieve the posted messages for this source
    if (gncdebug) qDebug() << "Building messages for source" << source;
    int i, j;
    for (i = 0; i < m_messageList.count(); i++) {
      GncMessageArgs *m = m_messageList[i];
      if (m->source == source) {
        if (gncdebug) qDebug() << "build text source" << m->source << "code" << m->code << "argcount" << m->args.count() ;
        QString ss = GncMessages::text (m->source, m->code);
        // add variable args. the .arg function seems always to replace the
        // lowest numbered placeholder it finds, so translating messages
        // with variables in a different order should still work okay (I think...)
        for (j = 0; j < m->args.count(); j++) ss = ss.arg (m->args[j]);
        s.append (ss + "\n");
      }
    }
  }
  if (gncdebug) qDebug () << s;
  return (static_cast<const QString>(s));
  PASS
}
//************************ writeReportToFile*********************************
bool MyMoneyGncReader::writeReportToFile (const QList<QString>& sectionsToReport) {
  TRY
  int i;
  QString fd = KFileDialog::getSaveFileName (KUrl(), QString::null,
                   0, i18n("Save report as"));
  if (fd.isEmpty()) return (false);
  QFile reportFile(fd);
  QFileInfo fi (reportFile);
  if (!reportFile.open (QIODevice::WriteOnly))  {
    return (false);
  }
  QTextStream stream (&reportFile);
  for (i = 0; i < sectionsToReport.count(); i++)
    stream << buildReportSection (sectionsToReport[i]) << endl;
  reportFile.close();
  return (true);
  PASS
}
/****************************************************************************
                    Utility routines
*****************************************************************************/
//************************ createPayee ***************************

QString MyMoneyGncReader::createPayee (const QString& gncDescription) {
  MyMoneyPayee payee;
  try {
    payee = m_storage->payeeByName (gncDescription);
  } catch (MyMoneyException *e) { // payee not found, create one
    delete e;
    payee.setName (gncDescription);
    m_storage->addPayee (payee);
  }
  return (payee.id());
}
//************************************** createOrphanAccount *******************************
QString MyMoneyGncReader::createOrphanAccount (const QString& gncName) {
  MyMoneyAccount acc;

  acc.setName ("orphan_" + gncName);
  acc.setDescription (i18n("Orphan created from unknown gnucash account"));

  QDate today = QDate::currentDate();

  acc.setOpeningDate (today);
  acc.setLastModified (today);
  acc.setLastReconciliationDate (today);
  acc.setCurrencyId (m_txCommodity);
  acc.setAccountType (MyMoneyAccount::Asset);
  acc.setParentAccountId (m_storage->asset().id());
  m_storage->addAccount (acc);
  // assign the gnucash id as the key into the map to find our id
  m_mapIds[gncName.toUtf8()] = acc.id();
  postMessage (QString("OR"), 1, acc.name().toLatin1());
  return (acc.id());
}
//****************************** incrDate *********************************************
QDate MyMoneyGncReader::incrDate (QDate lastDate, unsigned char interval, unsigned int intervalCount) {
  TRY
  switch (interval) {
  case 'd':
    return (lastDate.addDays(intervalCount));
  case 'w':
    return (lastDate.addDays(intervalCount * 7));
  case 'm':
    return (lastDate.addMonths(intervalCount));
  case 'y':
    return (lastDate.addYears(intervalCount));
  case 'o': // once-only
    return (lastDate);
  }
  throw new MYMONEYEXCEPTION (i18n("Internal error - invalid interval char in incrDate"));
  QDate r = QDate(); return (r); // to keep compiler happy
  PASS
}
//********************************* checkConsistency **********************************
MyMoneyAccount MyMoneyGncReader::checkConsistency (MyMoneyAccount& parent, MyMoneyAccount& child) {
  TRY
  // gnucash is flexible/weird enough to allow various inconsistencies
  // these are a couple I found in my file, no doubt more will be discovered
  if ((child.accountType() == MyMoneyAccount::Investment) &&
      (parent.accountType() != MyMoneyAccount::Asset)) {
    postMessage ("CC", 1, child.name().toLatin1());
    return m_storage->asset();
  }
  if ((child.accountType() == MyMoneyAccount::Income) &&
      (parent.accountType() != MyMoneyAccount::Income)) {
    postMessage ("CC", 2, child.name().toLatin1());
    return m_storage->income();
  }
  if ((child.accountType() == MyMoneyAccount::Expense) &&
      (parent.accountType() != MyMoneyAccount::Expense)) {
    postMessage ("CC", 3, child.name().toLatin1());
    return m_storage->expense();
  }
  return (parent);
  PASS
}
//*********************************** checkInvestmentOption *************************
void MyMoneyGncReader::checkInvestmentOption (QString stockId) {
  // implement the investment option for stock accounts
  // first check whether the parent account (gnucash id) is actually an
  // investment account. if it is, no further action is needed
  MyMoneyAccount stockAcc = m_storage->account (m_mapIds[stockId.toUtf8()]);
  MyMoneyAccount parent;
  QString parentKey = stockAcc.parentAccountId();
  map_accountIds::const_iterator id = m_mapIds.find (parentKey);
  if (id != m_mapIds.end()) {
    parent = m_storage->account (id.value());
    if (parent.accountType() == MyMoneyAccount::Investment) return ;
  }
  // so now, check the investment option requested by the user
  // option 0 creates a separate investment account for each stock account
  if (m_investmentOption == 0) {
    MyMoneyAccount invAcc (stockAcc);
    invAcc.setAccountType (MyMoneyAccount::Investment);
    invAcc.setCurrencyId (QString("")); // we don't know what currency it is!!
    invAcc.setParentAccountId (parentKey); // intersperse it between old parent and child stock acct
    m_storage->addAccount (invAcc);
    m_mapIds [invAcc.id()] = invAcc.id(); // so stock account gets parented (again) to investment account later
    if (gncdebug) qDebug ()
        << "Created investment account" << invAcc.name() << "as id" << invAcc.id()
        << "parent" << invAcc.parentAccountId();
    if (gncdebug) qDebug () << "Setting stock" << stockAcc.name() << "id" <<  stockAcc.id()
        << "as child of" << invAcc.id();
    stockAcc.setParentAccountId (invAcc.id());
    m_storage->addAccount(invAcc, stockAcc);
    // investment option 1 creates a single investment account for all stocks
  } else if (m_investmentOption == 1) {
    static QString singleInvAccId = "";
    MyMoneyAccount singleInvAcc;
    bool ok = false;
    if (singleInvAccId.isEmpty()) { // if the account has not yet been created
      QString invAccName;
      while (!ok) {
        invAccName = KInputDialog::getText (PACKAGE,
                                            i18n("Enter the investment account name "),
                                            i18n("My Investments"), &ok);
      }
      singleInvAcc.setName (invAccName);
      singleInvAcc.setAccountType (MyMoneyAccount::Investment);
      singleInvAcc.setCurrencyId (QString(""));
      singleInvAcc.setParentAccountId (m_storage->asset().id());
      m_storage->addAccount (singleInvAcc);
      m_mapIds [singleInvAcc.id()] = singleInvAcc.id(); // so stock account gets parented (again) to investment account later
      if (gncdebug) qDebug () << "Created investment account" << singleInvAcc.name()
          << "as id" << singleInvAcc.id() << "parent" << singleInvAcc.parentAccountId()
              << "reparenting stock";
      singleInvAccId = singleInvAcc.id();
    } else { // the account has already been created
      singleInvAcc = m_storage->account (singleInvAccId);
    }
    m_storage->addAccount(singleInvAcc, stockAcc); // add stock as child
    // the original intention of option 2 was to allow any asset account to be converted to an investment (broker) account
    // however, since we have already stored the accounts as asset, we have no way at present of changing their type
    // the only alternative would be to hold all the gnucash data in memory, then implement this option, then convert all the data
    // that would mean a major overhaul of the code. Perhaps I'll think of another way...
  } else if (m_investmentOption == 2) {
    static int lastSelected = 0;
    MyMoneyAccount invAcc (stockAcc);
    QStringList accList;
    QList<MyMoneyAccount> list;
    QList<MyMoneyAccount>::iterator acc;
    m_storage->accountList(list);
    // build a list of candidates for the input box
    for (acc = list.begin(); acc != list.end(); ++acc) {
      //      if (((*acc).accountGroup() == MyMoneyAccount::Asset) && ((*acc).accountType() != MyMoneyAccount::Stock)) accList.append ((*acc).name());
      if ((*acc).accountType() == MyMoneyAccount::Investment) accList.append ((*acc).name());
    }
    //if (accList.isEmpty()) qFatal ("No available accounts");
    bool ok = false;
    while (!ok) { // keep going till we have a valid investment parent
      QString invAccName = KInputDialog::getItem (
                             PACKAGE,
                             i18n("Select parent investment account or enter new name. Stock %1",stockAcc.name ()),
                             accList,
                             lastSelected, true, &ok);
      if (ok) {
        lastSelected = accList.indexOf (invAccName); // preserve selection for next time
        for (acc = list.begin(); acc != list.end(); ++acc) {
          if ((*acc).name() == invAccName) break;
        }
        if (acc != list.end()) { // an account was selected
          invAcc = *acc;
        } else {                 // a new account name was entered
          invAcc.setAccountType (MyMoneyAccount::Investment);
          invAcc.setName (invAccName);
          invAcc.setCurrencyId (QString(""));
          invAcc.setParentAccountId (m_storage->asset().id());
          m_storage->addAccount (invAcc);
          ok = true;
        }
        if (invAcc.accountType() == MyMoneyAccount::Investment) {
          ok = true;
        } else {
          // this code is probably not going to be implemented coz we can't change account types (??)
#if 0
          QMessageBox mb (PACKAGE,
                          i18n ("%1 is not an Investment Account. Do you wish to make it one?",invAcc.name()),
                          QMessageBox::Question,
                          QMessageBox::Yes | QMessageBox::Default,
                          QMessageBox::No | QMessageBox::Escape,
                          Qt::NoButton);
          switch (mb.exec()) {
          case QMessageBox::No :
            ok = false; break;
          default:
            // convert it - but what if it has splits???
            qFatal ("Not yet implemented");
            ok = true;
            break;
          }
#endif
          switch(KMessageBox::questionYesNo(0, i18n ("%1 is not an Investment Account. Do you wish to make it one?",invAcc.name()), PACKAGE)) {
            case KMessageBox::Yes:
              // convert it - but what if it has splits???
              qFatal ("Not yet implemented");
              ok = true;
              break;
            default:
              ok = false;
              break;
          }
        }
      } // end if ok - user pressed Cancel
    } // end while !ok
    m_mapIds [invAcc.id()] = invAcc.id(); // so stock account gets parented (again) to investment account later
    m_storage->addAccount(invAcc, stockAcc);
  } else { // investment option != 0, 1, 2
    qFatal ("Invalid investment option %d", m_investmentOption);
  }
}

// get the price source for a stock (gnc account) where online quotes are requested
void MyMoneyGncReader::getPriceSource (MyMoneySecurity stock, QString gncSource) {
  // if he wants to use Finance::Quote, no conversion of source name is needed
  if (m_useFinanceQuote) {
    stock.setValue ("kmm-online-quote-system", "Finance::Quote");
    stock.setValue ("kmm-online-source", gncSource.toLower());
    m_storage->modifySecurity(stock);
    return;
  }
  // first check if we have already asked about this source
  // (mapSources is initialy empty. We may be able to pre-fill it with some equivalent
  //  sources, if such things do exist. User feedback may help here.)
  QMap<QString, QString>::const_iterator it;
  for (it = m_mapSources.begin(); it != m_mapSources.end(); ++it) {
    if (it.key() == gncSource) {
      stock.setValue("kmm-online-source", it.value());
      m_storage->modifySecurity(stock);
      return;
    }
  }
  // not found in map, so ask the user
  KGncPriceSourceDlg *dlg = new KGncPriceSourceDlg (stock.name(), gncSource);
  dlg->exec();
  QString s = dlg->selectedSource();
  if (!s.isEmpty()) {
    stock.setValue("kmm-online-source", s);
    m_storage->modifySecurity(stock);
  }
  if (dlg->alwaysUse()) m_mapSources[gncSource] = s;
  delete dlg;
  return;
}

// functions to control the progress bar
//*********************** setProgressCallback *****************************
void MyMoneyGncReader::setProgressCallback(void(*callback)(int, int, const QString&)) {
  m_progressCallback = callback; return ;
}
//************************** signalProgress *******************************
void MyMoneyGncReader::signalProgress(int current, int total, const QString& msg) {
  if (m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
  return ;
}
// error and information reporting
//***************************** Information and error messages *********************
void MyMoneyGncReader::postMessage (const QString& source, const unsigned int code, const char* arg1) {
  postMessage (source, code, QStringList(arg1));
}
void MyMoneyGncReader::postMessage (const QString& source, const unsigned int code, const char* arg1, const char* arg2) {
  QStringList argList(arg1);
  argList.append(arg2);
  postMessage(source, code, argList);
}
void MyMoneyGncReader::postMessage (const QString& source, const unsigned int code, const char* arg1, const char* arg2, const char* arg3) {
  QStringList argList(arg1);
  argList.append(arg2);
  argList.append(arg3);
  postMessage(source, code, argList);
}
void MyMoneyGncReader::postMessage (const QString& source, const unsigned int code, const QStringList& argList) {
  int i;
  GncMessageArgs *m = new GncMessageArgs;

  m->source = source;
  m->code = code;
  // get the number of args this message requires
  const int argCount = GncMessages::argCount (source, code);
  if ((gncdebug) && (argCount != argList.count()))
    qDebug("%s", qPrintable(QString("MyMoneyGncReader::postMessage debug: Message %1, code %2, requires %3 arguments, got %4")
        .arg(source).arg(code).arg(argCount).arg(argList.count())));
  // store the arguments
  for (i = 0; i < argCount; ++i) {
    if (i > argList.count()) m->args.append(QString());
    else m->args.append (argList[i]); //Adds the next argument to the list
  }
  m_messageList.append (m);
  return ;
}
//********************************** Message texts **********************************************
GncMessages::messText GncMessages::texts [] = {
      {"CC", 1, i18n("An Investment account must be a child of an Asset account\n"
                            "Account %1 will be stored under the main Asset account")},
      {"CC", 2, i18n("An Income account must be a child of an Income account\n"
                            "Account %1 will be stored under the main Income account")},
      {"CC", 3, i18n("An Expense account must be a child of an Expense account\n"
                            "Account %1 will be stored under the main Expense account")},
      {"OR", 1, i18n("One or more transactions contain a reference to an otherwise unknown account\n"
                            "An asset account with the name %1 has been created to hold the data")},
      {"SC", 1, i18n("Schedule %1 has interval of %2 which is not currently available")},
      {"SC", 2, i18n("Schedule %1 dropped at user request")},
      {"SC", 3, i18n("Schedule %1 contains unknown action (key = %2, type = %3)")},
      {"SC", 4, i18n("Schedule %1 contains multiple actions; only one has been imported")},
      {"SC", 5, i18n("Schedule %1 contains no valid splits")},
      {"SC", 6, i18n("Schedule %1 appears to contain a formula. GnuCash formulae are not convertible")},
      {"SC", 7, i18n("Schedule %1 contains unknown interval specification; please check for correct operation")},
      {"SC", 8, i18n("Schedule %1 contains a deferred interval specification; please check for correct operation")},
      {"CC", 4, i18n("Account or Category %1, transaction date %2; split contains invalid value; please check")},
      {"ZZ", 0, ""} // stopper
    };
//
QString GncMessages::text (const QString source, const unsigned int code) {
  TRY
  unsigned int i;
  for (i = 0; texts[i].source != "ZZ"; i++) {
    if ((source == texts[i].source) && (code == texts[i].code)) break;
  }
  if (texts[i].source == "ZZ") {
    QString mess = QString().sprintf("Internal error - unknown message - source %s, code %i", qPrintable(source), code);
    throw new MYMONEYEXCEPTION (mess);
  }
  return (texts[i].text);
  PASS
}
//
unsigned int GncMessages::argCount (const QString source, const unsigned int code) {
  TRY
  unsigned int i;
  for (i = 0; texts[i].source != "ZZ"; i++) {
    if ((source == texts[i].source) && (code == texts[i].code)) break;
  }
  if (texts[i].source == "ZZ") {
    QString mess = QString().sprintf("Internal error - unknown message - source %s, code %d", qPrintable(source), code);
    throw new MYMONEYEXCEPTION (mess);
  }
  QRegExp argConst ("%\\d");
  /*int offset = 0;
  unsigned int argCount = 0;
  while ((offset = argConst.search (texts[i].text, offset)) != -1) {
    argCount++;
    offset += 2;
  }
  return (argCount);*/
  return (texts[i].text.count(argConst));
  PASS
}
#endif // _GNCFILEANON
