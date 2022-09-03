/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

#include <QColor>
#include <QDate>
#include <QIODevice>
#include <QMap>
#include <QSharedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ibanbic/ibanbic.h"
#include "mymoneyaccount.h"
#include "mymoneybudget.h"
#include "mymoneycostcenter.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneyreport.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneystoragenames.h"
#include "mymoneytag.h"
#include "mymoneytransaction.h"
#include "mymoneyutils.h"
#include "mymoneyxmlwriter.h"
#include "nationalaccount/nationalaccount.h"
#include "onlinejobadministration.h"
#include "payeeidentifier.h"
#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "payeeidentifiertyped.h"
#include "xmlstoragehelper.h"

#include "budgetsmodel.h"
#include "costcentermodel.h"
#include "institutionsmodel.h"
#include "journalmodel.h"
#include "onlinejobsmodel.h"
#include "parametersmodel.h"
#include "payeesmodel.h"
#include "pricemodel.h"
#include "reportsmodel.h"
#include "schedulesmodel.h"
#include "securitiesmodel.h"
#include "tagsmodel.h"

#include "mymoneyanonwriter.h"
#include "mymoneyxmlwriter_p.h"

using namespace MyMoneyXmlHelper;

class MyMoneyAnonWriterPrivate : public MyMoneyXmlWriterPrivate
{
public:
    MyMoneyAnonWriterPrivate();

    static void writeKeyValueContainer(QXmlStreamWriter* writer, const MyMoneyKeyValueContainer& kvp);
    static void writeKeyValueContainer(QXmlStreamWriter* writer, const QMap<QString, QString>& pairs);
    static void
    writeAddress(QXmlStreamWriter* writer, const QString& street, const QString& city, const QString& state, const QString& zip, const QString& phone);
    static void writeInstitution(const MyMoneyInstitution& institution, QXmlStreamWriter* writer);
    static void writePayee(const MyMoneyPayee& payee, QXmlStreamWriter* writer);
    static void writeTag(const MyMoneyTag& tag, QXmlStreamWriter* writer);
    void writeAccount(const MyMoneyAccount& account) override;
    static void writeCostCenter(const MyMoneyCostCenter& costCenter, QXmlStreamWriter* writer);
    void writeTransaction(QXmlStreamWriter* writer, const MyMoneyTransaction& account) override;
    void writeSchedule(QXmlStreamWriter* writer, const MyMoneySchedule& schedule) override;
    static void writeSecurity(const MyMoneySecurity& security, QXmlStreamWriter* writer);
    static void writeReport(const MyMoneyReport& report, QXmlStreamWriter* writer);
    static void writeBudget(const MyMoneyBudget& budget, QXmlStreamWriter* writer);
    static void writeOnlineJob(const onlineJob& job, QXmlStreamWriter* writer);

    void writeUserInformation() override;
    void writeInstitutions() override;
    void writePayees() override;
    void writeTags() override;
    void writeCostCenters() override;
    void writeSecurities() override;
    void writeReports() override;
    void writeBudgets() override;
    void writeOnlineJobs() override;

    static QString hideString(const QString& _in);
    static MyMoneyMoney hideNumber(const MyMoneyMoney& _in);

    static MyMoneyBudget fakeBudget(const MyMoneyBudget& bx);
    static MyMoneyTransaction fakeTransaction(const MyMoneyTransaction& transaction);
    static QMap<QString, QString> fakeKeyValuePair(const MyMoneyKeyValueContainer& kvp);
    static QMap<QString, QString> fakeKeyValuePair(const QMap<QString, QString>& pairs);

    static MyMoneyMoney m_factor;
    /**
     * The list of key-value pairs to not modify
     */
    static QStringList zKvpNoModify;

    /**
     * The list of key-value pairs which are numbers to be hidden
     */
    static QStringList zKvpXNumber;
};

QStringList MyMoneyAnonWriterPrivate::zKvpNoModify =
    QString(
        "kmm-baseCurrency,OpeningBalanceAccount,PreferredAccount,Tax,fixed-interest,interest-calculation,payee,schedule,term,kmm-online-source,kmm-brokerage-"
        "account,kmm-sort-reconcile,kmm-sort-std,kmm-iconpos,mm-closed,payee,schedule,term,lastImportedTransactionDate,VatAccount,VatRate,kmm-matched-tx,"
        "Imported,priceMode")
        .split(',');
