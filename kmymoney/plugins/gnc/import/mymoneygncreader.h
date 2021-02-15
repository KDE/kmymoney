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
                        (C) 2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
/*
The main class of this module, MyMoneyGncReader, contains only a readFile()
function, which controls the import of data from an XML file created by the
current GnuCash version (1.8.8).

The XML is processed in class XmlReader, which is an implementation of the Qt
SAX2 reader class.

Data in the input file is processed as a set of objects which fortunately,
though perhaps not surprisingly, have almost a one-for-one correspondence with
KMyMoney objects. These objects are bounded by start and end XML elements, and
may contain both nested objects (described as sub objects in the code), and data
items, also delimited by start and end elements. For example:
<gnc:account> * start of sub object within file
  <act:name>Account Name</act:name> * data string with start and end elements
  ...
</gnc:account> * end of sub objects

A GnuCash file may consist of more than one 'book', or set of data. It is not
clear how we could currently implement this, so only the first book in a file is
processed. This should satisfy most user situations.

GnuCash is somewhat inconsistent in its division of the major sections of the
file. For example, multiple price history entries are delimited by <gnc:pricedb>
elements, while each account starts with  its own top-level element. In general,
the 'container' elements are ignored.

XmlReader

This is an implementation of the Qt QXmlDefaultHandler class, which provides
three main function calls in addition to start and end of document. The
startElement() and endElement() calls are self-explanatory, the characters()
function provides data strings. Thus in the above example, the sequence of calls
would be
  startElement() for gnc:account
  startElement() for act:name
   characters() for 'Account Name'
  endElement() for act:name
  ...
  endElement() for gnc:account

Objects

Since the processing requirements of XML for most elements are very similar, the
common code is implemented in a GncObject class, from which the others are
derived, with virtual function calls to cater for any differences. The
'grandfather' object, GncFile representing the file (or more correctly, 'book')
as a whole, is created in the startDocument() function call.

The constructor function of each object is responsible for providing two lists
for the XmlReader to scan, a list of element names which represent sub objects
(called sub elements in the code), and a similar list of names representing data
elements. In addition, an array of variables (m_v) is provided and initialized,
to contain the actual data strings.

Implementation

Since objects may be nested, a stack is used, with the top element pointing to
the 'current object'. The startDocument() call creates the first, GncFile,
object at the top of the stack.

As each startElement() call occurs, the two#include "mymoneygncreader.h"
 element lists created by the current
object are scanned.
If this element represents the start of a sub object, the current object's subEl()
function is called to create an instance of the appropriate type. This is then
pushed to the top of the stack, and the new object's initiate() function is
called. This is used to process any XML attributes attached to the element;
GnuCash makes little use of these.
If this represents the start of a data element, a pointer (m_dataPointer) is set
to point to an entry in the array (m_v) in which a subsequent characters() call
can store the actual data.

When an endElement() call occurs, a check is made to see if it matches the
element name which started the current object. If so, the object's terminate()
function is called. If the object represents a similar KMM object, this will
normally result in a call to a conversion routine in the main
(MyMoneyGncReader) class to convert the data to native format and place it in
storage. The stack is then popped, and the parent (now current) object notified
by a call to its endSubEl() function. Again depending on the type of object,
this will either delete the instance, or save it in its own storage for later
processing.
For example, a GncSplit object makes little sense outside the context of its
transaction, so will be saved by the transaction. A GncTransaction object on the
other hand will be converted, along with its attendant splits, and then deleted
by its parent.

Since at any one time an object will only be processing either a subobject or a
data element, a single object variable, m_state, is used to determine the actual
type. In effect, it acts as the current index into either the subElement or
dataElement list. As an object variable, it will be saved on the stack across
subobject processing.

Exceptions and Problems

Fatal exceptions are processed via the standard MyMoneyException method.
Due to differences in implementation between GnuCash and KMM, it is not always
possible to provide an absolutely correct conversion. When such a problem
situation is recognized, a message, along with any relevant variable data, is
passed to the main class, and used to produce a report when processing
terminates.

Anonymizer

When debugging problems, it is often useful to have a trace of what is happening
within the module. However, in view of the sensitive nature of personal finance
data, most users will be reluctant to provide this. Accordingly, an anonymize
(hide()) function is provided to handle data strings. These may either be passed
through asis (non-personal data), blanked out (non-critical but possibly personal
data), replaced with a generated version (required, but possibly personal), or
randomized (monetary amounts). The action for each data item is determined in
the object's constructor function along with the creation of the data element
list.
This module will later be used as the basis of a file anonymizer, which will
enable users to safely provide us with a copy of their GnuCash files, and will
allow us to test the structure, if not the data content, of the file.
*/

