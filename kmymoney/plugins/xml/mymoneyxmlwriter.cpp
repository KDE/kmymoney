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

#include "mymoneyxmlwriter_p.h"

using namespace MyMoneyXmlHelper;

MyMoneyXmlWriterPrivate::MyMoneyXmlWriterPrivate()
    : m_writer(new QXmlStreamWriter)
{
}

MyMoneyXmlWriterPrivate::~MyMoneyXmlWriterPrivate()
{
    delete m_writer;
}

void MyMoneyXmlWriterPrivate::writeKeyValueContainer(QXmlStreamWriter* writer, const MyMoneyKeyValueContainer& kvp)
{
    writeKeyValueContainer(writer, kvp.pairs());
}

void MyMoneyXmlWriterPrivate::writeKeyValueContainer(QXmlStreamWriter* writer, const QMap<QString, QString>& pairs)
{
    if (!pairs.isEmpty()) {
        writer->writeStartElement(nodeName(Node::KeyValuePairs));
        for (auto it = pairs.cbegin(); it != pairs.cend(); ++it) {
            writer->writeStartElement(elementName(Element::KVP::Pair));
            writer->writeAttribute(attributeName(Attribute::KVP::Key), it.key());
            writer->writeAttribute(attributeName(Attribute::KVP::Value), it.value());
            writer->writeEndElement();
        }
        writer->writeEndElement();
    }
}