QStringList MyMoneyAnonWriterPrivate::zKvpXNumber = QString("final-payment,loan-amount,periodic-payment,lastStatementBalance").split(',');
MyMoneyMoney MyMoneyAnonWriterPrivate::m_factor;

MyMoneyAnonWriterPrivate::MyMoneyAnonWriterPrivate()
    : MyMoneyXmlWriterPrivate()
{
    // Choose a quasi-random 0.0-100.0 factor which will be applied to all splits this time
    // around.

    int msec;
    do {
        msec = QTime::currentTime().msec();
    } while (msec == 0);
    m_factor = MyMoneyMoney(msec, 10).reduce();
}

QString MyMoneyAnonWriterPrivate::hideString(const QString& _in)
{
    return QString(_in).fill('x');
}

MyMoneyMoney MyMoneyAnonWriterPrivate::hideNumber(const MyMoneyMoney& _in)
{
    MyMoneyMoney result;
    static MyMoneyMoney counter = MyMoneyMoney(100, 100);

    // preserve sign
    if (_in.isNegative())
        result = MyMoneyMoney::MINUS_ONE;
    else
        result = MyMoneyMoney::ONE;

    result = result * counter;
    counter += MyMoneyMoney(10, 100);

    // preserve > 1000
    if ((_in >= MyMoneyMoney(1000, 1)) || (_in <= MyMoneyMoney(-1000, 1))) {
        result = result * MyMoneyMoney(1000, 1);
    }

    return result.convert();
}

MyMoneyBudget MyMoneyAnonWriterPrivate::fakeBudget(const MyMoneyBudget& bx)
{
    MyMoneyBudget bn;

    bn.setName(bx.id());
    bn.setBudgetStart(bx.budgetStart());
    bn = MyMoneyBudget(bx.id(), bn);

    QList<MyMoneyBudget::AccountGroup> list = bx.getaccounts();
    QList<MyMoneyBudget::AccountGroup>::iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        // only add the account if there is a budget entered
        if (!(*it).balance().isZero()) {
            MyMoneyBudget::AccountGroup account;
            account.setId((*it).id());
            account.setBudgetLevel((*it).budgetLevel());
            account.setBudgetSubaccounts((*it).budgetSubaccounts());
            QMap<QDate, MyMoneyBudget::PeriodGroup> plist = (*it).getPeriods();
            QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_p;
            for (it_p = plist.constBegin(); it_p != plist.constEnd(); ++it_p) {
                MyMoneyBudget::PeriodGroup pGroup;
                pGroup.setAmount((*it_p).amount() * m_factor);
                pGroup.setStartDate((*it_p).startDate());
                account.addPeriod(pGroup.startDate(), pGroup);
            }
            bn.setAccount(account, account.id());
        }
    }

    return bn;
}

QMap<QString, QString> MyMoneyAnonWriterPrivate::fakeKeyValuePair(const MyMoneyKeyValueContainer& kvp)
{
    return fakeKeyValuePair(kvp.pairs());
}

QMap<QString, QString> MyMoneyAnonWriterPrivate::fakeKeyValuePair(const QMap<QString, QString>& pairs)
{
    QMap<QString, QString> newPairs;
    QMap<QString, QString>::const_iterator it;

    for (it = pairs.constBegin(); it != pairs.constEnd(); ++it) {
        if (zKvpXNumber.contains(it.key()) || it.key().left(3) == "ir-")
            newPairs[it.key()] = hideNumber(MyMoneyMoney(it.value())).toString();
        else if (zKvpNoModify.contains(it.key()))
            newPairs[it.key()] = it.value();
        else
            newPairs[it.key()] = hideString(it.value());
    }
    return newPairs;
}

