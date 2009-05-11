/***************************************************************************
                          mymoneystoragexml.cpp  -  description
                             -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2002 by Kevin Tambascio
                           (C) 2004 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "config-kmymoney.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qfile.h>
#include <qdom.h>
#include <qmap.h>
#include <qxml.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"
#include <klocale.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragexml.h"
#include "../mymoneyreport.h"
#include "../mymoneybudget.h"
#include "../mymoneyinstitution.h"

unsigned int MyMoneyStorageXML::fileVersionRead = 0;
unsigned int MyMoneyStorageXML::fileVersionWrite = 0;


class MyMoneyStorageXML::Private
{
  friend class MyMoneyStorageXML;
public:
  Private() {}

  QMap<QString, MyMoneyInstitution> iList;
  QMap<QString, MyMoneyAccount> aList;
  QMap<QString, MyMoneyTransaction> tList;
  QMap<QString, MyMoneyPayee> pList;
  QMap<QString, MyMoneySchedule> sList;
  QMap<QString, MyMoneySecurity> secList;
  QMap<QString, MyMoneyReport> rList;
  QMap<QString, MyMoneyBudget> bList;
  QMap<MyMoneySecurityPair, MyMoneyPriceEntries> prList;

  QString           m_fromSecurity;
  QString           m_toSecurity;

};


class MyMoneyXmlContentHandler : public QXmlContentHandler
{
public:
  MyMoneyXmlContentHandler(MyMoneyStorageXML* reader);
  virtual ~MyMoneyXmlContentHandler() {}
  virtual void setDocumentLocator (QXmlLocator * locator) { m_loc = locator; }
  virtual bool startDocument (void);
  virtual bool endDocument (void);
  virtual bool startPrefixMapping(const QString & prefix, const QString & uri);
  virtual bool endPrefixMapping(const QString & prefix);
  virtual bool startElement(const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts);
  virtual bool endElement(const QString & namespaceURI, const QString & localName, const QString & qName);
  virtual bool characters(const QString & ch);
  virtual bool ignorableWhitespace(const QString & ch);
  virtual bool processingInstruction(const QString & target, const QString & data);
  virtual bool skippedEntity(const QString & name);
  virtual QString errorString(void);

private:
  MyMoneyStorageXML* m_reader;
  QXmlLocator*       m_loc;
  int                m_level;
  int                m_elementCount;
  QDomDocument       m_doc;
  QDomElement        m_baseNode;
  QDomElement        m_currNode;
  QString            m_errMsg;
};

MyMoneyXmlContentHandler::MyMoneyXmlContentHandler(MyMoneyStorageXML* reader) :
  m_reader(reader),
  m_loc(0),
  m_level(0),
  m_elementCount(0)
{
}

bool MyMoneyXmlContentHandler::startDocument(void)
{
  qDebug("startDocument");
  return true;
}

bool MyMoneyXmlContentHandler::endDocument(void)
{
  qDebug("endDocument");
  return true;
}

bool MyMoneyXmlContentHandler::skippedEntity (const QString & /* name */)
{
  // qDebug(QString("Skipped entity '%1'").arg(name));
  return true;
}

bool MyMoneyXmlContentHandler::startPrefixMapping (const QString& /*prefix */, const QString & /* uri */)
{
  // qDebug(QString("start prefix '%1', '%2'").arg(prefix).arg(uri));
  return true;
}

bool MyMoneyXmlContentHandler::endPrefixMapping (const QString& /* prefix */)
{
  // qDebug(QString("end prefix '%1'").arg(prefix));
  return true;
}

