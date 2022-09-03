/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyxmlreader-test.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTest>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#define KMM_MYMONEY_UNIT_TESTABLE friend class MyMoneyXmlReaderTest;

#include "../mymoneyxmlreader.h"
#include "budgetsmodel.h"
#include "journalmodel.h"
#include "mymoneyaccount.h"
#include "mymoneycostcenter.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "onlinejobsmodel.h"
#include "onlinejobtyped.h"
#include "parametersmodel.h"
#include "payeesmodel.h"
#include "pricemodel.h"
#include "schedulesmodel.h"
#include "securitiesmodel.h"

#include "ibanbic/ibanbic.h"
#include "payeeidentifier/payeeidentifiertyped.h"
#include "tasks/credittransfer.h"
#include "tasks/onlinetask.h"

QTEST_GUILESS_MAIN(MyMoneyXmlReaderTest)

static QtMessageHandler oldHandler(nullptr);
static void hideDebugMessages(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if (type != QtDebugMsg && oldHandler) {
        oldHandler(type, context, msg);
    }
}

void MyMoneyXmlReaderTest::initTestCase()
{
    oldHandler = qInstallMessageHandler(hideDebugMessages);
}

void MyMoneyXmlReaderTest::cleanupTestCase()
{
    qInstallMessageHandler(oldHandler);
}

void MyMoneyXmlReaderTest::init()
{
    m_file = MyMoneyFile::instance();
    m_file->unload();

    r = new MyMoneyXmlReader();
    r->setFile(m_file);
}

void MyMoneyXmlReaderTest::cleanup()
{
    m_file->unload();
    delete r;
}

void MyMoneyXmlReaderTest::resetTest()
{
    cleanup();
    init();
}

QString MyMoneyXmlReaderTest::createFile(const QString& data) const
{
    return QStringLiteral("<KMYMONEY-FILE>%1</KMYMONEY-FILE>").arg(data);
}

QString MyMoneyXmlReaderTest::createInstitutionData() const
{
    return QLatin1String(
        "<INSTITUTIONS count=\"1\">"
        "<INSTITUTION id=\"I000001\" manager=\"manager\" name=\"Institution name\" sortcode=\"sortcode\">"
        "<ADDRESS city=\"Frankfurt\" street=\"Street\" telephone=\"Phone\" zip=\"Test1&#10;Test2\"/>"
        "<ACCOUNTIDS>"
        "<ACCOUNTID id=\"A000076\"/>"
        "</ACCOUNTIDS>"
        "<KEYVALUEPAIRS>"
        "<PAIR key=\"bic\" value=\"GENODEF1S12\"/>"
        "<PAIR key=\"icon\" value=\"enum:Bank\"/>"
        "<PAIR key=\"url\" value=\"www.mybank.com\"/>"
        "</KEYVALUEPAIRS>"
        "</INSTITUTION>"
        "</INSTITUTIONS>");
}

QString MyMoneyXmlReaderTest::createAccountsData() const
{
    QString data = QLatin1String("<ACCOUNTS>");
    data.append(createAccountData());
    data.append(QLatin1String("</ACCOUNTS>"));
    return data;
}

QString MyMoneyXmlReaderTest::createAccountsAndCategoryData() const
{
    QString data = QLatin1String("<ACCOUNTS>");
    data.append(createAccountData());
    data.append(createCategoryData());
    data.append(QLatin1String("</ACCOUNTS>"));
    return data;
}

QString MyMoneyXmlReaderTest::createAccountData() const
{
    return QLatin1String(
        "<ACCOUNT currency=\"EUR\" description=\"\" id=\"AStd::Asset\" institution=\"\" lastmodified=\"\" lastreconciled=\"\" name=\"Asset\" number=\"\" "
        "opened=\"\" parentaccount=\"\" type=\"9\">"
        "<SUBACCOUNTS>"
        "<SUBACCOUNT id=\"A000076\"/>"
        "<SUBACCOUNT id=\"A000077\"/>"
        "</SUBACCOUNTS>"
        "</ACCOUNT>"

        // first account uses new method of reconciliationHistory
        "<ACCOUNT currency=\"EUR\" description=\"Account Desc 1\" id=\"A000076\" institution=\"I000001\" lastmodified=\"2022-06-03\" "
        "lastreconciled=\"2022-05-18\" name=\"Account Name 1\" number=\"Number 1\" opened=\"2001-12-28\" parentaccount=\"AStd::Asset\" type=\"1\">"
        "<ONLINEBANKING kbanking-acc-ref=\"Bank-Ref\" kbanking-statementDate=\"2\" provider=\"kbanking\"/>"
        "<RECONCILIATIONS>"
        "<RECONCILIATION date=\"2021-01-31\" value=\"1/5\"/>"
        "<RECONCILIATION date=\"2021-04-28\" value=\"2/50\"/>"
        "</RECONCILIATIONS>"
        "<KEYVALUEPAIRS>"
        "<PAIR key=\"PreferredAccount\" value=\"Yes\"/>"
        "<PAIR key=\"iban\" value=\"IBAN\"/>"
        "</KEYVALUEPAIRS>"
        "</ACCOUNT>"

        // first account uses old method of reconciliationHistory
        "<ACCOUNT currency=\"EUR\" description=\"Account Desc 2\" id=\"A000077\" institution=\"\" lastmodified=\"2022-06-04\" lastreconciled=\"2022-05-05\" "
        "name=\"Account Name 2\" number=\"Number 2\" opened=\"2002-01-01\" parentaccount=\"AStd::Asset\" type=\"1\">"
        "<KEYVALUEPAIRS>"
        "<PAIR key=\"reconciliationHistory\" "
        "value=\"2014-10-02:1/5;2015-01-05:2/50;2015-04-02:3/50;2015-07-03:4/50;2015-10-05:5/25;2016-01-05:6/100;2016-04-05:7/100;2022-05-05:8/5\"/>"
        "</KEYVALUEPAIRS>"
        "</ACCOUNT>");
}