#ifndef MYMONEYGNCREADER_H
#define MYMONEYGNCREADER_H

// system includes
#include <stdlib.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QStack>
#include <QXmlDefaultHandler>
#include <QDate>

// ----------------------------------------------------------------------------
// Project Includes

#ifndef _GNCFILEANON
#include "storage/imymoneystorageformat.h"
#endif // _GNCFILEANON

// not sure what these are for, but leave them in
#define VERSION_0_60_XML  0x10000010    // Version 0.5 file version info
#define VERSION_0_61_XML  0x10000011    // use 8 bytes for MyMoneyMoney objects
#define GNUCASH_ID_KEY "GNUCASH_ID"

class MyMoneyAccount;
class MyMoneySecurity;
class MyMoneyTransaction;
class MyMoneySplit;

typedef QMap<QString, QString> map_accountIds;
typedef map_accountIds::iterator map_accountIds_iter;
typedef map_accountIds::const_iterator map_accountIds_citer;

typedef QMap<QString, QStringList> map_elementVersions;

class MyMoneyGncReader;
class QIODevice;
class QDate;
class QTextCodec;
class MyMoneyStorageMgr;
class QXmlAttributes;
class QXmlInputSource;
class QXmlSimpleReader;

/** GncObject is the base class for the various objects in the gnucash file
    Beyond the first level XML objects, elements will be of one of three types:
     1. Sub object elements, which require creation of another object to process
     2. Data object elements, which are only followed by data to be stored in a variable (m_v array)
     3. Ignored objects, data not needed and not included herein
*/
class GncKvp;
class GncObject
{
public:
  GncObject();
  virtual ~GncObject() {}  // make sure to have impl  of all virtual rtns to avoid vtable errors?
protected:
  friend class XmlReader;
  friend class MyMoneyGncReader;
  // check for sub object element; if it is, create the object
  GncObject *isSubElement(const QString &elName, const QXmlAttributes& elAttrs);
  // check for data element; if so, set data pointer
  bool isDataElement(const QString &elName, const QXmlAttributes& elAttrs);
  // process start element for 'this'; normally for attribute checking; other initialization done in constructor
  virtual void initiate(const QString&, const QXmlAttributes&) {
    return ;
  }
  // a sub object has completed; process the data it gathered
  virtual void endSubEl(GncObject *) {
    m_dataPtr = 0; return ;
  }
  // store data for data element
  void storeData(const QString& pData) { // NB - data MAY come in chunks, and may need to be anonymized
    if (m_dataPtr != 0)
      m_dataPtr->append(hide(pData, m_anonClass));
  }
  // following is provided only for a future file anonymizer
  QString getData() const {
    return ((m_dataPtr != 0) ? *m_dataPtr : "");
  }
  void resetDataPtr() {
    m_dataPtr = 0;
  }
  // process end element for 'this'; usually to convert to KMM format
  virtual void terminate() {
    return ;
  }
  void setVersion(const QString& v) {
    m_version = v; return;
  }
  QString version() const {
    return (m_version);
  }

  // some gnucash elements have version attribute; check it
  void checkVersion(const QString&, const QXmlAttributes&, const map_elementVersions&);
  // get name of element processed by 'this'
  QString getElName() const {
    return (m_elementName);
  }
  // pass 'main' pointer to object
  void setPm(MyMoneyGncReader *pM) {
    pMain = pM;
  }
  const QString getKvpValue(const QString& key, const QString& type = QString()) const;

  // debug only
  void debugDump();

  // called by isSubElement to create appropriate sub object
  virtual GncObject *startSubEl() {
    return (0);
  }
  // called by isDataElement to set variable pointer
  virtual void dataEl(const QXmlAttributes&) {
    m_dataPtr = &(m_v[m_state]); m_anonClass = m_anonClassList[m_state];
  }
  // return gnucash data string variable pointer
  virtual QString var(int i) const;
  // anonymize data
  virtual QString hide(QString, unsigned int);
  unsigned int kvpCount() const {
    return (m_kvpList.count());
  } //!

  MyMoneyGncReader *pMain;    // pointer to 'main' class
  // used at start of each transaction so same money hide factor is applied to all splits
  void adjustHideFactor();

