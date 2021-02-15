/*
 * SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2002-2016 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYMONEYSTORAGEXML_H
#define MYMONEYSTORAGEXML_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>
#include <QString>

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneystorageformat.h"

/**
  *@author Kevin Tambascio (ktambascio@users.sourceforge.net)
  */

#define VERSION_0_60_XML  0x10000010    // Version 0.5 file version info
#define VERSION_0_61_XML  0x10000011    // use 8 bytes for MyMoneyMoney objects

class QString;
class QIODevice;
class QDomElement;
class QDomDocument;
class QDate;

class MyMoneyStorageMgr;
class MyMoneyInstitution;
class MyMoneyAccount;
class MyMoneySecurity;
class MyMoneySchedule;
class MyMoneyPayee;
class MyMoneyTag;
class MyMoneyBudget;
class MyMoneyReport;
class MyMoneyPrice;
class MyMoneyTransaction;
class MyMoneyCostCenter;
class onlineJob;

template <typename T> class QList;
typedef QPair<QString, QString> MyMoneySecurityPair;
typedef QMap<QDate, MyMoneyPrice> MyMoneyPriceEntries;
typedef QMap<MyMoneySecurityPair, MyMoneyPriceEntries> MyMoneyPriceList;

class MyMoneyStorageXML : public IMyMoneyOperationsFormat
{
  friend class MyMoneyXmlContentHandler;
public:
  MyMoneyStorageXML();
  virtual ~MyMoneyStorageXML();

  enum fileVersionDirectionType {
    Reading = 0,          /**< version of file to be read */
    Writing = 1           /**< version to be used when writing a file */
  };

  void readFile(QIODevice* s, MyMoneyStorageMgr* storage) override;
  void writeFile(QIODevice* s, MyMoneyStorageMgr* storage) override;
  void setProgressCallback(void(*callback)(int, int, const QString&)) override;

  protected:
  void          signalProgress(int current, int total, const QString& = "");

  /**
    * This method returns the version of the underlying file. It
    * is used by the MyMoney objects contained in a MyMoneyStorageBin object (e.g.
    * MyMoneyAccount, MyMoneyInstitution, MyMoneyTransaction, etc.) to
    * determine the layout used when reading/writing a persistent file.
    * A parameter is used to determine the direction.
    *
    * @param dir information about the direction (reading/writing). The
    *            default is reading.
    *
    * @return version QString of file's version
    *
    * @see m_fileVersionRead, m_fileVersionWrite
    */
  static unsigned int fileVersion(fileVersionDirectionType dir = Reading);

  QList<QDomElement> readElements(QString groupTag, QString itemTag = QString());

  bool readFileInformation(const QDomElement& fileInfo);
  virtual void writeFileInformation(QDomElement& fileInfo);

  virtual void writeUserInformation(QDomElement& userInfo);

  virtual void writeInstitution(QDomElement& institutions, const MyMoneyInstitution& i);
  virtual void writeInstitutions(QDomElement& institutions);

  virtual void writePrices(QDomElement& prices);
  virtual void writePricePair(QDomElement& price, const MyMoneyPriceEntries& p);
  virtual void writePrice(QDomElement& prices, const MyMoneyPrice& p);

  virtual void writePayees(QDomElement& payees);
  virtual void writePayee(QDomElement& payees, const MyMoneyPayee& p);

  virtual void writeTags(QDomElement& tags);
  virtual void writeTag(QDomElement& tags, const MyMoneyTag& ta);

  virtual void writeAccounts(QDomElement& accounts);
  virtual void writeAccount(QDomElement& accounts, const MyMoneyAccount& p);

  virtual void writeTransactions(QDomElement& transactions);
  virtual void writeTransaction(QDomElement& transactions, const MyMoneyTransaction& tx);

  virtual void writeSchedules(QDomElement& scheduled);
  virtual void writeSchedule(QDomElement& scheduledTx, const MyMoneySchedule& tx);

  virtual void writeReports(QDomElement& e);
  virtual void writeReport(QDomElement& report, const MyMoneyReport& r);
  virtual void writeBudgets(QDomElement& e);
  virtual void writeBudget(QDomElement& budget, const MyMoneyBudget& b);

  virtual void writeOnlineJobs(QDomElement& onlineJobs);
  virtual void writeOnlineJob(QDomElement& onlineJobs, const onlineJob& job);

  virtual void writeSecurities(QDomElement& securities);
  virtual void writeSecurity(QDomElement& securityElement, const MyMoneySecurity& security);

  virtual void writeCostCenters(QDomElement& parent);
  virtual void writeCostCenter(QDomElement& costCenters, const MyMoneyCostCenter& costCenter);

  virtual void writeCurrencies(QDomElement& currencies);

  virtual QDomElement writeKeyValuePairs(const QMap<QString, QString> pairs);

  bool readUserInformation(const QDomElement& userElement);

  void readPricePair(const QDomElement& pricePair);
  const MyMoneyPrice readPrice(const QString& from, const QString& to, const QDomElement& price);

  QDomElement findChildElement(const QString& name, const QDomElement& root);

private:
  void (*m_progressCallback)(int, int, const QString&);

protected:
  MyMoneyStorageMgr *m_storage;
  QDomDocument *m_doc;

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;

  /**
    * This member is used to store the file version information
    * obtained while reading a file.
    */
  static unsigned int fileVersionRead;

  /**
    * This member is used to store the file version information
    * to be used when writing a file.
    */
  static unsigned int fileVersionWrite;
  /**
    * This member keeps the id of the base currency. We need this
    * temporarily to convert the price history from the old to the
    * new format. This should go at some time beyond 0.8 (ipwizard)
    */
  QString m_baseCurrencyId;

};

#endif