QString MyMoneyXmlReaderTest::createCategoryData() const
{
    return QLatin1String(
        "<ACCOUNT currency=\"EUR\" description=\"\" id=\"AStd::Expense\" institution=\"\" lastmodified=\"\" lastreconciled=\"\" name=\"Asset\" number=\"\" "
        "opened=\"\" parentaccount=\"\" type=\"13\">"
        "<SUBACCOUNTS>"
        "<SUBACCOUNT id=\"A000128\"/>"
        "<SUBACCOUNT id=\"A000136\"/>"
        "</SUBACCOUNTS>"
        "</ACCOUNT>"

        "<ACCOUNT currency=\"EUR\" description=\"\" id=\"A000128\" institution=\"\" lastmodified=\"2022-04-06\" lastreconciled=\"\" name=\"Goods\" number=\"\" "
        "opened=\"1900-01-01\" parentaccount=\"AStd::Expense\" type=\"13\">"
        "<KEYVALUEPAIRS>"
        "<PAIR key=\"VatAccount\" value=\"A000136\"/>"
        "</KEYVALUEPAIRS>"
        "</ACCOUNT>"

        "<ACCOUNT currency=\"EUR\" description=\"\" id=\"A000136\" institution=\"\" lastmodified=\"2022-05-20\" lastreconciled=\"\" name=\"Sales tax 19%\" "
        "number=\"\" opened=\"1900-01-01\" parentaccount=\"AStd::Expense\" type=\"13\">"
        "<KEYVALUEPAIRS>"
        "<PAIR key=\"Tax\" value=\"Yes\"/>"
        "<PAIR key=\"VatRate\" value=\"1900/10000\"/>"
        "</KEYVALUEPAIRS>"
        "</ACCOUNT>");
}

void MyMoneyXmlReaderTest::testReadFileInfo()
{
    const auto data = createFile(
        "<FILEINFO>"
        "<CREATION_DATE date=\"2003-09-29\"/>"
        "<LAST_MODIFIED_DATE date=\"2022-01-05\"/>"
        "<VERSION id=\"1\"/>"
        "<FIXVERSION id=\"5\"/>"
        "</FILEINFO>");

    QCOMPARE(r->read(data), true);
}

void MyMoneyXmlReaderTest::testReadUser()
{
    auto data = createFile(
        "<USER email=\"EMail@test.com\" name=\"My name\">"
        "<ADDRESS city=\"city\" country=\"country\" county=\"county\" postcode=\"postcode\" state=\"state\" street=\"street\" telephone=\"phone\" zip=\"zip\" "
        "zipcode=\"zipcode\"/>"
        "</USER>");

    QCOMPARE(r->read(data), true);

    auto user = m_file->user();

    QCOMPARE(user.name(), QLatin1String("My name"));
    QCOMPARE(user.email(), QLatin1String("EMail@test.com"));
    QCOMPARE(user.city(), QLatin1String("city"));
    QCOMPARE(user.state(), QLatin1String("state"));
    QCOMPARE(user.postcode(), QLatin1String("zip"));
    QCOMPARE(user.address(), QLatin1String("street"));
    QCOMPARE(user.telephone(), QLatin1String("phone"));

    resetTest();

    data = createFile(
        "<USER email=\"EMail@test.com\" name=\"My name\">"
        "<ADDRESS city=\"city\" country=\"country\" county=\"county\" postcode=\"postcode\" street=\"street\" telephone=\"phone\" zipcode=\"zipcode\"/>"
        "</USER>");

    QCOMPARE(r->read(data), true);

    user = m_file->user();

    QCOMPARE(user.name(), QLatin1String("My name"));
    QCOMPARE(user.email(), QLatin1String("EMail@test.com"));
    QCOMPARE(user.city(), QLatin1String("city"));
    QCOMPARE(user.state(), QLatin1String("country"));
    QCOMPARE(user.postcode(), QLatin1String("zipcode"));
    QCOMPARE(user.address(), QLatin1String("street"));
    QCOMPARE(user.telephone(), QLatin1String("phone"));

    resetTest();

    data = createFile(
        "<USER email=\"EMail@test.com\" name=\"My name\">"
        "<ADDRESS city=\"city\" county=\"county\" postcode=\"postcode\" street=\"street\" telephone=\"phone\"/>"
        "</USER>");

    QCOMPARE(r->read(data), true);

    user = m_file->user();

    QCOMPARE(user.name(), QLatin1String("My name"));
    QCOMPARE(user.email(), QLatin1String("EMail@test.com"));
    QCOMPARE(user.city(), QLatin1String("city"));
    QCOMPARE(user.state(), QLatin1String("county"));
    QCOMPARE(user.postcode(), QLatin1String("postcode"));
    QCOMPARE(user.address(), QLatin1String("street"));
    QCOMPARE(user.telephone(), QLatin1String("phone"));
}

void MyMoneyXmlReaderTest::testMissingId()
{
    const auto data = createFile(
        "<PAYEES>"
        "<PAYEE email=\"EMail@test.com\" matchignorecase=\"1\" matchingenabled=\"1\" matchkey=\"matchkey1;matchkey2\" name=\"Payee name\" "
        "reference=\"\" usingmatchkey=\"1\">"
        "</PAYEE>"
        "</PAYEES>");

    QCOMPARE(r->read(data), false);
    QCOMPARE(r->errorString(), QLatin1String("Missing attribute id in line 1"));
}