bool MyMoneyXmlContentHandler::startElement (const QString& /* namespaceURI */, const QString& /* localName */, const QString& qName, const QXmlAttributes & atts)
{
  if(m_level == 0) {
    QString s = qName.toLower();
    if(s == "transaction"
    || s == "account"
    || s == "price"
    || s == "payee"
    || s == "currency"
    || s == "security"
    || s == "keyvaluepairs"
    || s == "institution"
    || s == "report"
    || s == "budget"
    || s == "fileinfo"
    || s == "user"
    || s == "scheduled_tx") {
      m_baseNode = m_doc.createElement(qName);
      for(int i=0; i < atts.count(); ++i) {
        m_baseNode.setAttribute(atts.qName(i), atts.value(i));
      }
      m_currNode = m_baseNode;
      m_level = 1;

    } else if(s == "transactions") {
      qDebug("reading transactions");
      if(atts.count()) {
        int count = atts.value(QString("count")).toUInt();
        m_reader->signalProgress(0, count, i18n("Loading transactions..."));
        m_elementCount = 0;
      }
    } else if(s == "accounts") {
      qDebug("reading accounts");
      if(atts.count()) {
        int count = atts.value(QString("count")).toUInt();
        m_reader->signalProgress(0, count, i18n("Loading accounts..."));
        m_elementCount = 0;
      }
    } else if(s == "securities") {
      qDebug("reading securities");
      if(atts.count()) {
        int count = atts.value(QString("count")).toUInt();
        m_reader->signalProgress(0, count, i18n("Loading securities..."));
        m_elementCount = 0;
      }
    } else if(s == "reports") {
      qDebug("reading reports");
      if(atts.count()) {
        int count = atts.value(QString("count")).toUInt();
        m_reader->signalProgress(0, count, i18n("Loading reports..."));
        m_elementCount = 0;
      }
    } else if(s == "prices") {
      qDebug("reading prices");
      m_elementCount = 0;
    } else if(s == "pricepair") {
      if(atts.count()) {
        m_reader->d->m_fromSecurity = atts.value(QString("from"));
        m_reader->d->m_toSecurity = atts.value(QString("to"));
      }
    }

  } else {
    m_level++;
    QDomElement node = m_doc.createElement(qName);
    for(int i=0; i < atts.count(); ++i) {
      node.setAttribute(atts.qName(i), atts.value(i));
    }
    m_currNode.appendChild(node);
    m_currNode = node;
  }
  return true;
}