MyMoneyTransaction MyMoneyAnonWriterPrivate::fakeTransaction(const MyMoneyTransaction& transaction)
{
    auto anonTransaction(transaction);

    // hide transaction data
    anonTransaction.setMemo(transaction.id());
    anonTransaction.setBankID(hideString(transaction.bankID()));

    // hide split data
    for (const auto& split : transaction.splits()) {
        MyMoneySplit s = split;
        s.setMemo(QString("%1/%2").arg(anonTransaction.id()).arg(s.id()));

        if (s.value() != MyMoneyMoney::autoCalc) {
            s.setValue((s.value() * m_factor));
            s.setShares((s.shares() * m_factor));
        }
        s.setNumber(hideString(s.number()));

        // obfuscate a possibly matched transaction as well
        if (s.isMatched()) {
            MyMoneyTransaction t = s.matchedTransaction();
            fakeTransaction(t);
            s.removeMatch();
            s.addMatch(t);
        }
        anonTransaction.modifySplit(s);
    }
    anonTransaction.setPairs(fakeKeyValuePair(anonTransaction));

    return anonTransaction;
}

void MyMoneyAnonWriterPrivate::writeKeyValueContainer(QXmlStreamWriter* writer, const MyMoneyKeyValueContainer& kvp)
{
    writeKeyValueContainer(writer, kvp.pairs());
}

void MyMoneyAnonWriterPrivate::writeKeyValueContainer(QXmlStreamWriter* writer, const QMap<QString, QString>& pairs)
{
    MyMoneyXmlWriterPrivate::writeKeyValueContainer(writer, fakeKeyValuePair(pairs));
}

void MyMoneyAnonWriterPrivate::writeAddress(QXmlStreamWriter* writer,
                                            const QString& street,
                                            const QString& city,
                                            const QString& state,
                                            const QString& zip,
                                            const QString& phone)
{
    MyMoneyXmlWriterPrivate::writeAddress(writer, hideString(street), hideString(city), hideString(state), hideString(zip), hideString(phone));
}

#if 0
///  @todo anonymize - see also writePayee()
void MyMoneyAnonWriterPrivate::writePayeeIdentifier(QXmlStreamWriter* writer, const payeeIdentifier& obj)
{
    // Important: type must be set before calling m_payeeIdentifier->writeXML()
    // the plugin for unavailable plugins must be able to set type itself
    writer->writeStartElement(elementName(Element::Payee::Identifier));
    if (obj.id() != 0) {
        writer->writeAttribute(attributeName(Attribute::Payee::ID), QString::number(obj.id()));
    }
    if (!obj.isNull()) {
        writer->writeAttribute(attributeName(Attribute::Payee::Type), obj->payeeIdentifierId());
        obj->writeXML(writer);
    }
    writer->writeEndElement();
}
#endif

void MyMoneyAnonWriterPrivate::writeUserInformation()
{
    m_writer->writeStartElement(tagName(Tag::User));

    const auto user = m_file->userModel()->itemById(m_file->fixedKey(MyMoneyFile::UserID));
    m_writer->writeAttribute(attributeName(Attribute::General::Name), hideString(user.name()));
    m_writer->writeAttribute(attributeName(Attribute::General::Email), hideString(user.email()));

    writeAddress(m_writer, user.address(), user.city(), user.state(), user.postcode(), user.telephone());

    m_writer->writeEndElement();
}

void MyMoneyAnonWriterPrivate::writeInstitution(const MyMoneyInstitution& institution, QXmlStreamWriter* writer)
{
    MyMoneyInstitution anonInstitution(institution);

    // mangle fields
    anonInstitution.setName(anonInstitution.id());
    anonInstitution.setManager(hideString(anonInstitution.manager()));
    anonInstitution.setBankCode(hideString(anonInstitution.bankcode()));

    anonInstitution.setStreet(hideString(anonInstitution.street()));
    anonInstitution.setCity(hideString(anonInstitution.city()));
    anonInstitution.setPostcode(hideString(anonInstitution.postcode()));
    anonInstitution.setTelephone(hideString(anonInstitution.telephone()));

    anonInstitution.setPairs(fakeKeyValuePair(institution));

    MyMoneyXmlWriterPrivate::writeInstitution(anonInstitution, writer);
}