void MyMoneyXmlReaderTest::testEmptyId()
{
    const auto data = createFile(
        "<PAYEES>\n"
        "<PAYEE email=\"EMail@test.com\" id=\"\" matchignorecase=\"1\" matchingenabled=\"1\" matchkey=\"matchkey1;matchkey2\" name=\"Payee name\" "
        "reference=\"\" usingmatchkey=\"1\">"
        "</PAYEE>"
        "</PAYEES>");

    QCOMPARE(r->read(data), false);
    QCOMPARE(r->errorString(), QLatin1String("Empty attribute id in line 2"));
}

void MyMoneyXmlReaderTest::testReadInstitutions()
{
    const auto data = createInstitutionData();

    QCOMPARE(r->read(createFile(data)), true);

    const auto institution = m_file->institution(QLatin1String("I000001"));

    QCOMPARE(m_file->institutionCount(), 1);
    QCOMPARE(institution.id(), QLatin1String("I000001"));
    QCOMPARE(institution.manager(), QLatin1String("manager"));
    QCOMPARE(institution.name(), QLatin1String("Institution name"));
    QCOMPARE(institution.postcode(), QLatin1String("Test1\nTest2"));
    QCOMPARE(institution.street(), QLatin1String("Street"));
    QCOMPARE(institution.telephone(), QLatin1String("Phone"));
    QCOMPARE(institution.town(), QLatin1String("Frankfurt"));
    QCOMPARE(institution.bankcode(), QLatin1String("sortcode"));
    QCOMPARE(institution.accountCount(), 1);
    QCOMPARE(institution.accountList(), QStringList({"A000076"}));
    QCOMPARE(institution.value(QLatin1String("bic")), QLatin1String("GENODEF1S12"));
    QCOMPARE(institution.value(QLatin1String("icon")), QLatin1String("enum:Bank"));
    QCOMPARE(institution.value(QLatin1String("url")), QLatin1String("www.mybank.com"));
}

void MyMoneyXmlReaderTest::testReadPayees()
{
    const auto data = createFile(
        "<PAYEES>"
        "<PAYEE email=\"EMail@test.com\" id=\"P000002\" matchignorecase=\"1\" matchingenabled=\"1\" matchkey=\"matchkey1;matchkey2\" name=\"Payee name\" "
        "reference=\"\" usingmatchkey=\"1\">"
        "<ADDRESS city=\"Frankfurt\" street=\"Street\" telephone=\"Phone\" zip=\"Test1&#10;Test2\"/>"
        "<payeeIdentifier bic=\"BIC\" iban=\"IBAN\" id=\"1\" type=\"org.kmymoney.payeeIdentifier.ibanbic\"/>"
        "</PAYEE>"
        "</PAYEES>");

    QCOMPARE(r->read(data), true);

    const auto payee = m_file->payee(QLatin1String("P000002"));

    QCOMPARE(m_file->payeesModel()->rowCount(), 1);
    QCOMPARE(payee.id(), QLatin1String("P000002"));
    QCOMPARE(payee.name(), QLatin1String("Payee name"));
    QCOMPARE(payee.postcode(), QLatin1String("Test1\nTest2"));
    QCOMPARE(payee.address(), QLatin1String("Street"));
    QCOMPARE(payee.telephone(), QLatin1String("Phone"));
    QCOMPARE(payee.isMatchingEnabled(), true);
    bool ignoreCase(false);
    QStringList matchKeys;
    QCOMPARE(static_cast<int>(payee.matchData(ignoreCase, matchKeys)), static_cast<int>(eMyMoney::Payee::MatchType::Key));
    QCOMPARE(ignoreCase, true);
    QCOMPARE(matchKeys, QStringList({"matchkey1", "matchkey2"}));
    QCOMPARE(payee.payeeIdentifierCount(), 1);
    const auto payeeIdentifier = payee.payeeIdentifier(0);
    QCOMPARE(payeeIdentifier.iid(), QLatin1String("org.kmymoney.payeeIdentifier.ibanbic"));
}

void MyMoneyXmlReaderTest::testReadCostCenters()
{
    const auto data = createFile(
        "<COSTCENTERS count=\"2\">"
        "<COSTCENTER id=\"C000001\" name=\"CC1\"/>"
        "<COSTCENTER id=\"C000002\" name=\"CC2\"/>"
        "</COSTCENTERS>");

    QCOMPARE(r->read(data), true);

    QList<MyMoneyCostCenter> ccList;
    m_file->costCenterList(ccList);
    QCOMPARE(ccList.count(), 2);
    QCOMPARE(ccList.at(0).name(), QLatin1String("CC1"));
    QCOMPARE(ccList.at(1).name(), QLatin1String("CC2"));
}

void MyMoneyXmlReaderTest::testReadTags()
{
    const auto data = createFile(
        "<TAGS count=\"2\">"
        "<TAG closed=\"0\" id=\"G000003\" name=\"Tag 1\" tagcolor=\"#000000\"/>"
        "<TAG closed=\"1\" id=\"G000004\" name=\"Tag 2\" tagcolor=\"#0000ff\"/>"
        "</TAGS>");

    QCOMPARE(r->read(data), true);

    auto tag = m_file->tag(QLatin1String("G000003"));
    QCOMPARE(tag.id(), QLatin1String("G000003"));
    QCOMPARE(tag.name(), QLatin1String("Tag 1"));
    QCOMPARE(tag.tagColor(), QColor("black"));
    QCOMPARE(tag.isClosed(), false);

    tag = m_file->tag(QLatin1String("G000004"));
    QCOMPARE(tag.id(), QLatin1String("G000004"));
    QCOMPARE(tag.name(), QLatin1String("Tag 2"));
    QCOMPARE(tag.tagColor(), QColor("blue"));
    QCOMPARE(tag.isClosed(), true);
}