void MyMoneyXmlWriterPrivate::writeAddress(QXmlStreamWriter* writer,
                                           const QString& street,
                                           const QString& city,
                                           const QString& state,
                                           const QString& zip,
                                           const QString& phone)
{
    writer->writeStartElement(elementName(Element::General::Address));
    writer->writeAttribute(attributeName(Attribute::General::Street), street);
    writer->writeAttribute(attributeName(Attribute::General::City), city);
    writer->writeAttribute(attributeName(Attribute::General::State), state);
    writer->writeAttribute(attributeName(Attribute::General::ZipCode), zip);
    writer->writeAttribute(attributeName(Attribute::General::Telephone), phone);
    /// @todo eventually remove
    // the following four lines only for backward compatability to version 5.1 code
    writer->writeAttribute(attributeName(Attribute::General::Country), state);
    writer->writeAttribute(attributeName(Attribute::General::AltCountry), state);
    writer->writeAttribute(attributeName(Attribute::General::AltZipCode), zip);
    writer->writeAttribute(attributeName(Attribute::General::PostCode), zip);
    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeAddress(const QString& street, const QString& city, const QString& state, const QString& zip, const QString& phone)
{
    writeAddress(m_writer, street, city, state, zip, phone);
}

void MyMoneyXmlWriterPrivate::writeStartObject(QXmlStreamWriter* writer, const QString tagName, const MyMoneyObject& object)
{
    MyMoneyXmlHelper::writeStartObject(writer, tagName, object.id());
}

void MyMoneyXmlWriterPrivate::writePayeeIdentifier(QXmlStreamWriter* writer, const payeeIdentifier& obj)
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

void MyMoneyXmlWriterPrivate::writeFileInformation()
{
    m_writer->writeStartElement(tagName(Tag::FileInfo));

    m_writer->writeStartElement(elementName(Element::General::CreationDate));
    m_writer->writeAttribute(attributeName(Attribute::General::Date), m_file->parametersModel()->itemById(m_file->fixedKey(MyMoneyFile::CreationDate)).value());
    m_writer->writeEndElement();
    m_writer->writeStartElement(elementName(Element::General::LastModifiedDate));
    m_writer->writeAttribute(attributeName(Attribute::General::Date),
                             m_file->parametersModel()->itemById(m_file->fixedKey(MyMoneyFile::LastModificationDate)).value());
    m_writer->writeEndElement();
    m_writer->writeStartElement(elementName(Element::General::Version));
    m_writer->writeAttribute(attributeName(Attribute::General::ID), QLatin1String("1"));
    m_writer->writeEndElement();
    m_writer->writeStartElement(elementName(Element::General::FixVersion));
    m_writer->writeAttribute(attributeName(Attribute::General::Date),
                             m_file->parametersModel()->itemById(m_file->fixedKey(MyMoneyFile::FileFixVersion)).value());
    m_writer->writeEndElement();

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeUserInformation()
{
    m_writer->writeStartElement(tagName(Tag::User));

    const auto user = m_file->userModel()->itemById(m_file->fixedKey(MyMoneyFile::UserID));
    m_writer->writeAttribute(attributeName(Attribute::General::Name), user.name());
    m_writer->writeAttribute(attributeName(Attribute::General::Email), user.email());

    writeAddress(user.address(), user.city(), user.state(), user.postcode(), user.telephone());

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeInstitution(const MyMoneyInstitution& institution, QXmlStreamWriter* writer)
{
    writeStartObject(writer, nodeName(Node::Institution), institution);

    writer->writeAttribute(attributeName(Attribute::Institution::Name), institution.name());
    writer->writeAttribute(attributeName(Attribute::Institution::Manager), institution.manager());
    writer->writeAttribute(attributeName(Attribute::Institution::BankCode), institution.bankcode());

    writeAddress(writer, institution.street(), institution.town(), QString(), institution.postcode(), institution.telephone());

    writer->writeStartElement(elementName(Element::Institution::AccountIDS));
    for (const auto& accountId : institution.accountList()) {
        writer->writeStartElement(elementName(Element::Institution::AccountID));
        writer->writeAttribute(attributeName(Attribute::Institution::ID), accountId);
        writer->writeEndElement();
    }
    writer->writeEndElement();

    // Add in Key-Value Pairs for institutions.
    writeKeyValueContainer(writer, institution);

    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeInstitutions()
{
    m_writer->writeStartElement(tagName(Tag::Institutions));

    InstitutionsModel::xmlWriter writer(&MyMoneyXmlWriterPrivate::writeInstitution, m_writer);
    m_file->institutionsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writePayee(const MyMoneyPayee& payee, QXmlStreamWriter* writer)
{
    writeStartObject(writer, nodeName(Node::Payee), payee);

    writer->writeAttribute(attributeName(Attribute::Payee::Name), payee.name());
    writer->writeAttribute(attributeName(Attribute::Payee::Reference), payee.reference());
    writer->writeAttribute(attributeName(Attribute::Payee::Email), payee.email());
    if (!payee.notes().isEmpty()) {
        writer->writeAttribute(attributeName(Attribute::Payee::Notes), payee.notes());
    }

    writer->writeAttribute(attributeName(Attribute::Payee::MatchingEnabled), attrValue(payee.isMatchingEnabled()));
    if (payee.isMatchingEnabled()) {
        writer->writeAttribute(attributeName(Attribute::Payee::UsingMatchKey), attrValue(payee.isUsingMatchKey()));
        writer->writeAttribute(attributeName(Attribute::Payee::MatchIgnoreCase), attrValue(payee.isMatchKeyIgnoreCase()));
        writer->writeAttribute(attributeName(Attribute::Payee::MatchKey), payee.matchKey());
    }

    if (!payee.defaultAccountId().isEmpty()) {
        writer->writeAttribute(attributeName(Attribute::Payee::DefaultAccountID), payee.defaultAccountId());
    }

    // Save address
    writeAddress(writer, payee.address(), payee.city(), payee.state(), payee.postcode(), payee.telephone());

    // Save payeeIdentifiers (account numbers)
    for (const auto& payeeIdentifier : payee.payeeIdentifiers()) {
        if (!payeeIdentifier.isNull()) {
            writePayeeIdentifier(writer, payeeIdentifier);
        }
    }

    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writePayees()
{
    m_writer->writeStartElement(tagName(Tag::Payees));

    PayeesModel::xmlWriter writer(&MyMoneyXmlWriterPrivate::writePayee, m_writer);
    m_file->payeesModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeCostCenter(const MyMoneyCostCenter& costCenter, QXmlStreamWriter* writer)
{
    writeStartObject(writer, nodeName(Node::CostCenter), costCenter);
    writer->writeAttribute(attributeName(Attribute::CostCenter::Name), costCenter.name());
    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeCostCenters()
{
    m_writer->writeStartElement(tagName(Tag::CostCenters));

    CostCenterModel::xmlWriter writer(&MyMoneyXmlWriterPrivate::writeCostCenter, m_writer);
    m_file->costCenterModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeTag(const MyMoneyTag& tag, QXmlStreamWriter* writer)
{
    writeStartObject(writer, nodeName(Node::Tag), tag);

    writer->writeAttribute(attributeName(Attribute::Tag::Name), tag.name());
    writer->writeAttribute(attributeName(Attribute::Tag::Closed), attrValue(tag.isClosed()));
    if (tag.tagColor().isValid()) {
        writer->writeAttribute(attributeName(Attribute::Tag::TagColor), tag.tagColor().name());
    }
    if (!tag.notes().isEmpty()) {
        writer->writeAttribute(attributeName(Attribute::Tag::Notes), tag.notes());
    }

    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeTags()
{
    m_writer->writeStartElement(tagName(Tag::Tags));

    TagsModel::xmlWriter writer(&MyMoneyXmlWriterPrivate::writeTag, m_writer);
    m_file->tagsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeAccount(const MyMoneyAccount& account)
{
    writeStartObject(m_writer, nodeName(Node::Account), account);

    m_writer->writeAttribute(attributeName(Attribute::Account::ParentAccount), account.parentAccountId());
    m_writer->writeAttribute(attributeName(Attribute::Account::LastReconciled), MyMoneyUtils::dateToString(account.lastReconciliationDate()));
    m_writer->writeAttribute(attributeName(Attribute::Account::LastModified), MyMoneyUtils::dateToString(account.lastModified()));
    m_writer->writeAttribute(attributeName(Attribute::Account::Institution), account.institutionId());
    m_writer->writeAttribute(attributeName(Attribute::Account::Opened), MyMoneyUtils::dateToString(account.openingDate()));
    m_writer->writeAttribute(attributeName(Attribute::Account::Number), account.number());
    m_writer->writeAttribute(attributeName(Attribute::Account::Type), QString::number(static_cast<int>(account.accountType())));
    m_writer->writeAttribute(attributeName(Attribute::Account::Name), account.name());
    m_writer->writeAttribute(attributeName(Attribute::Account::Description), account.description());
    if (!account.currencyId().isEmpty()) {
        m_writer->writeAttribute(attributeName(Attribute::Account::Currency), account.currencyId());
    }

    // Add in subaccount information, if this account has subaccounts.
    if (!account.accountList().isEmpty()) {
        m_writer->writeStartElement(elementName(Element::Account::SubAccounts));
        for (const auto& accountId : account.accountList()) {
            m_writer->writeStartElement(elementName(Element::Account::SubAccount));
            m_writer->writeAttribute(attributeName(Attribute::Account::ID), accountId);
            m_writer->writeEndElement();
        }
        m_writer->writeEndElement();
    }

    // Write online banking settings
    const auto onlineBankSettingsPairs = account.onlineBankingSettings().pairs();
    if (!onlineBankSettingsPairs.isEmpty()) {
        m_writer->writeStartElement(elementName(Element::Account::OnlineBanking));
        QMap<QString, QString>::const_iterator it_key = onlineBankSettingsPairs.constBegin();
        while (it_key != onlineBankSettingsPairs.constEnd()) {
            m_writer->writeAttribute(it_key.key(), it_key.value());
            ++it_key;
        }
        m_writer->writeEndElement();
    }

    // Write reconciliation history
    const auto reconciliationHistory(account.reconciliationHistory());
    if (!reconciliationHistory.isEmpty()) {
        m_writer->writeStartElement(elementName(Element::Account::ReconciliationHistory));

        for (auto it = reconciliationHistory.cbegin(); it != reconciliationHistory.cend(); ++it) {
            m_writer->writeStartElement(elementName(Element::Account::ReconciliationEntry));
            m_writer->writeAttribute(attributeName(Attribute::Reconciliation::Date), MyMoneyUtils::dateToString(it.key()));
            m_writer->writeAttribute(attributeName(Attribute::Reconciliation::Amount), it.value().toString());
            m_writer->writeEndElement();
        }

        m_writer->writeEndElement();
    }

    // Add in Key-Value Pairs for accounts.
    writeKeyValueContainer(m_writer, account);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeAccounts()
{
    m_writer->writeStartElement(tagName(Tag::Accounts));

    m_accountList = m_file->accountsModel()->itemList();
    QList<MyMoneyAccount>::ConstIterator it;

    writeAccount(m_file->accountsModel()->itemByIndex(m_file->accountsModel()->assetIndex()));
    writeAccount(m_file->accountsModel()->itemByIndex(m_file->accountsModel()->liabilityIndex()));
    writeAccount(m_file->accountsModel()->itemByIndex(m_file->accountsModel()->expenseIndex()));
    writeAccount(m_file->accountsModel()->itemByIndex(m_file->accountsModel()->incomeIndex()));
    writeAccount(m_file->accountsModel()->itemByIndex(m_file->accountsModel()->equityIndex()));

    // we save the accounts ordered by id
    // allowing to compare the file contents before and after write
    std::sort(m_accountList.begin(), m_accountList.end(), [](const MyMoneyAccount& a1, const MyMoneyAccount& a2) {
        return a1.id() < a2.id();
    });

    for (const auto& account : m_accountList) {
        writeAccount(account);
    }

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeSplit(QXmlStreamWriter* writer, const MyMoneySplit& _split)
{
    writeStartObject(writer, elementName(Element::Split::Split), _split);

    auto split = _split; // we need to convert matched transaction to kvp pair

    writer->writeAttribute(attributeName(Attribute::Split::Payee), split.payeeId());
    writer->writeAttribute(attributeName(Attribute::Split::ReconcileDate), MyMoneyUtils::dateToString(split.reconcileDate()));
    writer->writeAttribute(attributeName(Attribute::Split::Action), split.action());
    writer->writeAttribute(attributeName(Attribute::Split::ReconcileFlag), QString::number(static_cast<int>(split.reconcileFlag())));
    writer->writeAttribute(attributeName(Attribute::Split::Value), split.value().toString());
    writer->writeAttribute(attributeName(Attribute::Split::Shares), split.shares().toString());
    if (!split.price().isZero()) {
        writer->writeAttribute(attributeName(Attribute::Split::Price), split.price().toString());
    }
    writer->writeAttribute(attributeName(Attribute::Split::Memo), split.memo());
    writer->writeAttribute(attributeName(Attribute::Split::Account), split.accountId());
    writer->writeAttribute(attributeName(Attribute::Split::Number), split.number());
    writer->writeAttribute(attributeName(Attribute::Split::BankID), split.bankID());
    if (!split.costCenterId().isEmpty()) {
        writer->writeAttribute(attributeName(Attribute::Split::CostCenter), split.costCenterId());
    }
    const auto tagIdList = split.tagIdList();
    for (auto i = 0; i < tagIdList.count(); ++i) {
        writer->writeStartElement(elementName(Element::Split::Tag));
        writer->writeAttribute(attributeName(Attribute::Split::ID), tagIdList[i]);
        writer->writeEndElement();
    }

    if (split.isMatched()) {
        QString xml;
        QXmlStreamWriter matchWriter(&xml);
        matchWriter.setAutoFormattingIndent(0);
        matchWriter.setAutoFormatting(true);
        matchWriter.writeDTD(QLatin1String("<!DOCTYPE MATCH>"));
        matchWriter.writeStartElement(elementName(Element::Split::Container));
        writeTransaction(&matchWriter, split.matchedTransaction());
        matchWriter.writeEndElement();
        xml.replace(QLatin1String("<"), QLatin1String("&#60;"));
        split.setValue(attributeName(Attribute::Split::KMMatchedTx), xml);
    } else {
        split.deletePair(attributeName(Attribute::Split::KMMatchedTx));
    }

    writeKeyValueContainer(writer, split);

    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeTransaction(QXmlStreamWriter* writer, const MyMoneyTransaction& transaction)
{
    writeStartObject(writer, nodeName(Node::Transaction), transaction);

    writer->writeAttribute(attributeName(Attribute::Transaction::PostDate), MyMoneyUtils::dateToString(transaction.postDate()));
    writer->writeAttribute(attributeName(Attribute::Transaction::Memo), transaction.memo());
    writer->writeAttribute(attributeName(Attribute::Transaction::EntryDate), MyMoneyUtils::dateToString(transaction.entryDate()));
    writer->writeAttribute(attributeName(Attribute::Transaction::Commodity), transaction.commodity());

    writer->writeStartElement(elementName(Element::Transaction::Splits));
    for (const auto& split : transaction.splits()) {
        writeSplit(writer, split);
    }
    writer->writeEndElement();

    writeKeyValueContainer(writer, transaction);

    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeTransactions()
{
    m_writer->writeStartElement(tagName(Tag::Transactions));

    const auto model = m_file->journalModel();
    const auto rows = model->rowCount();

    for (int row = 0; row < rows;) {
        const auto idx = model->index(row, 0);
        const auto journalEntry = model->itemByIndex(idx);
        const auto transaction = journalEntry.transaction();
        writeTransaction(m_writer, transaction);
        row += transaction.splitCount();
    }

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeSchedule(QXmlStreamWriter* writer, const MyMoneySchedule& schedule)
{
    writeStartObject(writer, nodeName(Node::ScheduleTX), schedule);

    writer->writeAttribute(attributeName(Attribute::Schedule::Name), schedule.name());
    writer->writeAttribute(attributeName(Attribute::Schedule::Type), attrValue(static_cast<int>(schedule.type())));
    writer->writeAttribute(attributeName(Attribute::Schedule::Occurrence), attrValue(static_cast<int>(schedule.occurrence())));
    writer->writeAttribute(attributeName(Attribute::Schedule::OccurrenceMultiplier), attrValue(schedule.occurrenceMultiplier()));
    writer->writeAttribute(attributeName(Attribute::Schedule::PaymentType), attrValue(static_cast<int>(schedule.paymentType())));
    writer->writeAttribute(attributeName(Attribute::Schedule::StartDate), MyMoneyUtils::dateToString(schedule.startDate()));
    writer->writeAttribute(attributeName(Attribute::Schedule::EndDate), MyMoneyUtils::dateToString(schedule.endDate()));
    writer->writeAttribute(attributeName(Attribute::Schedule::Fixed), attrValue(schedule.isFixed()));
    writer->writeAttribute(attributeName(Attribute::Schedule::LastDayInMonth), attrValue(schedule.lastDayInMonth()));
    writer->writeAttribute(attributeName(Attribute::Schedule::AutoEnter), attrValue(schedule.autoEnter()));
    writer->writeAttribute(attributeName(Attribute::Schedule::LastPayment), MyMoneyUtils::dateToString(schedule.lastPayment()));
    writer->writeAttribute(attributeName(Attribute::Schedule::WeekendOption), attrValue(static_cast<int>(schedule.weekendOption())));

    writeKeyValueContainer(writer, schedule);

    // store the payment history for this schedule.
    // ipwizard: i am not sure if this is used at all
    QList<QDate> payments = schedule.recordedPayments();
    writer->writeStartElement(elementName(Element::Schedule::Payments));
    for (const auto date : payments) {
        writer->writeStartElement(elementName(Element::Schedule::Payment));
        writer->writeAttribute(attributeName(Attribute::Schedule::Date), MyMoneyUtils::dateToString(date));
        writer->writeEndElement();
    }
    writer->writeEndElement();

    // store the transaction data for this schedule.
    writeTransaction(writer, schedule.transaction());

    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeSchedules()
{
    m_writer->writeStartElement(tagName(Tag::Schedules));

    auto list = m_file->scheduleList();

    // we save the schedules ordered by id
    // allowing to compare the file contents before and after write
    std::sort(list.begin(), list.end(), [](const MyMoneySchedule& t1, const MyMoneySchedule& t2) {
        return t1.id() < t2.id();
    });

    for (const auto& schedule : list) {
        writeSchedule(m_writer, schedule);
    }

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeSecurity(const MyMoneySecurity& security, QXmlStreamWriter* writer)
{
    if (security.isCurrency())
        writeStartObject(writer, nodeName(Node::Currency), security);
    else
        writeStartObject(writer, nodeName(Node::Security), security);

    writer->writeAttribute(attributeName(Attribute::Security::Name), security.name());
    writer->writeAttribute(attributeName(Attribute::Security::Symbol), security.tradingSymbol());
    writer->writeAttribute(attributeName(Attribute::Security::Type), attrValue(static_cast<int>(security.securityType())));
    writer->writeAttribute(attributeName(Attribute::Security::RoundingMethod), attrValue(static_cast<int>(security.roundingMethod())));
    writer->writeAttribute(attributeName(Attribute::Security::SAF), attrValue(security.smallestAccountFraction()));
    writer->writeAttribute(attributeName(Attribute::Security::PP), attrValue(security.pricePrecision()));
    if (security.isCurrency())
        writer->writeAttribute(attributeName(Attribute::Security::SCF), attrValue(security.smallestCashFraction()));
    else {
        writer->writeAttribute(attributeName(Attribute::Security::TradingCurrency), security.tradingCurrency());
        writer->writeAttribute(attributeName(Attribute::Security::TradingMarket), security.tradingMarket());
    }

    writeKeyValueContainer(writer, security);

    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeSecurities()
{
    m_writer->writeStartElement(tagName(Tag::Securities));

    SecuritiesModel::xmlWriter writer(&MyMoneyXmlWriterPrivate::writeSecurity, m_writer);
    m_file->securitiesModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeCurrencies()
{
    m_writer->writeStartElement(tagName(Tag::Currencies));

    SecuritiesModel::xmlWriter writer(&MyMoneyXmlWriterPrivate::writeSecurity, m_writer);
    m_file->currenciesModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writePrices()
{
    m_writer->writeStartElement(tagName(Tag::Prices));

    QString from;
    QString to;
    PriceModel* model = m_file->priceModel();
    auto const rows = model->rowCount();
    QModelIndex idx;

    bool needEndElement(false);

    for (auto row = 0; row < rows; ++row) {
        idx = model->index(row, 0);
        const auto entry = model->itemByIndex(idx);

        if ((entry.from() != from) || (entry.to() != to)) {
            if (needEndElement) {
                m_writer->writeEndElement();
            }

            try {
                const auto fromSecurity = m_file->security(entry.from());
                const auto toSecurity = m_file->security(entry.to());
                if (fromSecurity.isCurrency() && !toSecurity.isCurrency()) {
                    qDebug() << QStringLiteral("The currency pair %1->%2 is invalid (from currency to equity). Omitting from storage.")
                                    .arg(entry.from(), entry.to());
                    continue;
                }
            } catch (MyMoneyException&) {
                qDebug() << QStringLiteral("The currency pair %1->%2 is invalid. Omitting from storage.").arg(entry.from(), entry.to());
                continue;
            }

            m_writer->writeStartElement(nodeName(Node::PricePair));
            m_writer->writeAttribute(attributeName(Attribute::General::From), entry.from());
            m_writer->writeAttribute(attributeName(Attribute::General::To), entry.to());
            needEndElement = true;
            from = entry.from();
            to = entry.to();
        }
        m_writer->writeStartElement(nodeName(Node::Price));
        m_writer->writeAttribute(attributeName(Attribute::General::Date), MyMoneyUtils::dateToString(entry.date()));
        m_writer->writeAttribute(attributeName(Attribute::General::Price), entry.rate(QString()).toString());
        m_writer->writeAttribute(attributeName(Attribute::General::Source), entry.source());
        m_writer->writeEndElement();
    }

    if (needEndElement) {
        m_writer->writeEndElement();
    }

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeReports()
{
    m_writer->writeStartElement(tagName(Tag::Reports));

    ReportsModel::xmlWriter writer(&MyMoneyXmlHelper::writeReport, m_writer);
    m_file->reportsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeBudgets()
{
    m_writer->writeStartElement(tagName(Tag::Budgets));

    BudgetsModel::xmlWriter writer(&MyMoneyXmlHelper::writeBudget, m_writer);
    m_file->budgetsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeOnlineJob(const onlineJob& job, QXmlStreamWriter* writer)
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
        void();
    }

    writer->writeStartElement(elementName(Element::OnlineJob::OnlineTask));
    writer->writeAttribute(attributeName(Attribute::OnlineJob::IID), job.taskIid());
    try {
        job.task()->writeXML(writer); // throws exception if there is no task
    } catch (const onlineJob::emptyTask&) {
    }
    writer->writeEndElement();

    writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeOnlineJobs()
{
    m_writer->writeStartElement(tagName(Tag::OnlineJobs));

    OnlineJobsModel::xmlWriter writer(&MyMoneyXmlWriterPrivate::writeOnlineJob, m_writer);
    m_file->onlineJobsModel()->processItems(&writer);

    m_writer->writeEndElement();
}

void MyMoneyXmlWriterPrivate::writeKMyMoney()
{
    writeFileInformation();
    writeUserInformation();
    writeInstitutions();
    writePayees();
    writeCostCenters();
    writeTags();
    writeAccounts();
    writeTransactions();
    writeKeyValueContainer(m_writer, m_file->parametersModel()->pairs());
    writeSchedules();
    writeSecurities();
    writeCurrencies();
    writePrices();
    writeReports();
    writeBudgets();
    writeOnlineJobs();
}

bool MyMoneyXmlWriterPrivate::write(QIODevice* device)
{
    m_writer->setDevice(device);
    m_writer->setAutoFormatting(true);
    m_writer->setAutoFormattingIndent(2);
    m_writer->writeStartDocument();
    m_writer->writeDTD(QLatin1String("<!DOCTYPE KMYMONEY-FILE>"));
    m_writer->writeStartElement(QLatin1String("KMYMONEY-FILE"));

    writeKMyMoney();

    m_writer->writeEndElement();
    m_writer->writeEndDocument();
    return !m_writer->hasError();
}

MyMoneyXmlWriter::MyMoneyXmlWriter()
    : d_ptr(new MyMoneyXmlWriterPrivate)
{
}

MyMoneyXmlWriter::MyMoneyXmlWriter(MyMoneyXmlWriterPrivate* dd)
    : d_ptr(dd)
{
}

MyMoneyXmlWriter::~MyMoneyXmlWriter()
{
    delete d_ptr;
}

void MyMoneyXmlWriter::setFile(MyMoneyFile* file)
{
    Q_D(MyMoneyXmlWriter);
    Q_ASSERT(file != nullptr);

    d->m_file = file;
}

QString MyMoneyXmlWriter::errorString() const
{
#if 0
    Q_D(const MyMoneyXmlWriter);
    if (d->m_writer->hasError()) {
        return d->m_writer->errorString();
    }
#endif
    return {};
}

bool MyMoneyXmlWriter::write(QIODevice* device)
{
    Q_D(MyMoneyXmlWriter);
    Q_ASSERT(d->m_file != nullptr);
    Q_ASSERT(device->isOpen());

    return d->write(device);
}
