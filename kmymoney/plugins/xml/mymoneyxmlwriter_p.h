/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYXMLWRITER_P_H
#define MYMONEYXMLWRITER_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
class QXmlStreamWriter;

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyFile;

class MyMoneyXmlWriterPrivate
{
public:
    typedef struct {
        QString street;
        QString city;
        QString state;
        QString zipcode;
        QString telephone;
    } Address;

    QXmlStreamWriter* m_writer;
    MyMoneyFile* m_file = nullptr;

    MyMoneyXmlWriterPrivate();

    virtual ~MyMoneyXmlWriterPrivate();

    inline static QString attrValue(bool attribute)
    {
        return attribute ? QStringLiteral("1") : QStringLiteral("0");
    }

    inline static QString attrValue(int attribute)
    {
        return QString::number(attribute);
    }

    inline static QString attrValue(uint attribute)
    {
        return QString::number(attribute);
    }

    static void writeKeyValueContainer(QXmlStreamWriter* writer, const QMap<QString, QString>& pairs);
    static void writeKeyValueContainer(QXmlStreamWriter* writer, const MyMoneyKeyValueContainer& kvp);
    static void
    writeAddress(QXmlStreamWriter* writer, const QString& street, const QString& city, const QString& state, const QString& zip, const QString& phone);
    void writeAddress(const QString& street, const QString& city, const QString& state, const QString& zip, const QString& phone);

    static void writeStartObject(QXmlStreamWriter* writer, const QString tagName, const MyMoneyObject& object);
    static void writeStartObject(QXmlStreamWriter* writer, const QString tagName, const QString& id);
    static void writePayeeIdentifier(QXmlStreamWriter* writer, const payeeIdentifier& obj);
    static void writeInstitution(const MyMoneyInstitution& institution, QXmlStreamWriter* writer);
    static void writePayee(const MyMoneyPayee& payee, QXmlStreamWriter* writer);
    static void writeCostCenter(const MyMoneyCostCenter& costCenter, QXmlStreamWriter* writer);
    static void writeTag(const MyMoneyTag& tag, QXmlStreamWriter* writer);
    virtual void writeAccount(const MyMoneyAccount& account);
    virtual void writeSplit(QXmlStreamWriter* writer, const MyMoneySplit& _split);
    virtual void writeTransaction(QXmlStreamWriter* writer, const MyMoneyTransaction& transaction);
    virtual void writeSchedule(QXmlStreamWriter* writer, const MyMoneySchedule& schedule);
    static void writeSecurity(const MyMoneySecurity& security, QXmlStreamWriter* writer);
    static void writeReport(const MyMoneyReport& report, QXmlStreamWriter* writer);
    static void writeBudget(const MyMoneyBudget& budget, QXmlStreamWriter* writer);
    static void writeOnlineJob(const onlineJob& job, QXmlStreamWriter* writer);

    virtual void writeFileInformation();
    virtual void writeUserInformation();
    virtual void writeInstitutions();
    virtual void writePayees();
    virtual void writeCostCenters();
    virtual void writeTags();
    virtual void writeAccounts();
    virtual void writeTransactions();
    virtual void writeSchedules();
    virtual void writeSecurities();
    void writeCurrencies();
    void writePrices();
    virtual void writeReports();
    virtual void writeBudgets();
    virtual void writeOnlineJobs();

    void writeKMyMoney();
    bool write(QIODevice* device);

    // used primarily for the anonymization
    QList<MyMoneyAccount> m_accountList;
};

#endif // MYMONEYXMLWRITER_P_H