void MyMoneyXmlReaderTest::testReadAccounts()
{
    auto data = createInstitutionData();
    data.append(createAccountsData());

    QCOMPARE(r->read(createFile(data)), true);

    auto acc1 = m_file->account(QLatin1String("A000076"));
    QCOMPARE(acc1.name(), QLatin1String("Account Name 1"));
    QCOMPARE(acc1.parentAccountId(), QLatin1String("AStd::Asset"));
    QCOMPARE(acc1.institutionId(), QLatin1String("I000001"));
    QCOMPARE(acc1.description(), QLatin1String("Account Desc 1"));
    QCOMPARE(acc1.openingDate(), QDate(2001, 12, 28));
    QCOMPARE(acc1.lastModified(), QDate(2022, 6, 3));
    QCOMPARE(acc1.lastReconciliationDate(), QDate(2022, 5, 18));
    QCOMPARE(acc1.number(), QLatin1String("Number 1"));
    QCOMPARE(acc1.accountType(), eMyMoney::Account::Type::Checkings);
    QCOMPARE(acc1.onlineBankingSettings().value(QLatin1String("kbanking-acc-ref")), QLatin1String("Bank-Ref"));
    QCOMPARE(acc1.onlineBankingSettings().value(QLatin1String("kbanking-statementDate")), QLatin1String("2"));
    QCOMPARE(acc1.onlineBankingSettings().value(QLatin1String("provider")), QLatin1String("kbanking"));
    auto history = acc1.reconciliationHistory();
    QCOMPARE(history.count(), 2);
    QCOMPARE(history.value(QDate(2021, 1, 31)).toDouble(), MyMoneyMoney(1, 5).toDouble());
    QCOMPARE(history.value(QDate(2021, 4, 28)).toDouble(), MyMoneyMoney(2, 50).toDouble());

    auto acc2 = m_file->account(QLatin1String("A000077"));
    QCOMPARE(acc2.name(), QLatin1String("Account Name 2"));
    QCOMPARE(acc2.parentAccountId(), QLatin1String("AStd::Asset"));
    QCOMPARE(acc2.institutionId(), QString());
    QCOMPARE(acc2.description(), QLatin1String("Account Desc 2"));
    QCOMPARE(acc2.openingDate(), QDate(2002, 1, 1));
    QCOMPARE(acc2.lastModified(), QDate(2022, 6, 4));
    QCOMPARE(acc2.lastReconciliationDate(), QDate(2022, 5, 5));
    QCOMPARE(acc2.number(), QLatin1String("Number 2"));
    QCOMPARE(acc2.accountType(), eMyMoney::Account::Type::Checkings);
    history = acc2.reconciliationHistory();
    QCOMPARE(history.count(), 8);
    QCOMPARE(history.value(QDate(2014, 10, 2)).toDouble(), MyMoneyMoney(1, 5).toDouble());
    QCOMPARE(history.value(QDate(2022, 5, 5)).toDouble(), MyMoneyMoney(8, 5).toDouble());
}