  QString m_elementName; // save 'this' element's name
  QString m_version;     // and it's gnucash version
  const QString *m_subElementList; // list of sub object element names for 'this'
  unsigned int m_subElementListCount; // count of above
  const QString *m_dataElementList; // ditto for data elements
  unsigned int m_dataElementListCount;
  QString *m_dataPtr; // pointer to m_v variable for current data item
  mutable QList<QString> m_v; // storage for variable pointers

  unsigned int m_state; // effectively, the index to subElementList or dataElementList, whichever is currently in use

  const unsigned int *m_anonClassList;
  enum anonActions {ASIS, SUPPRESS, NXTACC, NXTEQU, NXTPAY, NXTSCHD, MAYBEQ, MONEY1, MONEY2}; // anonymize actions - see hide()
  unsigned int m_anonClass; // class of current data item for anonymizer
  static double m_moneyHideFactor; // a per-transaction factor
  QList<GncKvp> m_kvpList; //!
};

// *****************************************************************************
// This is the 'grandfather' object representing the gnucash file as a whole
class GncFile : public GncObject
{
public:
  GncFile();
  ~GncFile();
private:
  enum iSubEls {BOOK, COUNT, CMDTY, PRICE, ACCT, TX, TEMPLATES, SCHEDULES, END_FILE_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;

  bool m_processingTemplates; // gnc uses same transaction element for ordinary and template tx's; this will distinguish
  bool m_bookFound;  // to  detect multi-book files
};
// The following are 'utility' objects, which occur within several other object types
// ************* GncKvp********************************************
// Key/value pairs, which are introduced by the 'slot' element
// Consist of slot:key (the 'name' of the kvp), and slot:value (the data value)
// the slot value also contains a slot type (string, integer, etc) implemented as an XML attribute
// kvp's may be nested
class GncKvp : public GncObject
{
public:
  GncKvp();
  ~GncKvp();
//protected:
  friend class MyMoneyGncReader;