bool MyMoneyXmlContentHandler::endElement(const QString& /* namespaceURI */, const QString& /* localName */, const QString& qName)
{
  bool rc = true;
  QString s = qName.toLower();
  if(m_level) {
    m_currNode = m_currNode.parentNode().toElement();
    m_level--;
    if(!m_level) {
      try {
        if(s == "transaction") {
          MyMoneyTransaction t(m_baseNode);
          if(!t.id().isEmpty())
            m_reader->d->tList[t.uniqueSortKey()] = t;
        } else if(s == "account") {
          MyMoneyAccount a(m_baseNode);
          if(!a.id().isEmpty())
            m_reader->d->aList[a.id()] = a;
        } else if(s == "payee") {
          MyMoneyPayee p(m_baseNode);
          if(!p.id().isEmpty())
            m_reader->d->pList[p.id()] = p;
        } else if(s == "currency") {
          MyMoneySecurity s(m_baseNode);
          if(!s.id().isEmpty())
            m_reader->d->secList[s.id()] = s;
        } else if(s == "security") {
          MyMoneySecurity s(m_baseNode);
          if(!s.id().isEmpty())
            m_reader->d->secList[s.id()] = s;
        } else if(s == "keyvaluepairs") {
          MyMoneyKeyValueContainer kvp(m_baseNode);
          m_reader->m_storage->setPairs(kvp.pairs());
        } else if(s == "institution") {
          MyMoneyInstitution i(m_baseNode);
          if(!i.id().isEmpty())
            m_reader->d->iList[i.id()] = i;
        } else if(s == "report") {
          MyMoneyReport r(m_baseNode);
          if(!r.id().isEmpty())
            m_reader->d->rList[r.id()] = r;
        } else if(s == "budget") {
          MyMoneyBudget b(m_baseNode);
          if(!b.id().isEmpty())
            m_reader->d->bList[b.id()] = b;
        } else if(s == "fileinfo") {
          rc = m_reader->readFileInformation(m_baseNode);
        } else if(s == "user") {
          rc = m_reader->readUserInformation(m_baseNode);
        } else if(s == "scheduled_tx") {
          MyMoneySchedule s(m_baseNode);
          if(!s.id().isEmpty())
            m_reader->d->sList[s.id()] = s;
        } else if(s == "price") {
          MyMoneyPrice p(m_reader->d->m_fromSecurity, m_reader->d->m_toSecurity, m_baseNode);
          m_reader->d->prList[MyMoneySecurityPair(m_reader->d->m_fromSecurity, m_reader->d->m_toSecurity)][p.date()] = p;
        } else {
          m_errMsg = i18n("Unknown XML tag %1 found in line %2").arg(qName).arg(m_loc->lineNumber());
          kWarning() << m_errMsg;
          rc = false;
        }
        m_reader->signalProgress(++m_elementCount, 0);
      } catch(MyMoneyException* e) {
        m_errMsg = i18n("Exception while creating a %1 element: %2").arg(s).arg(e->what());
        kWarning() << m_errMsg;
        delete e;
        rc = false;
      }
      m_doc = QDomDocument();
    }
  } else {
    if(s == "institutions") {
      // last institution read, now dump them into the engine
      m_reader->m_storage->loadInstitutions(m_reader->d->iList);
      m_reader->d->iList.clear();
      m_reader->signalProgress(-1, -1);
    } else if(s == "accounts") {
      // last account read, now dump them into the engine
      m_reader->m_storage->loadAccounts(m_reader->d->aList);
      m_reader->d->aList.clear();
      m_reader->signalProgress(-1, -1);
    } else if(s == "payees") {
      // last payee read, now dump them into the engine
      m_reader->m_storage->loadPayees(m_reader->d->pList);
      m_reader->d->pList.clear();
      m_reader->signalProgress(-1, -1);
    } else if(s == "transactions") {
      // last transaction read, now dump them into the engine
      m_reader->m_storage->loadTransactions(m_reader->d->tList);
      m_reader->d->tList.clear();
      m_reader->signalProgress(-1, -1);
    } else if(s == "schedules") {
      // last schedule read, now dump them into the engine
      m_reader->m_storage->loadSchedules(m_reader->d->sList);
      m_reader->d->sList.clear();
      m_reader->signalProgress(-1, -1);
    } else if(s == "securities") {
      // last security read, now dump them into the engine
      m_reader->m_storage->loadSecurities(m_reader->d->secList);
      m_reader->d->secList.clear();
      m_reader->signalProgress(-1, -1);
    } else if(s == "currencies") {
      // last currency read, now dump them into the engine
      m_reader->m_storage->loadCurrencies(m_reader->d->secList);
      m_reader->d->secList.clear();
      m_reader->signalProgress(-1, -1);
    } else if(s == "reports") {
      // last report read, now dump them into the engine
      m_reader->m_storage->loadReports(m_reader->d->rList);
      m_reader->d->rList.clear();
      m_reader->signalProgress(-1, -1);
    } else if(s == "budgets") {
      // last budget read, now dump them into the engine
      m_reader->m_storage->loadBudgets(m_reader->d->bList);
      m_reader->d->bList.clear();
      m_reader->signalProgress(-1, -1);
    } else if(s == "prices") {
      // last price read, now dump them into the engine
      m_reader->m_storage->loadPrices(m_reader->d->prList);
      m_reader->d->bList.clear();
      m_reader->signalProgress(-1, -1);
    }
  }
  return rc;
}

bool MyMoneyXmlContentHandler::characters(const QString& /* ch */)
{
  return true;
}

bool MyMoneyXmlContentHandler::ignorableWhitespace(const QString& /* ch */)
{
  return true;
}

bool MyMoneyXmlContentHandler::processingInstruction(const QString& /* target */, const QString& /* data */)
{
  return true;
}

QString MyMoneyXmlContentHandler::errorString(void)
{
  return m_errMsg;
}







MyMoneyStorageXML::MyMoneyStorageXML() :
  m_storage(0),
  m_doc(0),
  d(new Private())
{
}

MyMoneyStorageXML::~MyMoneyStorageXML()
{
  delete d;
}