void MyMoneyXmlReaderTest::testReadTransactions()
{
    auto data = createInstitutionData();
    data.append(createAccountsAndCategoryData());

    data.append(QLatin1String(
        "<TRANSACTIONS>"
        "<TRANSACTION commodity=\"EUR\" entrydate=\"2020-07-30\" id=\"T000000000000008787\" memo=\"\" postdate=\"2020-07-29\">"
        "<SPLITS>"
        "<SPLIT account=\"A000076\" action=\"Deposit\" bankid=\"A000076-2020-07-28-fa28c20-1\" id=\"S0001\" memo=\"DA Miete&#10;EREF: 000000009802\" "
        "number=\"001\" payee=\"P000118\" price=\"1/1\" reconciledate=\"2020-07-31\" reconcileflag=\"2\" shares=\"3/1\" value=\"3/1\">"
        "<KEYVALUEPAIRS>"
        "<PAIR key=\"kmm-match-split\" value=\"S0003\"/>"
        "<PAIR key=\"kmm-matched-tx\" value=\"&amp;#60;!DOCTYPE MATCH&gt;&#10;&amp;#60;CONTAINER&gt;&#10; &amp;#60;TRANSACTION commodity=&quot;EUR&quot; "
        "memo=&quot;DA Miete&amp;#xa;EREF: 000000009802&quot; entrydate=&quot;2020-07-28&quot; postdate=&quot;2020-07-28&quot; id=&quot;&quot;&gt;&#10;  "
        "&amp;#60;SPLITS&gt;&#10;   &amp;#60;SPLIT payee=&quot;&quot; price=&quot;1/1&quot; bankid=&quot;&quot; memo=&quot;&quot; reconciledate=&quot;&quot; "
        "reconcileflag=&quot;0&quot; value=&quot;-2/1&quot; action=&quot;Deposit&quot; account=&quot;A000002&quot; shares=&quot;-2/1&quot; "
        "id=&quot;S0001&quot; number=&quot;&quot;/&gt;&#10;   &amp;#60;SPLIT payee=&quot;&quot; price=&quot;1/1&quot; bankid=&quot;&quot; memo=&quot;&quot; "
        "reconciledate=&quot;&quot; reconcileflag=&quot;0&quot; value=&quot;-1/1&quot; action=&quot;Deposit&quot; account=&quot;A000003&quot; "
        "shares=&quot;-1/1&quot; id=&quot;S0002&quot; number=&quot;&quot;/&gt;&#10;   &amp;#60;SPLIT payee=&quot;P000118&quot; price=&quot;1/1&quot; "
        "bankid=&quot;A000076-2020-07-28-fa28c20-1&quot; memo=&quot;DA Miete&amp;#xa;EREF: 000000009802&quot; reconciledate=&quot;&quot; "
        "reconcileflag=&quot;0&quot; value=&quot;3/1&quot; action=&quot;&quot; account=&quot;A000076&quot; shares=&quot;3/1&quot; id=&quot;S0003&quot; "
        "number=&quot;&quot;/&gt;&#10;  &amp;#60;/SPLITS&gt;&#10;  &amp;#60;KEYVALUEPAIRS&gt;&#10;   &amp;#60;PAIR value=&quot;true&quot; "
        "key=&quot;Imported&quot;/&gt;&#10;  &amp;#60;/KEYVALUEPAIRS&gt;&#10; &amp;#60;/TRANSACTION&gt;&#10;&amp;#60;/CONTAINER&gt;&#10;\"/>"
        "<PAIR key=\"kmm-orig-memo\" value=\"omemo\"/>"
        "<PAIR key=\"kmm-orig-postdate\" value=\"2020-08-01\"/>"
        "</KEYVALUEPAIRS>"
        "<TAG id=\"G000002\"/>"
        "<TAG id=\"G000003\"/>"
        "</SPLIT>"
        "<SPLIT account=\"A000002\" action=\"Deposit\" bankid=\"\" id=\"S0002\" memo=\"\" number=\"\" payee=\"\" price=\"1/1\" reconciledate=\"\" "
        "reconcileflag=\"1\" shares=\"-2/1\" value=\"-2/1\"/>"
        "<SPLIT account=\"A000003\" action=\"Deposit\" bankid=\"\" id=\"S0003\" memo=\"\" number=\"\" payee=\"\" price=\"1/1\" reconciledate=\"\" "
        "reconcileflag=\"0\" shares=\"-1/1\" value=\"-1/1\">"
        "</SPLIT>"
        "</SPLITS>"
        "</TRANSACTION>"
        "</TRANSACTIONS>"));

    QCOMPARE(r->read(createFile(data)), true);

    const auto model = MyMoneyFile::instance()->journalModel();
    QCOMPARE(model->rowCount(), 3);

    auto idx = model->index(0, 0);
    auto journalEntry = model->itemByIndex(idx);
    auto transaction = journalEntry.transaction();
    auto split = journalEntry.split();
    QCOMPARE(transaction.commodity(), QLatin1String("EUR"));
    QCOMPARE(transaction.postDate(), QDate(2020, 7, 29));
    QCOMPARE(transaction.entryDate(), QDate(2020, 7, 30));
    QCOMPARE(idx.data(eMyMoney::Model::SplitAccountIdRole).toString(), QLatin1String("A000076"));
    QCOMPARE(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>().toDouble(), 3.0);
    QCOMPARE(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().toDouble(), 3.0);
    QCOMPARE(split.id(), QLatin1String("S0001"));
    QCOMPARE(split.action(), QLatin1String("Deposit"));
    QCOMPARE(split.bankID(), QLatin1String("A000076-2020-07-28-fa28c20-1"));
    QCOMPARE(split.memo(), QLatin1String("DA Miete\nEREF: 000000009802"));
    QCOMPARE(split.number(), QLatin1String("001"));
    QCOMPARE(split.payeeId(), QLatin1String("P000118"));
    QCOMPARE(split.reconcileFlag(), eMyMoney::Split::State::Reconciled);
    QCOMPARE(split.reconcileDate(), QDate(2020, 7, 31));
    QCOMPARE(split.value(QLatin1String("kmm-match-split")), QLatin1String("S0003"));
    QCOMPARE(split.value(QLatin1String("kmm-orig-memo")), QLatin1String("omemo"));
    QCOMPARE(split.value(QLatin1String("kmm-orig-postdate")), QLatin1String("2020-08-01"));
    QCOMPARE(split.tagIdList().count(), 2);
    QCOMPARE(split.tagIdList().contains(QLatin1String("G000002")), true);
    QCOMPARE(split.tagIdList().contains(QLatin1String("G000003")), true);

    transaction = split.matchedTransaction();
    QCOMPARE(transaction.commodity(), QLatin1String("EUR"));
    QCOMPARE(transaction.postDate(), QDate(2020, 7, 28));
    QCOMPARE(transaction.splitCount(), 3);

    idx = model->index(1, 0);
    journalEntry = model->itemByIndex(idx);
    split = journalEntry.split();
    QCOMPARE(idx.data(eMyMoney::Model::SplitAccountIdRole).toString(), QLatin1String("A000002"));
    QCOMPARE(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>().toDouble(), -2.0);
    QCOMPARE(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().toDouble(), -2.0);
    QCOMPARE(split.reconcileFlag(), eMyMoney::Split::State::Cleared);

    idx = model->index(2, 0);
    journalEntry = model->itemByIndex(idx);
    split = journalEntry.split();
    QCOMPARE(idx.data(eMyMoney::Model::SplitAccountIdRole).toString(), QLatin1String("A000003"));
    QCOMPARE(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>().toDouble(), -1.0);
    QCOMPARE(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().toDouble(), -1.0);
    QCOMPARE(split.reconcileFlag(), eMyMoney::Split::State::NotReconciled);
}

void MyMoneyXmlReaderTest::testReadKeyValuePairs()
{
    auto data = QLatin1String(
        "<FILEINFO>"
        "<CREATION_DATE date=\"2003-09-29\"/>"
        "<LAST_MODIFIED_DATE date=\"2022-01-05\"/>"
        "<VERSION id=\"1\"/>"
        "<FIXVERSION id=\"5\"/>"
        "</FILEINFO>"
        "<KEYVALUEPAIRS>"
        "<PAIR key=\"CreationDate\" value=\"2003-09-29\"/>"
        "<PAIR key=\"FixVersion\" value=\"5\"/>"
        "<PAIR key=\"LastModificationDate\" value=\"2022-01-05\"/>"
        "<PAIR key=\"kmm-baseCurrency\" value=\"EUR\"/>"
        "<PAIR key=\"kmm-encryption-key\" value=\"0xB7BA5DD3\"/>"
        "<PAIR key=\"kmm-id\" value=\"{7bc9cc97-d690-4540-90c2-6e8f88b27581}\"/>"
        "</KEYVALUEPAIRS>");

    QCOMPARE(r->read(createFile(data)), true);

    const auto model = MyMoneyFile::instance()->parametersModel();
    QCOMPARE(model->rowCount(), 6);
    QCOMPARE(model->itemById(QLatin1String("CreationDate")).value(), QLatin1String("2003-09-29"));
    QCOMPARE(model->itemById(QLatin1String("FixVersion")).value().toUInt(), 5);
    QCOMPARE(model->itemById(QLatin1String("LastModificationDate")).value(), QLatin1String("2022-01-05"));
    QCOMPARE(model->itemById(QLatin1String("kmm-baseCurrency")).value(), QLatin1String("EUR"));
    QCOMPARE(model->itemById(QLatin1String("kmm-encryption-key")).value(), QLatin1String("0xB7BA5DD3"));
    QCOMPARE(model->itemById(QLatin1String("kmm-id")).value(), QLatin1String("{7bc9cc97-d690-4540-90c2-6e8f88b27581}"));
}