  QString key() const {
    return (var(KEY));
  }
  QString value() const {
    return (var(VALUE));
  }
  QString type() const {
    return (m_kvpType);
  }
  const GncKvp getKvp(unsigned int i) const {
    return (m_kvpList[i]);
  }
private:
  // subsidiary objects/elements
  enum KvpSubEls {KVP, END_Kvp_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;
  // data elements
  enum KvpDataEls {KEY, VALUE, END_Kvp_DELS };
  void dataEl(const QXmlAttributes&) final override;
  QString m_kvpType;  // type is an XML attribute
};
// ************* GncLot********************************************
// KMM doesn't have support for lots as yet
class GncLot : public GncObject
{
public:
  GncLot();
  ~GncLot();
protected:
  friend class MyMoneyGncReader;
private:
};
// ****************************************************************************
// commodity specification. consists of
//  cmdty:space - either ISO4217 if this cmdty is a currency, or, usually, the name of a stock exchange
//  cmdty:id - ISO4217 currency symbol, or 'ticker symbol'
class GncCmdtySpec : public GncObject
{
public:
  GncCmdtySpec();
  ~GncCmdtySpec();
protected:
  friend class MyMoneyGncReader;
  friend class GncTransaction;
  bool isCurrency() const {
    return (m_v[CMDTYSPC] == QStringLiteral("ISO4217") || m_v[CMDTYSPC] == QStringLiteral("CURRENCY"));
  };
  QString id() const {
    return (m_v[CMDTYID]);
  };
  QString space() const {
    return (m_v[CMDTYSPC]);
  };
private:
  // data elements
  enum CmdtySpecDataEls {CMDTYSPC, CMDTYID, END_CmdtySpec_DELS};
  QString hide(QString, unsigned int) final override;
};
// *********************************************************************
// date; maybe one of two types, ts:date which is date/time, gdate which is date only
// we do not preserve time data (at present)
class GncDate : public GncObject
{
public:
  GncDate();
  ~GncDate();
protected:
  friend class MyMoneyGncReader;
  friend class GncPrice;
  friend class GncTransaction;
  friend class GncSplit;
  friend class GncSchedule;
  friend class GncRecurrence;
  const QDate date() const {
    return (QDate::fromString(m_v[TSDATE].section(' ', 0, 0), Qt::ISODate));
  };
private:
  // data elements
  enum DateDataEls {TSDATE, GDATE, END_Date_DELS};
  void dataEl(const QXmlAttributes&) final override {
    m_dataPtr = &(m_v[TSDATE]); m_anonClass = GncObject::ASIS;
  }
   // treat both date types the same
};

/** Following are the main objects within the gnucash file, which correspond largely one-for-one
    with similar objects in the kmymoney structure, apart from schedules which gnc splits between
    template (transaction data) and schedule (date data)
*/
//********************************************************************
class GncCountData : public GncObject
{
public:
  GncCountData();
  ~GncCountData();
private:
  void initiate(const QString&, const QXmlAttributes&) final override;
  void terminate() final override;
  QString m_countType; // type of element being counted
};
//********************************************************************
class GncCommodity : public GncObject
{
public:
  GncCommodity();
  ~GncCommodity();
protected:
  friend class MyMoneyGncReader;
  // access data values
  bool isCurrency() const {
    return (var(SPACE) == QStringLiteral("ISO4217") || var(SPACE) == QStringLiteral("CURRENCY"));
  }
  QString space() const {
    return (var(SPACE));
  }
  QString id() const {
    return (var(ID));
  }
  QString name() const {
    return (var(NAME));
  }
  QString fraction() const {
    return (var(FRACTION));
  }
private:
  void terminate() final override;
  // data elements
  enum {SPACE, ID, NAME, FRACTION, END_Commodity_DELS};
};
// ************* GncPrice********************************************
class GncPrice : public GncObject
{
public:
  GncPrice();
  ~GncPrice();
protected:
  friend class MyMoneyGncReader;
  // access data values
  const GncCmdtySpec *commodity() const {
    return (m_vpCommodity);
  }
  const GncCmdtySpec *currency() const {
    return (m_vpCurrency);
  }
  QString value() const {
    return (var(VALUE));
  }
  QDate priceDate() const {
    return (m_vpPriceDate->date());
  }
private:
  void terminate() final override;
  // sub object elements
  enum PriceSubEls {CMDTY, CURR, PRICEDATE, END_Price_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;
  // data elements
  enum PriceDataEls {VALUE, END_Price_DELS };
  GncCmdtySpec *m_vpCommodity, *m_vpCurrency;
  GncDate *m_vpPriceDate;
};
// ************* GncAccount********************************************
class GncAccount : public GncObject
{
public:
  GncAccount();
  ~GncAccount();
protected:
  friend class MyMoneyGncReader;
  // access data values
  GncCmdtySpec *commodity() const {
    return (m_vpCommodity);
  }
  QString id() const {
    return (var(ID));
  }
  QString name() const {
    return (var(NAME));
  }
  QString desc() const {
    return (var(DESC));
  }
  QString type() const {
    return (var(TYPE));
  }
  QString parent() const {
    return (var(PARENT));
  }
private:
  // subsidiary objects/elements
  enum AccountSubEls {CMDTY, KVP, LOTS, END_Account_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;
  void terminate() final override;
  // data elements
  enum AccountDataEls {ID, NAME, DESC, TYPE, PARENT, END_Account_DELS };
  GncCmdtySpec *m_vpCommodity;
};
// ************* GncSplit********************************************
class GncSplit : public GncObject
{
public:
  GncSplit();
  ~GncSplit();
protected:
  friend class MyMoneyGncReader;
  // access data values
  QString id() const {
    return (var(ID));
  }
  QString memo() const {
    return (var(MEMO));
  }
  QString recon() const {
    return (var(RECON));
  }
  QString value() const {
    return (var(VALUE));
  }
  QString qty() const {
    return (var(QTY));
  }
  QString acct() const {
    return (var(ACCT));
  }
  const QDate reconDate() const {
    QDate x = QDate(); return (m_vpDateReconciled == NULL ? x : m_vpDateReconciled->date());
  }
private:
  // subsidiary objects/elements
  enum TransactionSubEls {RECDATE, END_Split_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;
  // data elements
  enum SplitDataEls {ID, MEMO, RECON, VALUE, QTY, ACCT, END_Split_DELS };
  GncDate *m_vpDateReconciled;
};
// ************* GncTransaction********************************************
class GncTransaction : public GncObject
{
public:
  GncTransaction(bool processingTemplates);
  ~GncTransaction();
protected:
  friend class MyMoneyGncReader;
  // access data values
  QString id() const {
    return (var(ID));
  }
  QString no() const {
    return (var(NO));
  }
  QString desc() const {
    return (var(DESC));
  }
  QString currency() const {
    return (m_vpCurrency == NULL ? QString() : m_vpCurrency->id());
  }
  QDate dateEntered() const {
    return (m_vpDateEntered->date());
  }
  QDate datePosted() const {
    return (m_vpDatePosted->date());
  }
  bool isTemplate() const {
    return (m_template);
  }
  unsigned int splitCount() const {
    return (m_splitList.count());
  }
  const GncObject *getSplit(unsigned int i) const {
    return (m_splitList.at(i));
  }
private:
  // subsidiary objects/elements
  enum TransactionSubEls {CURRCY, POSTED, ENTERED, SPLIT, KVP, END_Transaction_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;
  void terminate() final override;
  const GncKvp getKvp(unsigned int i) const {
    return (m_kvpList.at(i));
  }
  // data elements
  enum TransactionDataEls {ID, NO, DESC, END_Transaction_DELS };
  GncCmdtySpec *m_vpCurrency;
  GncDate *m_vpDateEntered, *m_vpDatePosted;
  mutable QList<GncObject*> m_splitList;
  bool m_template; // true if this is a template for scheduled transaction
};

// ************* GncTemplateSplit********************************************
class GncTemplateSplit : public GncObject
{
public:
  GncTemplateSplit();
  ~GncTemplateSplit();
protected:
  friend class MyMoneyGncReader;
  // access data values
  QString id() const {
    return (var(ID));
  }
  QString memo() const {
    return (var(MEMO));
  }
  QString recon() const {
    return (var(RECON));
  }
  QString value() const {
    return (var(VALUE));
  }
  QString qty() const {
    return (var(QTY));
  }
  QString acct() const {
    return (var(ACCT));
  }
private:
  const GncKvp getKvp(unsigned int i) const {
    return (m_kvpList[i]);
  };
  // subsidiary objects/elements
  enum TemplateSplitSubEls {KVP, END_TemplateSplit_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;
  // data elements
  enum TemplateSplitDataEls {ID, MEMO, RECON, VALUE, QTY, ACCT, END_TemplateSplit_DELS };
};
// ************* GncSchedule********************************************
class GncFreqSpec;
class GncRecurrence;
class GncSchedDef;
class GncSchedule : public GncObject
{
public:
  GncSchedule();
  ~GncSchedule();
protected:
  friend class MyMoneyGncReader;
  // access data values
  QString name() const {
    return (var(NAME));
  }
  QString enabled() const {
    return var(ENABLED);
  }
  QString autoCreate() const {
    return (var(AUTOC));
  }
  QString autoCrNotify() const {
    return (var(AUTOCN));
  }
  QString autoCrDays() const {
    return (var(AUTOCD));
  }
  QString advCrDays() const {
    return (var(ADVCD));
  }
  QString advCrRemindDays() const {
    return (var(ADVRD));
  }
  QString instanceCount() const {
    return (var(INSTC));
  }
  QString numOccurs() const {
    return (var(NUMOCC));
  }
  QString remOccurs() const {
    return (var(REMOCC));
  }
  QString templId() const {
    return (var(TEMPLID));
  }
  QDate startDate() const {
    QDate x = QDate(); return (m_vpStartDate == NULL ? x : m_vpStartDate->date());
  }
  QDate lastDate() const {
    QDate x = QDate(); return (m_vpLastDate == NULL ? x : m_vpLastDate->date());
  }
  QDate endDate() const {
    QDate x = QDate(); return (m_vpEndDate == NULL ? x : m_vpEndDate->date());
  }
  const GncFreqSpec *getFreqSpec() const {
    return (m_vpFreqSpec);
  }
  const GncSchedDef *getSchedDef() const {
    return (m_vpSchedDef);
  }
private:
  // subsidiary objects/elements
  enum ScheduleSubEls {STARTDATE, LASTDATE, ENDDATE, FREQ, RECURRENCE, DEFINST, END_Schedule_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;
  void terminate() final override;
  // data elements
  enum ScheduleDataEls {NAME, ENABLED, AUTOC, AUTOCN, AUTOCD, ADVCD, ADVRD, INSTC,
                        NUMOCC, REMOCC, TEMPLID, END_Schedule_DELS
                       };
  GncDate *m_vpStartDate, *m_vpLastDate, *m_vpEndDate;
  GncFreqSpec *m_vpFreqSpec;
  mutable QList<GncRecurrence*> m_vpRecurrence; // gnc handles multiple occurrences
  GncSchedDef *m_vpSchedDef;
};
// ************* GncFreqSpec********************************************
class GncFreqSpec : public GncObject
{
public:
  GncFreqSpec();
  ~GncFreqSpec();
protected:
  friend class MyMoneyGncReader;
  // access data values (only interval type used at present)
  QString intervalType() const {
    return (var(INTVT));
  }
private:
  // subsidiary objects/elements
  enum FreqSpecSubEls {COMPO, END_FreqSpec_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;
  // data elements
  enum FreqSpecDataEls {INTVT, MONTHLY, DAILY, WEEKLY, INTVI, INTVO, INTVD, END_FreqSpec_DELS};
  void terminate() final override;
  mutable QList<GncObject*> m_fsList;
};

// ************* GncRecurrence********************************************
// this object replaces GncFreqSpec from Gnucash 2.2 onwards
class GncRecurrence : public GncObject
{
public:
  GncRecurrence();
  ~GncRecurrence();
protected:
  friend class MyMoneyGncReader;
  // access data values
  QDate startDate() const {
    QDate x = QDate(); return (m_vpStartDate == NULL ? x : m_vpStartDate->date());
  }
  QString mult() const {
    return (var(MULT));
  }
  QString periodType() const {
    return (var(PERIODTYPE));
  }
  QString getFrequency() const;
private:
  // subsidiary objects/elements
  enum RecurrenceSubEls {STARTDATE, END_Recurrence_SELS };
  GncObject *startSubEl() final override;
  void endSubEl(GncObject *) final override;
  // data elements
  enum RecurrenceDataEls {MULT, PERIODTYPE, END_Recurrence_DELS};
  void terminate() final override;
  GncDate *m_vpStartDate;
};

// ************* GncSchedDef********************************************
// This is a sub-object of GncSchedule, (sx:deferredInstance) function currently unknown
class GncSchedDef : public GncObject
{
public:
  GncSchedDef();
  ~GncSchedDef();
protected:
  friend class MyMoneyGncReader;
private:
  // subsidiary objects/elements
};

// ****************************************************************************************
/**
                                    XML Reader
 The XML reader is an implementation of the Qt SAX2 XML parser. It determines the type
 of object represented by the XMl, and calls the appropriate object functions
*/
// *****************************************************************************************
class XmlReader : public QXmlDefaultHandler
{
protected:
  friend class MyMoneyGncReader;
  XmlReader(MyMoneyGncReader *pM);                 // keep pointer to 'main'
  void processFile(QIODevice*);  // main entry point of reader
  //  define xml content handler functions
  bool startDocument() final override;
  bool startElement(const QString&, const QString&, const QString&, const QXmlAttributes&) final override;
  bool endElement(const QString&, const QString&, const QString&) final override;
  bool characters(const QString &) final override;
  bool endDocument() final override;
private:
  QXmlInputSource *m_source;
  QXmlSimpleReader *m_reader;
  QStack<GncObject*> m_os; // stack of sub objects
  GncObject *m_co;           // current object, for ease of coding (=== m_os.top)
  MyMoneyGncReader *pMain;  // the 'main' pointer, to pass on to objects
  bool m_headerFound; // check for gnc-v2 header
#ifdef _GNCFILEANON
  int lastType; // 0 = start element, 1 = data, 2 = end element
  int indentCount;
#endif // _GNCFILEANON
};

/**
                   MyMoneyGncReader -  Main class for this module
  Controls overall operation of the importer
  */

#ifndef _GNCFILEANON
class MyMoneyGncReader : public IMyMoneyOperationsFormat
{
#else
class MyMoneyGncReader
{
#endif // _GNCFILEANON
public:
  MyMoneyGncReader();
  ~MyMoneyGncReader() override;
  /**
    * Import a GnuCash XML file
    *
    * @param pDevice : pointer to GnuCash file
    * @param storage : pointer to MyMoneySerialize storage
    *
    * @return void
    *
    */
#ifndef _GNCFILEANON
  void readFile(QIODevice* pDevice, MyMoneyStorageMgr *storage) final override;  // main entry point, IODevice is gnucash file
  void writeFile(QIODevice*, MyMoneyStorageMgr*) final override {
    return ;
  } // dummy entry needed by kmymoneywiew. we will not be writing
  void setProgressCallback(void(*callback)(int, int, const QString&)) final override;
#else
  void readFile(QString, QString);
#endif // _GNCFILEANON
  QTextCodec *m_decoder;
protected:
  friend class GncObject; // pity we can't just say GncObject. And compiler doesn't like multiple friends on one line...
  friend class GncFile; // there must be a better way...
  friend class GncDate;
  friend class GncCmdtySpec;
  friend class GncKvp;
  friend class GncLot;
  friend class GncCountData;
  friend class GncCommodity;
  friend class GncPrice;
  friend class GncAccount;
  friend class GncTransaction;
  friend class GncSplit;
  friend class GncTemplateTransaction;
  friend class GncTemplateSplit;
  friend class GncSchedule;
  friend class GncFreqSpec;
  friend class GncRecurrence;
  friend class XmlReader;
#ifndef _GNCFILEANON
  /** functions to convert gnc objects to our equivalent */
  void convertCommodity(const GncCommodity *);
  void convertPrice(const GncPrice *);
  void convertAccount(const GncAccount *);
  void convertTransaction(const GncTransaction *);
  void convertSplit(const GncSplit *);
  void saveTemplateTransaction(GncTransaction *t) {
    m_templateList.append(t);
  };
  void convertSchedule(const GncSchedule *);
  void convertFreqSpec(const GncFreqSpec *);
  void convertRecurrence(const GncRecurrence *);
#else
  /** functions to convert gnc objects to our equivalent */
  void convertCommodity(const GncCommodity *) {
    return;
  };
  void convertPrice(const GncPrice *) {
    return;
  };
  void convertAccount(const GncAccount *) {
    return;
  };
  void convertTransaction(const GncTransaction *) {
    return;
  };
  void convertSplit(const GncSplit *) {
    return;
  };
  void saveTemplateTransaction(GncTransaction *t)  {
    return;
  };
  void convertSchedule(const GncSchedule *) {
    return;
  };
  void convertFreqSpec(const GncFreqSpec *) {
    return;
  };
#endif // _GNCFILEANON
  /** to post messages for final report */
  void postMessage(const QString&, const unsigned int, const char *);
  void postMessage(const QString&, const unsigned int, const char *, const char *);
  void postMessage(const QString&, const unsigned int, const char *, const char *, const char *);
  void postMessage(const QString&, const unsigned int, const QStringList&);
  void signalProgress(int current, int total, const QString& = "");
  /** user options */
  /**
              Scheduled Transactions
    Due to differences in implementation, it is not always possible to import scheduled
    transactions correctly. Though best efforts are made, it may be that some
    imported transactions cause problems within kmymoney.
    An attempt is made within the importer to identify potential problem transactions,
    and setting this option will cause them to be dropped from the file.
    A report of which were dropped, and why, will be produced.
       m_dropSuspectSchedules - drop suspect scheduled transactions
  */
  bool m_dropSuspectSchedules;
  /**
                Investments
    In kmymoney, all accounts representing investments (stocks, shares, bonds, etc.) must
    have an associated investment account (e.g. a broker account). The stock account holds
    the share balance, the investment account a money balance.
    Gnucash does not do this, so we cannot automate this function. If you have investments,
    you must select one of the following options.
       0 - create a separate investment account for each stock with the same name as the stock
       1 - create a single investment account to hold all stocks - you will be asked for a name
       2 - create multiple investment accounts - you will be asked for a name for each stock
       N.B. :- option 2 doesn't really work quite as desired at present
  */
  unsigned int m_investmentOption;
  /**           Online quotes
    The user has the option to use the Finance::Quote system, as used by GnuCash, to
    retrieve online share price quotes
  */
  bool m_useFinanceQuote;
  /**           Tx Notes handling
    Under some usage conditions, non-split GnuCash transactions may contain residual, usually incorrect, memo
    data which is not normally visible to the user. When imported into KMyMoney however, due to display
    differences, this data can become visible. Often, these transactions will have a Notes field describing
    the real purpose of the transaction. If this option is selected, these notes, if present, will be used to
    override the extraneous memo data."  */
  bool m_useTxNotes;
  // set gnucash counts (not always accurate!)
  void setGncCommodityCount(int i) {
    m_gncCommodityCount = i;
  }
  void setGncAccountCount(int i) {
    m_gncAccountCount = i;
  }
  void setGncTransactionCount(int i) {
    m_gncTransactionCount = i;
  }
  void setGncScheduleCount(int i) {
    m_gncScheduleCount = i;
  }
  void setSmallBusinessFound(bool b) {
    m_smallBusinessFound = b;
  }
  void setBudgetsFound(bool b) {
    m_budgetsFound = b;
  }
  void setLotsFound(bool b) {
    m_lotsFound = b;
  }
  /*          Debug Options
    If you don't know what these are, best leave them alone.
       gncdebug - produce general debug messages
       xmldebug - produce a trace of the gnucash file XML
       bAnonymize - hide personal data (account names, payees, etc., randomize money amounts)
  */
  bool gncdebug; // general debug messages
  bool xmldebug; // xml trace
  bool bAnonymize; // anonymize input
  static double m_fileHideFactor; // an overall anonymization factor to be applied to all items
  bool developerDebug;
private:
  void setOptions();  // to set user options from dialog
  void setFileHideFactor();
  // the following handles the gnucash indicator for a bad value (-1/0) which causes us probs
  QString convBadValue(QString gncValue) const {
    return (gncValue == "-1/0" ? "0/1" : gncValue);
  }
#ifndef _GNCFILEANON
  MyMoneyTransaction convertTemplateTransaction(const QString&, const GncTransaction *);
  void convertTemplateSplit(const QString&, const GncTemplateSplit *);
#endif // _GNCFILEANON
  // wind up when all done
  void terminate();
  QString buildReportSection(const QString&);
  bool writeReportToFile(const QList<QString>&);
  // main storage
#ifndef _GNCFILEANON
  MyMoneyStorageMgr *m_storage;
#else
  QTextStream oStream;
#endif // _GNCFILEANON
  XmlReader *m_xr;
  /** to hold the callback pointer for the progress bar */
  void (*m_progressCallback)(int, int, const QString&);
  // a map of which versions of the various elements (objects) we can import
  map_elementVersions m_versionList;
  // counters holding count data from the Gnc 'count-data' section
  int m_gncCommodityCount;
  int m_gncAccountCount;
  int m_gncTransactionCount;
  int m_gncScheduleCount;