// Function to read in the file, send to XML parser.
void MyMoneyStorageXML::readFile(QIODevice* pDevice, IMyMoneySerialize* storage)
{
  Q_CHECK_PTR(storage);
  Q_CHECK_PTR(pDevice);
  if(!storage)
    return;

  m_storage = storage;

  m_doc = new QDomDocument;
  Q_CHECK_PTR(m_doc);

  qDebug("reading file");
  // creating the QXmlInputSource object based on a QIODevice object
  // reads all data of the underlying object into memory. I have not
  // found an object that reads on the fly. I tried to derive one myself,
  // but there could be a severe problem with decoding when reading
  // blocks of data and not a stream. So I left it the way it is. (ipwizard)
  QXmlInputSource xml(pDevice);

  qDebug("start parsing file");
  MyMoneyXmlContentHandler mmxml(this);
  QXmlSimpleReader reader;
  reader.setContentHandler(&mmxml);

  if(!reader.parse(&xml, false)) {
    delete m_doc;
    m_doc = NULL;
    signalProgress(-1, -1);
    throw new MYMONEYEXCEPTION("File was not parsable!");
  }

  // check if we need to build up the account balances
  if(fileVersionRead < 2)
    m_storage->rebuildAccountBalances();

  delete m_doc;
  m_doc = NULL;

  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
  m_storage->setLastModificationDate(m_storage->lastModificationDate());
  m_storage = NULL;

  //hides the progress bar.
  signalProgress(-1, -1);
}

void MyMoneyStorageXML::writeFile(QIODevice* qf, IMyMoneySerialize* storage)
{
  Q_CHECK_PTR(qf);
  Q_CHECK_PTR(storage);
  if(!storage)
  {
    return;
  }
  m_storage = storage;

  // qDebug("XMLWRITER: Starting file write");
  m_doc = new QDomDocument("KMYMONEY-FILE");
  Q_CHECK_PTR(m_doc);
  QDomProcessingInstruction instruct = m_doc->createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
  m_doc->appendChild(instruct);

  QDomElement mainElement = m_doc->createElement("KMYMONEY-FILE");
  m_doc->appendChild(mainElement);

  QDomElement fileInfo = m_doc->createElement("FILEINFO");
  writeFileInformation(fileInfo);
  mainElement.appendChild(fileInfo);

  QDomElement userInfo = m_doc->createElement("USER");
  writeUserInformation(userInfo);
  mainElement.appendChild(userInfo);

  QDomElement institutions = m_doc->createElement("INSTITUTIONS");
  writeInstitutions(institutions);
  mainElement.appendChild(institutions);

  QDomElement payees = m_doc->createElement("PAYEES");
  writePayees(payees);
  mainElement.appendChild(payees);

  QDomElement accounts = m_doc->createElement("ACCOUNTS");
  writeAccounts(accounts);
  mainElement.appendChild(accounts);

  QDomElement transactions = m_doc->createElement("TRANSACTIONS");
  writeTransactions(transactions);
  mainElement.appendChild(transactions);

  QDomElement keyvalpairs = writeKeyValuePairs(m_storage->pairs());
  mainElement.appendChild(keyvalpairs);

  QDomElement schedules = m_doc->createElement("SCHEDULES");
  writeSchedules(schedules);
  mainElement.appendChild(schedules);

  QDomElement equities = m_doc->createElement("SECURITIES");
  writeSecurities(equities);
  mainElement.appendChild(equities);

  QDomElement currencies = m_doc->createElement("CURRENCIES");
  writeCurrencies(currencies);
  mainElement.appendChild(currencies);

  QDomElement prices = m_doc->createElement("PRICES");
  writePrices(prices);
  mainElement.appendChild(prices);

  QDomElement reports = m_doc->createElement("REPORTS");
  writeReports(reports);
  mainElement.appendChild(reports);

  QDomElement budgets = m_doc->createElement("BUDGETS");
  writeBudgets(budgets);
  mainElement.appendChild(budgets);

  Q3TextStream stream(qf);
  stream.setEncoding(Q3TextStream::UnicodeUTF8);
  stream << m_doc->toString();

  delete m_doc;
  m_doc = NULL;

  //hides the progress bar.
  signalProgress(-1, -1);

  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
  m_storage->setLastModificationDate(m_storage->lastModificationDate());

  m_storage = NULL;
}