void MyMoneyXmlReaderTest::testReadSchedules()
{
    auto data = createInstitutionData();
    data.append(createAccountsAndCategoryData());

    data.append(
        QLatin1String("<SCHEDULES>"
                      "<SCHEDULED_TX autoEnter=\"0\" endDate=\"\" fixed=\"1\" id=\"SCH000148\" lastDayInMonth=\"0\" lastPayment=\"\" name=\"Monthly payment\" "
                      "occurence=\"32\" occurenceMultiplier=\"1\" paymentType=\"64\" startDate=\"2016-11-18\" type=\"1\" weekendOption=\"0\">"
                      "<PAYMENTS/>"
                      "<TRANSACTION commodity=\"EUR\" entrydate=\"\" id=\"\" memo=\"\" postdate=\"2022-07-18\">"
                      "<SPLITS>"
                      "<SPLIT account=\"A000076\" action=\"\" bankid=\"\" id=\"S0001\" memo=\"\" number=\"\" payee=\"P000069\" price=\"1/1\" "
                      "reconciledate=\"\" reconcileflag=\"0\" shares=\"-600/1\" value=\"-600/1\">"
                      "<TAG id=\"G000005\"/>"
                      "</SPLIT>"
                      "<SPLIT account=\"A000128\" action=\"\" bankid=\"\" id=\"S0002\" memo=\"\" number=\"\" payee=\"\" price=\"1/1\" reconciledate=\"\" "
                      "reconcileflag=\"0\" shares=\"380/1\" value=\"380/1\"/>"
                      "<SPLIT account=\"A000136\" action=\"\" bankid=\"\" id=\"S0003\" memo=\"\" number=\"\" payee=\"\" price=\"1/1\" reconciledate=\"\" "
                      "reconcileflag=\"0\" shares=\"220/1\" value=\"220/1\"/>"
                      "</SPLITS>"
                      "</TRANSACTION>"
                      "</SCHEDULED_TX>"
                      "</SCHEDULES>"));

    QCOMPARE(r->read(createFile(data)), true);

    const auto model = MyMoneyFile::instance()->schedulesModel();

    const auto parentCount(model->rowCount());
    QCOMPARE(parentCount, 4);
    int childCount(0);
    for (int row(0); row < parentCount; ++row) {
        const auto index = model->index(row, 0);
        childCount += model->rowCount(index);
    }
    QCOMPARE(childCount, 1);

    const auto schedule = model->itemById(QLatin1String("SCH000148"));

    QCOMPARE(schedule.id(), QLatin1String("SCH000148"));
    QCOMPARE(schedule.name(), QLatin1String("Monthly payment"));
    QCOMPARE(schedule.occurrence(), eMyMoney::Schedule::Occurrence::Monthly);
    QCOMPARE(schedule.occurrenceMultiplier(), 1);
    QCOMPARE(schedule.paymentType(), eMyMoney::Schedule::PaymentType::BankTransfer);
    QCOMPARE(schedule.type(), eMyMoney::Schedule::Type::Bill);
    QCOMPARE(schedule.isFixed(), true);
    QCOMPARE(schedule.autoEnter(), false);
    QCOMPARE(schedule.weekendOption(), eMyMoney::Schedule::WeekendOption::MoveBefore);
    QCOMPARE(schedule.startDate(), QDate(2016, 11, 18));
    QCOMPARE(schedule.endDate(), QDate());
    QCOMPARE(schedule.lastDayInMonth(), false);

    QCOMPARE(schedule.transaction().splitCount(), 3);
}

void MyMoneyXmlReaderTest::testReadSecurities()
{
    const auto data = QLatin1String(
        "<SECURITIES>"
        "<SECURITY id=\"E000003\" name=\"TietoEnator\" pp=\"4\" rounding-method=\"4\" saf=\"100\" symbol=\"TIEN.ST\" trading-currency=\"SEK\" "
        "trading-market=\"Stockholm\" type=\"0\">"
        "<KEYVALUEPAIRS>"
        "<PAIR key=\"kmm-online-quote-system\" value=\"Finance::Quote\"/>"
        "<PAIR key=\"kmm-online-source\" value=\"union\"/>"
        "</KEYVALUEPAIRS>"
        "</SECURITY>"
        "</SECURITIES>");

    QCOMPARE(r->read(createFile(data)), true);

    const auto model = MyMoneyFile::instance()->securitiesModel();
    QCOMPARE(model->rowCount(), 1);
    const auto security = model->itemById(QLatin1String("E000003"));

    QCOMPARE(security.id(), QLatin1String("E000003"));
    QCOMPARE(security.name(), QLatin1String("TietoEnator"));
    QCOMPARE(security.securityType(), eMyMoney::Security::Type::Stock);
    QCOMPARE(security.isCurrency(), false);
    QCOMPARE(security.pricePrecision(), 4);
    QCOMPARE(security.roundingMethod(), AlkValue::RoundPromote);
    QCOMPARE(security.smallestAccountFraction(), 100);
    QCOMPARE(security.tradingSymbol(), QLatin1String("TIEN.ST"));
    QCOMPARE(security.tradingCurrency(), QLatin1String("SEK"));
    QCOMPARE(security.tradingMarket(), QLatin1String("Stockholm"));
    QCOMPARE(security.value(QLatin1String("kmm-online-quote-system")), QLatin1String("Finance::Quote"));
    QCOMPARE(security.value(QLatin1String("kmm-online-source")), QLatin1String("union"));
}