void MyMoneyAnonWriterPrivate::writeInstitutions()
{
    m_writer->writeStartElement(tagName(Tag::Institutions));

    InstitutionsModel::xmlWriter writer(&MyMoneyAnonWriterPrivate::writeInstitution, m_writer);
    m_file->institutionsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyAnonWriterPrivate::writePayee(const MyMoneyPayee& payee, QXmlStreamWriter* writer)
{
    MyMoneyPayee anonPayee(payee);

    anonPayee.setName(anonPayee.id());
    anonPayee.setReference(hideString(anonPayee.reference()));

    anonPayee.setAddress(hideString(anonPayee.address()));
    anonPayee.setCity(hideString(anonPayee.city()));
    anonPayee.setPostcode(hideString(anonPayee.postcode()));
    anonPayee.setState(hideString(anonPayee.state()));
    anonPayee.setTelephone(hideString(anonPayee.telephone()));
    anonPayee.setNotes(hideString(anonPayee.notes()));
    bool ignoreCase;
    QStringList keys;
    auto matchType = anonPayee.matchData(ignoreCase, keys);
    const QRegularExpression expChar("[A-Za-z]");
    const QRegularExpression expNum("[0-9]");
    anonPayee.setMatchData(matchType, ignoreCase, keys.join(";").replace(expChar, "x").replace(expNum, "#").split(';'));

    /// @todo anonymize: data from plugins cannot be estranged, yet.
    anonPayee.resetPayeeIdentifiers();

    MyMoneyXmlWriterPrivate::writePayee(anonPayee, writer);
}

void MyMoneyAnonWriterPrivate::writePayees()
{
    m_writer->writeStartElement(tagName(Tag::Payees));

    PayeesModel::xmlWriter writer(&MyMoneyAnonWriterPrivate::writePayee, m_writer);
    m_file->payeesModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyAnonWriterPrivate::writeCostCenter(const MyMoneyCostCenter& costCenter, QXmlStreamWriter* writer)
{
    auto cc(costCenter);
    cc.setName(hideString(costCenter.name()));

    MyMoneyXmlWriterPrivate::writeCostCenter(cc, writer);
}

void MyMoneyAnonWriterPrivate::writeCostCenters()
{
    m_writer->writeStartElement(tagName(Tag::CostCenters));

    CostCenterModel::xmlWriter writer(&MyMoneyAnonWriterPrivate::writeCostCenter, m_writer);
    m_file->costCenterModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyAnonWriterPrivate::writeTag(const MyMoneyTag& tag, QXmlStreamWriter* writer)
{
    auto t(tag);

    t.setName(tag.id());
    t.setNotes(hideString(tag.notes()));

    MyMoneyXmlWriterPrivate::writeTag(t, writer);
}

void MyMoneyAnonWriterPrivate::writeTags()
{
    m_writer->writeStartElement(tagName(Tag::Tags));

    TagsModel::xmlWriter writer(&MyMoneyAnonWriterPrivate::writeTag, m_writer);
    m_file->tagsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyAnonWriterPrivate::writeAccount(const MyMoneyAccount& account)
{
    auto newAccount(account);

    const auto isBrokerageAccount = newAccount.name().contains(i18n(" (Brokerage)"));
    newAccount.setNumber(hideString(newAccount.number()));
    newAccount.setName(newAccount.id());
    if (isBrokerageAccount) {
        // search the name of the corresponding investment account
        // and setup the name according to the rule of brokerage accounts
        for (const auto& acc : m_accountList) {
            if (acc.accountType() == eMyMoney::Account::Type::Investment && account.name() == i18n("%1 (Brokerage)", acc.name())) {
                newAccount.setName(i18n("%1 (Brokerage)", acc.id()));
                break;
            }
        }
    }
    newAccount.setDescription(hideString(newAccount.description()));
    fakeKeyValuePair(newAccount);

    // Remove the online banking settings entirely.
    newAccount.setOnlineBankingSettings(MyMoneyKeyValueContainer());

    MyMoneyXmlWriterPrivate::writeAccount(newAccount);
}

void MyMoneyAnonWriterPrivate::writeTransaction(QXmlStreamWriter* writer, const MyMoneyTransaction& transaction)
{
    MyMoneyXmlWriterPrivate::writeTransaction(writer, fakeTransaction(transaction));
}

void MyMoneyAnonWriterPrivate::writeSchedule(QXmlStreamWriter* writer, const MyMoneySchedule& schedule)
{
    auto anonSchedule(schedule);

    anonSchedule.setName(schedule.id());
    anonSchedule.setTransaction(fakeTransaction(schedule.transaction()), true);

    MyMoneyXmlWriterPrivate::writeSchedule(writer, anonSchedule);
}

void MyMoneyAnonWriterPrivate::writeSecurity(const MyMoneySecurity& security, QXmlStreamWriter* writer)
{
    MyMoneySecurity anonSecurity(security);
    anonSecurity.setName(security.id());
    fakeKeyValuePair(anonSecurity);

    MyMoneyXmlWriterPrivate::writeSecurity(anonSecurity, writer);
}

void MyMoneyAnonWriterPrivate::writeSecurities()
{
    m_writer->writeStartElement(tagName(Tag::Securities));

    SecuritiesModel::xmlWriter writer(&MyMoneyAnonWriterPrivate::writeSecurity, m_writer);
    m_file->securitiesModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyAnonWriterPrivate::writeReport(const MyMoneyReport& report, QXmlStreamWriter* writer)
{
    auto anonReport(report);
    anonReport.setName(report.id());
    anonReport.setComment(hideString(report.comment()));

    MyMoneyXmlHelper::writeReport(anonReport, writer);
}

void MyMoneyAnonWriterPrivate::writeReports()
{
    m_writer->writeStartElement(tagName(Tag::Reports));

    ReportsModel::xmlWriter writer(&MyMoneyAnonWriterPrivate::writeReport, m_writer);
    m_file->reportsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyAnonWriterPrivate::writeBudget(const MyMoneyBudget& budget, QXmlStreamWriter* writer)
{
    MyMoneyXmlHelper::writeBudget(fakeBudget(budget), writer);
}

void MyMoneyAnonWriterPrivate::writeBudgets()
{
    m_writer->writeStartElement(tagName(Tag::Budgets));

    BudgetsModel::xmlWriter writer(&MyMoneyAnonWriterPrivate::writeBudget, m_writer);
    m_file->budgetsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyAnonWriterPrivate::writeOnlineJob(const onlineJob& job, QXmlStreamWriter* writer)
{
    MyMoneyXmlHelper::writeStartObject(writer, nodeName(Node::OnlineJob), job.id());

    if (!job.sendDate().isNull())
        writer->writeAttribute(attributeName(Attribute::OnlineJob::Send), MyMoneyUtils::dateToString(job.sendDate().date()));
    if (!job.bankAnswerDate().isNull())
        writer->writeAttribute(attributeName(Attribute::OnlineJob::BankAnswerDate), MyMoneyUtils::dateToString(job.bankAnswerDate().date()));

    switch (job.bankAnswerState()) {
    case eMyMoney::OnlineJob::sendingState::abortedByUser:
        writer->writeAttribute(attributeName(Attribute::OnlineJob::BankAnswerState), attributeName(Attribute::OnlineJob::AbortedByUser));
        break;
    case eMyMoney::OnlineJob::sendingState::acceptedByBank:
        writer->writeAttribute(attributeName(Attribute::OnlineJob::BankAnswerState), attributeName(Attribute::OnlineJob::AcceptedByBank));
        break;
    case eMyMoney::OnlineJob::sendingState::rejectedByBank:
        writer->writeAttribute(attributeName(Attribute::OnlineJob::BankAnswerState), attributeName(Attribute::OnlineJob::RejectedByBank));
        break;
    case eMyMoney::OnlineJob::sendingState::sendingError:
        writer->writeAttribute(attributeName(Attribute::OnlineJob::BankAnswerState), attributeName(Attribute::OnlineJob::SendingError));
        break;
    case eMyMoney::OnlineJob::sendingState::noBankAnswer:
    default:
        break;
    }

    writer->writeStartElement(elementName(Element::OnlineJob::OnlineTask));
    writer->writeAttribute(attributeName(Attribute::OnlineJob::IID), job.taskIid());
#if 0
    /// @todo anonymize
    try {
        job.task()->writeXML(writer); // throws exception if there is no task
    } catch (const onlineJob::emptyTask&) {
    }
#endif
    writer->writeEndElement();

    writer->writeEndElement();
}

void MyMoneyAnonWriterPrivate::writeOnlineJobs()
{
    m_writer->writeStartElement(tagName(Tag::OnlineJobs));

    OnlineJobsModel::xmlWriter writer(&MyMoneyAnonWriterPrivate::writeOnlineJob, m_writer);
    m_file->onlineJobsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

MyMoneyAnonWriter::MyMoneyAnonWriter()
    : MyMoneyXmlWriter(new MyMoneyAnonWriterPrivate)
{
}