bool MyMoneyStorageXML::readFileInformation(const QDomElement& fileInfo)
{
  signalProgress(0, 3, i18n("Loading file information..."));
  bool rc = true;
  QDomElement temp = findChildElement("CREATION_DATE", fileInfo);
  if (temp == QDomElement()) {
    rc = false;
  }
  QString strDate = QStringEmpty(temp.attribute("date"));
  m_storage->setCreationDate(stringToDate(strDate));
  signalProgress(1, 0);

  temp = findChildElement("LAST_MODIFIED_DATE", fileInfo);
  if (temp == QDomElement()) {
    rc = false;
  }
  strDate = QStringEmpty(temp.attribute("date"));
  m_storage->setLastModificationDate(stringToDate(strDate));
  signalProgress(2, 0);

  temp = findChildElement("VERSION", fileInfo);
  if (temp == QDomElement()) {
    rc = false;
  }
  QString strVersion = QStringEmpty(temp.attribute("id"));
  fileVersionRead = strVersion.toUInt(NULL, 16);

  temp = findChildElement("FIXVERSION", fileInfo);
  if (temp != QDomElement()) {
    QString strFixVersion = QStringEmpty(temp.attribute("id"));
    m_storage->setFileFixVersion (strFixVersion.toUInt());
  }
  // FIXME The old version stuff used this rather odd number
  //       We now use increments
  if(fileVersionRead == VERSION_0_60_XML)
    fileVersionRead = 1;
  signalProgress(3, 0);

  return rc;
}

void MyMoneyStorageXML::writeFileInformation(QDomElement& fileInfo)
{
  QDomElement creationDate = m_doc->createElement("CREATION_DATE");
  creationDate.setAttribute("date", dateToString(m_storage->creationDate()));
  fileInfo.appendChild(creationDate);

  QDomElement lastModifiedDate = m_doc->createElement("LAST_MODIFIED_DATE");
  lastModifiedDate.setAttribute("date", dateToString(m_storage->lastModificationDate()));
  fileInfo.appendChild(lastModifiedDate);

  QDomElement version = m_doc->createElement("VERSION");

  version.setAttribute("id", "1");
  fileInfo.appendChild(version);

  QDomElement fixVersion = m_doc->createElement("FIXVERSION");
  fixVersion.setAttribute("id", m_storage->fileFixVersion());
  fileInfo.appendChild(fixVersion);
}

void MyMoneyStorageXML::writeUserInformation(QDomElement& userInfo)
{
  MyMoneyPayee user = m_storage->user();
  userInfo.setAttribute("name", user.name());
  userInfo.setAttribute("email", user.email());

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute("street", user.address());
  address.setAttribute("city", user.city());
  address.setAttribute("county", user.state());
  address.setAttribute("zipcode", user.postcode());
  address.setAttribute("telephone", user.telephone());

  userInfo.appendChild(address);
}

bool MyMoneyStorageXML::readUserInformation(const QDomElement& userElement)
{
  bool rc = true;
  signalProgress(0, 1, i18n("Loading user information..."));

  MyMoneyPayee user;
  user.setName(QStringEmpty(userElement.attribute("name")));
  user.setEmail(QStringEmpty(userElement.attribute("email")));

  QDomElement addressNode = findChildElement("ADDRESS", userElement);
  if(!addressNode.isNull()) {
    user.setAddress(QStringEmpty(addressNode.attribute("street")));
    user.setCity(QStringEmpty(addressNode.attribute("city")));
    user.setState(QStringEmpty(addressNode.attribute("county")));
    user.setPostcode(QStringEmpty(addressNode.attribute("zipcode")));
    user.setTelephone(QStringEmpty(addressNode.attribute("telephone")));
  }

  m_storage->setUser(user);
  signalProgress(1, 0);

  return rc;
}

void MyMoneyStorageXML::writeInstitutions(QDomElement& institutions)
{
  const Q3ValueList<MyMoneyInstitution> list = m_storage->institutionList();
  Q3ValueList<MyMoneyInstitution>::ConstIterator it;
  institutions.setAttribute("count", list.count());

  for(it = list.begin(); it != list.end(); ++it)
    writeInstitution(institutions, *it);
}

void MyMoneyStorageXML::writeInstitution(QDomElement& institution, const MyMoneyInstitution& i)
{
  i.writeXML(*m_doc, institution);
}