void MyMoneyXmlReaderTest::testReadCurrencies()
{
    const auto data = QLatin1String(
        "<CURRENCIES>"
        "<CURRENCY id=\"HUF\" name=\"Hungarian Forint\" pp=\"4\" rounding-method=\"0\" saf=\"100\" scf=\"1\" symbol=\"HUF\" type=\"3\"/>"
        "</CURRENCIES>");

    QCOMPARE(r->read(createFile(data)), true);

    const auto model = MyMoneyFile::instance()->currenciesModel();
    QCOMPARE(model->rowCount(), 1);
    const auto currency = model->itemById(QLatin1String("HUF"));
    QCOMPARE(currency.id(), QLatin1String("HUF"));
    QCOMPARE(currency.name(), QLatin1String("Hungarian Forint"));
    QCOMPARE(currency.securityType(), eMyMoney::Security::Type::Currency);
    QCOMPARE(currency.isCurrency(), true);
    QCOMPARE(currency.pricePrecision(), 4);
    QCOMPARE(currency.roundingMethod(), AlkValue::RoundNever);
    QCOMPARE(currency.smallestAccountFraction(), 100);
    QCOMPARE(currency.smallestCashFraction(), 1);
}

void MyMoneyXmlReaderTest::testReadPrices_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QDate>("date");
    QTest::addColumn<MyMoneyMoney>("value");
    QTest::addColumn<QString>("source");

    QTest::newRow("1") << 0 << QDate(2005, 12, 26) << MyMoneyMoney(21, 25) << QStringLiteral("Yahoo Currency");
    QTest::newRow("2") << 1 << QDate(2005, 12, 27) << MyMoneyMoney(8447, 10000) << QStringLiteral("Yahoo Currency");
    QTest::newRow("3") << 2 << QDate(2006, 1, 25) << MyMoneyMoney(1629, 2000) << QStringLiteral("Yahoo Prices");
}

void MyMoneyXmlReaderTest::testReadPrices()
{
    const auto data = QLatin1String(
        "<PRICES>"
        "<PRICEPAIR from=\"USD\" to=\"EUR\">"
        "<PRICE date=\"2005-12-26\" price=\"21/25\" source=\"Yahoo Currency\"/>"
        "<PRICE date=\"2005-12-27\" price=\"8447/10000\" source=\"Yahoo Currency\"/>"
        "<PRICE date=\"2006-01-25\" price=\"1629/2000\" source=\"Yahoo Prices\"/>"
        "</PRICEPAIR>"
        "</PRICES>");

    QCOMPARE(r->read(createFile(data)), true);

    const auto model = MyMoneyFile::instance()->priceModel();
    const auto rows = model->rowCount();
    const auto pair = qMakePair(QStringLiteral("USD"), QStringLiteral("EUR"));

    QCOMPARE(rows, 3);

    QFETCH(int, row);
    QFETCH(QDate, date);
    QFETCH(MyMoneyMoney, value);
    QFETCH(QString, source);

    const auto index = model->index(row, 0);
    const auto price = model->itemByIndex(index);

    QCOMPARE(price.date(), date);
    QCOMPARE(price.pricePair(), pair);
    QCOMPARE(price.rate(QString()), value);
    QCOMPARE(price.source(), source);
}