  // flags indicating detection of features not (yet?) supported
  bool m_smallBusinessFound;
  bool m_budgetsFound;
  bool m_lotsFound;

  /** counters for reporting */
  int m_commodityCount;
  int m_priceCount;
  int m_accountCount;
  int m_transactionCount;
  int m_templateCount;
  int m_scheduleCount;
#ifndef _GNCFILEANON
  // counters for error reporting
  int m_ccCount, m_orCount, m_scCount;
  // currency counter
  QMap<QString, unsigned int> m_currencyCount;
  /**
    * Map gnucash vs. Kmm ids for accounts, equities, schedules, price sources
    */
  QMap<QString, QString> m_mapIds;
  QString m_rootId; // save the root id for terminate()
  QMap<QString, QString> m_mapEquities;
  QMap<QString, QString> m_mapSchedules;
  QMap<QString, QString> m_mapSources;
  /**
    * A list of stock accounts (gnc ids) which will be held till the end
      so we can implement the user's investment option
    */
  QList<QString> m_stockList;
  /**
    * Temporary storage areas for transaction processing
    */
  QString m_txCommodity; // save commodity for current transaction
  QString m_txPayeeId;    // gnc has payee at tx level, we need it at split level
  QDate m_txDatePosted;   // ditto for post date
  QString m_txChequeNo;    // ditto for cheque number
  /** In kmm, the order of splits is critical to some operations. These
    * areas will hold the splits until we've read them all */
  QList<MyMoneySplit> m_splitList, m_liabilitySplitList, m_otherSplitList;
  bool m_potentialTransfer;       // to determine whether this might be a transfer
  /** Schedules are processed through 3 different functions, any of which may set this flag */
  bool m_suspectSchedule;
  /**
  * A holding area for template txs while we're waiting for the schedules
  */
  QList<GncTransaction*> m_templateList;
  /** Hold a list of suspect schedule ids for later processing? */
  QList<QString> m_suspectList;
  /**
    * To hold message data till final report
    */
  QMap<QString, QStringList> m_messageList;
  /**
    * Internal utility functions
    */
  QString createPayee(const QString&);  // create a payee and return it's id
  QString createOrphanAccount(const QString&);  // create unknown account and return the id
  QDate incrDate(QDate lastDate, unsigned char interval, unsigned int intervalCount);  // for date calculations
  MyMoneyAccount checkConsistency(MyMoneyAccount& parent, MyMoneyAccount& child);  // gnucash is sometimes TOO flexible
  void checkInvestmentOption(QString stockId);  // implement user investment option
  void getPriceSource(MyMoneySecurity stock, QString gncSource);
#endif // _GNCFILEANON
};

#endif // MYMONEYGNCREADER_H