void MyMoneyStorageXML::writePayees(QDomElement& payees)
{
  const Q3ValueList<MyMoneyPayee> list = m_storage->payeeList();
  Q3ValueList<MyMoneyPayee>::ConstIterator it;
  payees.setAttribute("count", list.count());

  for(it = list.begin(); it != list.end(); ++it)
    writePayee(payees, *it);
}

void MyMoneyStorageXML::writePayee(QDomElement& payee, const MyMoneyPayee& p)
{
  p.writeXML(*m_doc, payee);
}

void MyMoneyStorageXML::writeAccounts(QDomElement& accounts)
{
  Q3ValueList<MyMoneyAccount> list;
  m_storage->accountList(list);
  Q3ValueList<MyMoneyAccount>::ConstIterator it;
  accounts.setAttribute("count", list.count()+5);

  writeAccount(accounts, m_storage->asset());
  writeAccount(accounts, m_storage->liability());
  writeAccount(accounts, m_storage->expense());
  writeAccount(accounts, m_storage->income());
  writeAccount(accounts, m_storage->equity());

  signalProgress(0, list.count(), i18n("Saving accounts..."));
  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i) {
    writeAccount(accounts, *it);
    signalProgress(i, 0);
  }
}

void MyMoneyStorageXML::writeAccount(QDomElement& account, const MyMoneyAccount& p)
{
  p.writeXML(*m_doc, account);
}

void MyMoneyStorageXML::writeTransactions(QDomElement& transactions)
{
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  Q3ValueList<MyMoneyTransaction> list;
  m_storage->transactionList(list, filter);
  transactions.setAttribute("count", list.count());

  Q3ValueList<MyMoneyTransaction>::ConstIterator it;

  signalProgress(0, list.count(), i18n("Saving transactions..."));

  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i)
  {
    writeTransaction(transactions, *it);
    signalProgress(i, 0);
  }
}

void MyMoneyStorageXML::writeTransaction(QDomElement& transaction, const MyMoneyTransaction& tx)
{
  tx.writeXML(*m_doc, transaction);
}

void MyMoneyStorageXML::writeSchedules(QDomElement& scheduled)
{
  const Q3ValueList<MyMoneySchedule> list = m_storage->scheduleList();
  Q3ValueList<MyMoneySchedule>::ConstIterator it;
  scheduled.setAttribute("count", list.count());

  for(it = list.begin(); it != list.end(); ++it)
  {
    this->writeSchedule(scheduled, *it);
  }
}

void MyMoneyStorageXML::writeSchedule(QDomElement& scheduledTx, const MyMoneySchedule& tx)
{
  tx.writeXML(*m_doc, scheduledTx);
}

void MyMoneyStorageXML::writeSecurities(QDomElement& equities)
{
  const Q3ValueList<MyMoneySecurity> securityList = m_storage->securityList();
  equities.setAttribute("count", securityList.count());
  if(securityList.size())
  {
    for(Q3ValueList<MyMoneySecurity>::ConstIterator it = securityList.begin(); it != securityList.end(); ++it)
    {
      writeSecurity(equities, (*it));
    }
  }
}

void MyMoneyStorageXML::writeSecurity(QDomElement& securityElement, const MyMoneySecurity& security)
{
  security.writeXML(*m_doc, securityElement);
}

void MyMoneyStorageXML::writeCurrencies(QDomElement& currencies)
{
  const Q3ValueList<MyMoneySecurity> currencyList = m_storage->currencyList();
  currencies.setAttribute("count", currencyList.count());
  if(currencyList.size())
  {
    for(Q3ValueList<MyMoneySecurity>::ConstIterator it = currencyList.begin(); it != currencyList.end(); ++it)
    {
      writeSecurity(currencies, (*it));
    }
  }
}

void MyMoneyStorageXML::writeReports(QDomElement& parent)
{
  const Q3ValueList<MyMoneyReport> list = m_storage->reportList();
  Q3ValueList<MyMoneyReport>::ConstIterator it;
  parent.setAttribute("count", list.count());

  signalProgress(0, list.count(), i18n("Saving reports..."));
  unsigned i = 0;
  for(it = list.begin(); it != list.end(); ++it)
  {
    (*it).writeXML(*m_doc, parent);
    signalProgress(++i, 0);
  }
}