void MyMoneyXmlReaderTest::testReadBudgets()
{
    const auto data = QLatin1String(
        "<BUDGETS>"
        "<BUDGET id=\"B000001\" name=\"Budget 2008\" start=\"2008-01-01\" version=\"2\">"
        "<ACCOUNT budgetlevel=\"monthly\" budgetsubaccounts=\"0\" id=\"A000002\">"
        "<PERIOD amount=\"740/1\" start=\"2008-01-01\"/>"
        "</ACCOUNT>"
        "<ACCOUNT budgetlevel=\"monthbymonth\" budgetsubaccounts=\"0\" id=\"A000011\">"
        "<PERIOD amount=\"4875/10\" start=\"2008-01-01\"/>"
        "<PERIOD amount=\"4875/10\" start=\"2008-02-01\"/>"
        "<PERIOD amount=\"4875/10\" start=\"2008-03-01\"/>"
        "<PERIOD amount=\"4875/10\" start=\"2008-04-01\"/>"
        "<PERIOD amount=\"4875/10\" start=\"2008-05-01\"/>"
        "<PERIOD amount=\"8125/10\" start=\"2008-06-01\"/>"
        "<PERIOD amount=\"4875/10\" start=\"2008-07-01\"/>"
        "<PERIOD amount=\"4875/10\" start=\"2008-08-01\"/>"
        "<PERIOD amount=\"4875/10\" start=\"2008-09-01\"/>"
        "<PERIOD amount=\"4875/10\" start=\"2008-10-01\"/>"
        "<PERIOD amount=\"8125/10\" start=\"2008-11-01\"/>"
        "<PERIOD amount=\"4875/10\" start=\"2008-12-01\"/>"
        "</ACCOUNT>"
        "<ACCOUNT budgetlevel=\"monthbymonth\" budgetsubaccounts=\"1\" id=\"A000037\">"
        "<PERIOD amount=\"3/1\" start=\"2008-02-01\"/>"
        "<PERIOD amount=\"3/1\" start=\"2008-05-01\"/>"
        "<PERIOD amount=\"3/1\" start=\"2008-08-01\"/>"
        "<PERIOD amount=\"3/1\" start=\"2008-11-01\"/>"
        "</ACCOUNT>"
        "<ACCOUNT budgetlevel=\"yearly\" budgetsubaccounts=\"0\" id=\"A000108\">"
        "<PERIOD amount=\"500/1\" start=\"2008-01-01\"/>"
        "</ACCOUNT>"
        "</BUDGET>"
        "</BUDGETS>");

    QCOMPARE(r->read(createFile(data)), true);

    const auto model = MyMoneyFile::instance()->budgetsModel();
    const auto rows = model->rowCount();

    QCOMPARE(rows, 1);

    const auto budget = model->itemById(QLatin1String("B000001"));
    QCOMPARE(budget.id(), QLatin1String("B000001"));
    QCOMPARE(budget.name(), QLatin1String("Budget 2008"));
    QCOMPARE(budget.budgetStart(), QDate(2008, 1, 1));
    QCOMPARE(budget.accountsMap().count(), 4);
    QCOMPARE(budget.contains(QLatin1String("A000002")), true);
    QCOMPARE(budget.contains(QLatin1String("A000011")), true);
    QCOMPARE(budget.contains(QLatin1String("A000037")), true);
    QCOMPARE(budget.contains(QLatin1String("A000108")), true);

    auto accountGroup = budget.account(QLatin1String("A000002"));
    QCOMPARE(accountGroup.budgetLevel(), eMyMoney::Budget::Level::Monthly);
    QCOMPARE(accountGroup.budgetSubaccounts(), false);
    QCOMPARE(accountGroup.getPeriods().count(), 1);
    QCOMPARE(accountGroup.getPeriods()[QDate(2008, 1, 1)].amount(), MyMoneyMoney(740, 1));

    accountGroup = budget.account(QLatin1String("A000011"));
    QCOMPARE(accountGroup.budgetLevel(), eMyMoney::Budget::Level::MonthByMonth);
    QCOMPARE(accountGroup.budgetSubaccounts(), false);
    QCOMPARE(accountGroup.getPeriods().count(), 12);
    QCOMPARE(accountGroup.getPeriods()[QDate(2008, 1, 1)].amount(), MyMoneyMoney(4875, 10));
    QCOMPARE(accountGroup.getPeriods()[QDate(2008, 6, 1)].amount(), MyMoneyMoney(8125, 10));

    accountGroup = budget.account(QLatin1String("A000037"));
    QCOMPARE(accountGroup.budgetLevel(), eMyMoney::Budget::Level::MonthByMonth);
    QCOMPARE(accountGroup.budgetSubaccounts(), true);
    QCOMPARE(accountGroup.getPeriods().count(), 4);
    QCOMPARE(accountGroup.getPeriods()[QDate(2008, 1, 1)].amount(), MyMoneyMoney());
    QCOMPARE(accountGroup.getPeriods()[QDate(2008, 2, 1)].amount(), MyMoneyMoney(3, 1));

    accountGroup = budget.account(QLatin1String("A000108"));
    QCOMPARE(accountGroup.budgetLevel(), eMyMoney::Budget::Level::Yearly);
    QCOMPARE(accountGroup.budgetSubaccounts(), false);
    QCOMPARE(accountGroup.getPeriods().count(), 1);
    QCOMPARE(accountGroup.getPeriods()[QDate(2008, 1, 1)].amount(), MyMoneyMoney(500, 1));
}

void MyMoneyXmlReaderTest::testReadOnlineJobs()
{
    const auto data = QLatin1String(
        "<ONLINEJOBS>"
        "<ONLINEJOB bankAnswerDate=\"2015-05-30T15:02:39\" bankAnswerState=\"acceptedByBank\" id=\"O000002\" send=\"2015-05-30T15:02:36\">"
        "<onlineTask iid=\"org.kmymoney.creditTransfer.sepa\" originAccount=\"A000076\" purpose=\"Purpose Line 1&#10;Purpose Line 2\" subTextKey=\"0\" "
        "textKey=\"51\" value=\"75/10\">"
        "<beneficiary bic=\"NASSDE55\" iban=\"DE08154711081547110815\" ownerName=\"Somebody\"/>"
        "</onlineTask>"
        "</ONLINEJOB>"
        "</ONLINEJOBS>");

    QCOMPARE(r->read(createFile(data)), true);

    const auto model = MyMoneyFile::instance()->onlineJobsModel();
    const auto rows = model->rowCount();

    QCOMPARE(rows, 1);

    const auto onlineJob = model->itemById(QLatin1String("O000002"));
    QCOMPARE(onlineJob.id(), QLatin1String("O000002"));
    QCOMPARE(onlineJob.sendDate(), QDateTime(QDate(2015, 5, 30), QTime(15, 2, 36)));
    QCOMPARE(onlineJob.bankAnswerDate(), QDateTime(QDate(2015, 5, 30), QTime(15, 2, 39)));
    QCOMPARE(onlineJob.bankAnswerState(), eMyMoney::OnlineJob::sendingState::acceptedByBank);
    QCOMPARE(onlineJob.taskIid(), QLatin1String("org.kmymoney.creditTransfer.sepa"));
    QCOMPARE(onlineJob.responsibleAccount(), QLatin1String("A000076"));
    QCOMPARE(onlineJob.purpose(), QLatin1String("Purpose Line 1\nPurpose Line 2"));

    onlineJobTyped<creditTransfer> transfer(onlineJob);
    const payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBic(transfer.constTask()->beneficiary());
    QCOMPARE(transfer.task()->value().toDouble(), 7.5);
    QCOMPARE(ibanBic->ownerName(), QLatin1String("Somebody"));
    QCOMPARE(ibanBic->bic(), QLatin1String("NASSDE55"));
    QCOMPARE(ibanBic->electronicIban(), QLatin1String("DE08154711081547110815"));
}