void MyMoneyStorageXML::writeBudgets(QDomElement& parent)
{
  const Q3ValueList<MyMoneyBudget> list = m_storage->budgetList();
  Q3ValueList<MyMoneyBudget>::ConstIterator it;
  parent.setAttribute("count", list.count());

  signalProgress(0, list.count(), i18n("Saving budgets..."));
  unsigned i = 0;
  for(it = list.begin(); it != list.end(); ++it)
  {
    writeBudget(parent, (*it));
    signalProgress(++i, 0);
  }
}

void MyMoneyStorageXML::writeBudget(QDomElement& budget, const MyMoneyBudget& b)
{
  b.writeXML(*m_doc, budget);
}


QDomElement MyMoneyStorageXML::findChildElement(const QString& name, const QDomElement& root)
{
  QDomNode child = root.firstChild();
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(name == childElement.tagName())
      {
        return childElement;
      }
    }

    child = child.nextSibling();
  }
  return QDomElement();
}

QDomElement MyMoneyStorageXML::writeKeyValuePairs(const QMap<QString, QString> pairs)
{
  if(m_doc)
  {
    QDomElement keyValPairs = m_doc->createElement("KEYVALUEPAIRS");

    QMap<QString, QString>::const_iterator it;
    for(it = pairs.begin(); it != pairs.end(); ++it)
    {
      QDomElement pair = m_doc->createElement("PAIR");
      pair.setAttribute("key", it.key());
      pair.setAttribute("value", it.data());
      keyValPairs.appendChild(pair);
    }
    return keyValPairs;
  }
  return QDomElement();
}

void MyMoneyStorageXML::writePrices(QDomElement& prices)
{
  const MyMoneyPriceList list = m_storage->priceList();
  MyMoneyPriceList::ConstIterator it;
  prices.setAttribute("count", list.count());

  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement price = m_doc->createElement("PRICEPAIR");
    price.setAttribute("from", it.key().first);
    price.setAttribute("to", it.key().second);
    writePricePair(price, *it);
    prices.appendChild(price);
  }
}

void MyMoneyStorageXML::writePricePair(QDomElement& price, const MyMoneyPriceEntries& p)
{
  MyMoneyPriceEntries::ConstIterator it;
  for(it = p.begin(); it != p.end(); ++it) {
    QDomElement entry = m_doc->createElement("PRICE");
    writePrice(entry, *it);
    price.appendChild(entry);
  }
}

void MyMoneyStorageXML::writePrice(QDomElement& price, const MyMoneyPrice& p)
{
  price.setAttribute("date", p.date().toString(Qt::ISODate));
  price.setAttribute("price", p.rate(QString()).toString());
  price.setAttribute("source", p.source());
}

void MyMoneyStorageXML::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyStorageXML::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

/*!
    This convenience function returns all of the remaining data in the
    device.

    @note It's copied from the original Qt sources and modified to
          fix a problem with KFilterDev that does not correctly return
          atEnd() status in certain circumstances which caused our
          application to lock at startup.
*/
QByteArray QIODevice::readAll()
{
  if ( isDirectAccess() ) {
    // we know the size
    int n = size()-at(); // ### fix for 64-bit or large files?
    int totalRead = 0;
    QByteArray ba( n );
    char* c = ba.data();
    while ( n ) {
      int r = read( c, n );
      if ( r < 0 )
        return QByteArray();
      n -= r;
      c += r;
      totalRead += r;
      // If we have a translated file, then it is possible that
      // we read less bytes than size() reports
      if ( atEnd() ) {
        ba.resize( totalRead );
        break;
      }
    }
    return ba;
  } else {
    // read until we reach the end
    const int blocksize = 512;
    int nread = 0;
    QByteArray ba;
    int r = 1;
    while ( !atEnd() && r != 0) {
      ba.resize( nread + blocksize );
      r = read( ba.data()+nread, blocksize );
      if ( r < 0 )
        return QByteArray();
      nread += r;
    }
    ba.resize( nread );
    return ba;
  }
}

