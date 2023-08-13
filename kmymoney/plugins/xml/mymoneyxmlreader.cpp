/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

#include <QColor>
#include <QDate>
#include <QMap>
#include <QSharedPointer>
#include <QXmlStreamReader>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ibanbic/ibanbic.h"
#include "mymoneyaccount.h"
#include "mymoneybudget.h"
#include "mymoneycostcenter.h"
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
#include "mymoneyxmlreader.h"
#include "nationalaccount/nationalaccount.h"
#include "onlinejobadministration.h"
#include "payeeidentifier.h"
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

class MyMoneyXmlReaderPrivate
{
public:
    typedef struct {
        QString street;
        QString city;
        QString state;
        QString zipcode;
        QString telephone;
    } Address;

    QXmlStreamReader* m_reader;
    MyMoneyFile* m_file = nullptr;

    QMap<QString, QString> m_fileInformation;
    MyMoneyKeyValueContainer m_kvp;
    MyMoneyInstitution m_institution;
    MyMoneyAccount m_account;
    MyMoneyPayee m_payee;
    MyMoneyCostCenter m_costCenter;
    MyMoneyTag m_tag;
    MyMoneyTransaction m_transaction;
    MyMoneySplit m_split;
    MyMoneySchedule m_schedule;
    MyMoneySecurity m_security;
    MyMoneyPrice m_price;
    onlineJob m_onlineJob;

    uint m_fileVersionRead;

    MyMoneyXmlReaderPrivate()
        : m_reader(new QXmlStreamReader)
    {
    }

    ~MyMoneyXmlReaderPrivate()
    {
        delete m_reader;
    }

    inline MyMoneyMoney readValueAttribute(const QString& attribute) const
    {
        return MyMoneyXmlHelper::readValueAttribute(m_reader, attribute);
    }

    inline QDate readDateAttribute(const QString& attribute) const
    {
        return MyMoneyXmlHelper::readDateAttribute(m_reader, attribute);
    }

    inline QDateTime readDateTimeAttribute(const QString& attribute) const
    {
        return MyMoneyXmlHelper::readDateTimeAttribute(m_reader, attribute);
    }

    inline QString readRequiredStringAttribute(const QString& attribute) const
    {
        return MyMoneyXmlHelper::readRequiredStringAttribute(m_reader, attribute);
    }

    inline QString readStringAttribute(const QString& attribute) const
    {
        return MyMoneyXmlHelper::readStringAttribute(m_reader, attribute);
    }

    inline bool readBoolAttribute(const QString& attribute, bool defaultValue = false) const
    {
        return MyMoneyXmlHelper::readBoolAttribute(m_reader, attribute, defaultValue);
    }

    inline uint readUintAttribute(const QString& attribute, uint defaultValue = 0, int base = 0) const
    {
        return MyMoneyXmlHelper::readUintAttribute(m_reader, attribute, defaultValue, base);
    }

    inline QString readId(MyMoneyXmlHelper::IdRequirement idRequirement = MyMoneyXmlHelper::IdRequirement::Required) const
    {
        return MyMoneyXmlHelper::readId(m_reader, idRequirement);
    }

    Address readAddress() const
    {
        Address rc;
        rc.street = readStringAttribute(attributeName(Attribute::General::Street));
        rc.city = readStringAttribute(attributeName(Attribute::General::City));

        // there are multiple identifiers used for the state in different
        // locations of the KMM file. We try to read all until we find an
        // entry to maintain backward compatability
        rc.state = readStringAttribute(attributeName(Attribute::General::State));
        if (rc.state.isEmpty()) {
            rc.state = readStringAttribute(attributeName(Attribute::General::Country));
        }
        if (rc.state.isEmpty()) {
            rc.state = readStringAttribute(attributeName(Attribute::General::AltCountry));
        }

        // there are multiple identifiers used for the zipcode in different
        // locations of the KMM file. We try to read all until we find an
        // entry to maintain backward compatability
        rc.zipcode = readStringAttribute(attributeName(Attribute::General::ZipCode));
        if (rc.zipcode.isEmpty()) {
            rc.zipcode = readStringAttribute(attributeName(Attribute::General::AltZipCode));
        }
        if (rc.zipcode.isEmpty()) {
            rc.zipcode = readStringAttribute(attributeName(Attribute::General::PostCode));
        }

        rc.telephone = readStringAttribute(attributeName(Attribute::General::Telephone));
        return rc;
    }

    QStringList readAccountIds(const QStringView containerTagName, const QString& tagName)
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == containerTagName));

        QStringList result;
        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == tagName) {
                result.append(readStringAttribute(attributeName(Attribute::General::ID)));
            }
            m_reader->skipCurrentElement();
        }
        return result;
    }

    bool read(QIODevice* device)
    {
        m_reader->setDevice(device);

        if (m_reader->readNextStartElement()) {
            if (m_reader->name() == tagName(Tag::KMMFile)) {
                readKMyMoney();
            }
        }
        return !m_reader->hasError();
    }

    void readFileInfo()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::FileInfo)));

        m_fileInformation.clear();
        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (tag == elementName(Element::General::CreationDate)) {
                const auto strDate = readStringAttribute(attributeName(Attribute::General::Date));
                if (MyMoneyUtils::stringToDate(strDate).isValid()) {
                    m_fileInformation.insert(m_file->fixedKey(MyMoneyFile::CreationDate), strDate);
                }

            } else if (tag == elementName(Element::General::LastModifiedDate)) {
                const auto strDate = readStringAttribute(attributeName(Attribute::General::Date));
                if (MyMoneyUtils::stringToDate(strDate).isValid()) {
                    m_fileInformation.insert(m_file->fixedKey(MyMoneyFile::LastModificationDate), strDate);
                }

            } else if (tag == elementName(Element::General::Version)) {
                m_fileVersionRead = readUintAttribute(attributeName(Attribute::General::ID), 0, 16);

            } else if (tag == elementName(Element::General::FixVersion)) {
                auto strFixVersion = readUintAttribute(attributeName(Attribute::General::ID));
                // skip KMyMoneyView::fixFile_2()
                if (strFixVersion == 2)
                    strFixVersion = 3;
                m_fileInformation.insert(m_file->fixedKey(MyMoneyFile::FileFixVersion), QString("%1").arg(strFixVersion));

            }
            m_reader->skipCurrentElement();
        }
    }

    void readUser()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::User)));

        MyMoneyPayee user = MyMoneyPayee(m_file->fixedKey(MyMoneyFile::UserID), MyMoneyPayee());
        user.setName(readStringAttribute(attributeName(Attribute::General::Name)));
        user.setEmail(readStringAttribute(attributeName(Attribute::General::Email)));

        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == elementName(Element::General::Address)) {
                const auto address = readAddress();
                user.setAddress(address.street);
                user.setCity(address.city);
                user.setState(address.state);
                user.setPostcode(address.zipcode);
                user.setTelephone(address.telephone);

                m_file->userModel()->unload();
                m_file->userModel()->addItem(user);
                // loading does not count as making dirty
                m_file->userModel()->setDirty(false);

            }
            m_reader->skipCurrentElement();
        }
    }

    void readInstitution()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == nodeName(Node::Institution)));

        m_institution = MyMoneyInstitution(readId());
        m_institution.setBankCode(readStringAttribute(attributeName(Attribute::Institution::BankCode)));
        m_institution.setName(readStringAttribute(attributeName(Attribute::Institution::Name)));
        m_institution.setManager(readStringAttribute(attributeName(Attribute::Institution::Manager)));

        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (tag == elementName(Element::Institution::Address)) {
                auto address = readAddress();
                m_institution.setStreet(address.street);
                m_institution.setCity(address.city);
                m_institution.setPostcode(address.zipcode);
                m_institution.setTelephone(address.telephone);
                m_reader->skipCurrentElement();
            } else if (tag == elementName(Element::Institution::AccountIDS)) {
                const auto accountIds = readAccountIds(tag, elementName(Element::Institution::AccountID));
                for (const auto& id : accountIds) {
                    m_institution.addAccountId(id);
                }
            } else if (tag == nodeName(Node::KeyValuePairs)) {
                readKeyValueContainer();
                m_institution.setPairs(m_kvp.pairs());
            } else {
                m_reader->skipCurrentElement();
            }
        }
    }

    void readInstitutions()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Institutions)));

        QMap<QString, MyMoneyInstitution> institutions;
        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::Institution)) {
                readInstitution();
                if (!m_reader->hasError()) {
                    institutions[m_institution.id()] = m_institution;
                }
            } else {
                m_reader->skipCurrentElement();
            }
        }
        if (!institutions.isEmpty()) {
            m_file->institutionsModel()->load(institutions);
        }
    }

    payeeIdentifierData* readPayeeIdentifier()
    {
        const auto identifierType = readStringAttribute(attributeName(Attribute::Payee::Type));

        payeeIdentifierData* identData = nullptr;

        if (identifierType == payeeIdentifiers::ibanBic::staticPayeeIdentifierIid()) {
            identData = readIBANBIC();
        } else if (identifierType == payeeIdentifiers::nationalAccount::staticPayeeIdentifierIid()) {
            identData = readNationalAccount();
        } else {
            m_reader->raiseError(i18nc("Error message when reading XML file", "Unknown payee identifier type %1").arg(identifierType));
        }

        return identData;
    }

    payeeIdentifierData* readIBANBIC()
    {
        payeeIdentifiers::ibanBic* const ident = new payeeIdentifiers::ibanBic;

        ident->setBic(readStringAttribute(attributeName(Attribute::Payee::BIC)));
        ident->setIban(readStringAttribute(attributeName(Attribute::Payee::IBAN)));
        ident->setOwnerName(readStringAttribute(attributeName(Attribute::Payee::OwnerVer1)));
        return ident;
    }

    payeeIdentifierData* readNationalAccount()
    {
        payeeIdentifiers::nationalAccount* const ident = new payeeIdentifiers::nationalAccount;

        ident->setBankCode(readStringAttribute(attributeName(Attribute::Payee::BankCode)));
        ident->setAccountNumber(readStringAttribute(attributeName(Attribute::Payee::AccountNumber)));
        ident->setOwnerName(readStringAttribute(attributeName(Attribute::Payee::OwnerVer2)));
        ident->setCountry(readStringAttribute(attributeName(Attribute::Payee::Country)));
        return ident;
    }

    void readPayee()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == nodeName(Node::Payee)));

        m_payee = MyMoneyPayee(readId());

        m_payee.setName(readStringAttribute(attributeName(Attribute::Payee::Name)));
        m_payee.setReference(readStringAttribute(attributeName(Attribute::Payee::Reference)));
        m_payee.setEmail(readStringAttribute(attributeName(Attribute::Payee::Email)));

        m_payee.setMatchData(eMyMoney::Payee::MatchType::Disabled, true, QString());
        auto type = static_cast<eMyMoney::Payee::MatchType>(readUintAttribute(attributeName(Attribute::Payee::MatchingEnabled), 0));
        if (type != eMyMoney::Payee::MatchType::Disabled) {
            type = readUintAttribute(attributeName(Attribute::Payee::UsingMatchKey)) != 0 ? eMyMoney::Payee::MatchType::Key : eMyMoney::Payee::MatchType::Name;
            const auto ignoreCase = readUintAttribute(attributeName(Attribute::Payee::MatchIgnoreCase));
            const auto matchKey = readStringAttribute(attributeName(Attribute::Payee::MatchKey));
            m_payee.setMatchData(type, ignoreCase, matchKey);
        }

        m_payee.setNotes(readStringAttribute(attributeName(Attribute::Payee::Notes)));
        m_payee.setDefaultAccountId(readStringAttribute(attributeName(Attribute::Payee::DefaultAccountID)));

        int identifierIndex = 1;
        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (tag == elementName(Element::Payee::Address)) {
                auto address = readAddress();
                m_payee.setAddress(address.street);
                m_payee.setCity(address.city);
                m_payee.setState(address.state);
                m_payee.setPostcode(address.zipcode);
                m_payee.setTelephone(address.telephone);
            } else if (tag == elementName(Element::Payee::Identifier)) {
                const auto identifierData = readPayeeIdentifier();
                if (identifierData) {
                    m_payee.addPayeeIdentifier(payeeIdentifier(identifierIndex, identifierData));
                    ++identifierIndex;
                }
            }
            m_reader->skipCurrentElement();
        }
    }

    void readPayees()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Payees)));

        QMap<QString, MyMoneyPayee> payees;
        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::Payee)) {
                readPayee();
                if (!m_reader->hasError()) {
                    payees[m_payee.id()] = m_payee;
                }
            } else {
                m_reader->skipCurrentElement();
            }
        }

        if (!payees.isEmpty()) {
            m_file->payeesModel()->load(payees);
        }
    }

    void readCostCenter()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == nodeName(Node::CostCenter)));

        m_costCenter = MyMoneyCostCenter(readId());
        m_costCenter.setName(readStringAttribute(attributeName(Attribute::CostCenter::Name)));
        m_reader->skipCurrentElement();
    }

    void readCostCenters()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::CostCenters)));

        QMap<QString, MyMoneyCostCenter> costCenters;
        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::CostCenter)) {
                readCostCenter();
                if (!m_reader->hasError()) {
                    costCenters[m_costCenter.id()] = m_costCenter;
                }

            } else {
                m_reader->skipCurrentElement();
            }
        }
        if (!costCenters.isEmpty()) {
            m_file->costCenterModel()->load(costCenters);
        }
    }

    void readTag()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == nodeName(Node::Tag)));

        m_tag = MyMoneyTag(readId());
        m_tag.setName(readStringAttribute(attributeName(Attribute::Tag::Name)));
        const auto color = readStringAttribute(attributeName(Attribute::Tag::TagColor));
        if (!color.isEmpty()) {
            m_tag.setTagColor(QColor(color));
        }
        m_tag.setNotes(readStringAttribute(attributeName(Attribute::Tag::Notes)));
        m_tag.setClosed(readBoolAttribute(attributeName(Attribute::Tag::Closed)));
        m_reader->skipCurrentElement();
    }

    void readTags()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Tags)));

        QMap<QString, MyMoneyTag> tags;
        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::Tag)) {
                readTag();
                if (!m_reader->hasError()) {
                    tags[m_tag.id()] = m_tag;
                }
            } else {
                m_reader->skipCurrentElement();
            }
        }
        if (!tags.isEmpty()) {
            m_file->tagsModel()->load(tags);
        }
    }

    void readReconciliationHistory()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == elementName(Element::Account::ReconciliationHistory)));

        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == elementName(Element::Account::ReconciliationEntry)) {
                const auto date = readDateAttribute(attributeName(Attribute::Reconciliation::Date));
                const auto balance = MyMoneyMoney(readStringAttribute(attributeName(Attribute::Reconciliation::Amount)));
                m_account.addReconciliation(date, balance);
            }
            m_reader->skipCurrentElement();
        }
    }

    void readOnlineBanking()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == elementName(Element::Account::OnlineBanking)));

        m_kvp.clear();
        for (const auto& attribute : m_reader->attributes()) {
            attribute.name();
            const auto key = attribute.name().toString();
            const auto value = attribute.value().toString();
            m_kvp.setValue(key, value);
        }
        // TODO: these should be moved to some upgradeVXX() function, akin to mymoneystoragesql
        // Up to and including 4.8 the OFX importer plugin was called "KMyMoney OFX"
        // From version 5 on it is called "ofximporter". So we update it here in
        // case we find the 'old' name
        if (m_kvp.value(QStringLiteral("provider")).toLower().compare(QLatin1String("kmymoney ofx")) == 0) {
            m_kvp.setValue(QStringLiteral("provider"), QStringLiteral("ofximporter"));
        }

        // Up to and including 5.1.2, the Woob plugin was called "Weboob"
        // From version 5.1.3 on it is called "Woob". So we update it here in
        // case we find the 'old' name
        if (m_kvp.value(QStringLiteral("provider")).toLower().compare(QLatin1String("weboob")) == 0) {
            m_kvp.setValue(QStringLiteral("provider"), QStringLiteral("woob"));
        }
        m_reader->skipCurrentElement();
    }

    void readAccount()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == nodeName(Node::Account)));

        m_account = MyMoneyAccount(readId());
        m_account.setName(readStringAttribute(attributeName(Attribute::Account::Name)));
        m_account.setDescription(readStringAttribute(attributeName(Attribute::Account::Description)));
        m_account.setParentAccountId(readStringAttribute(attributeName(Attribute::Account::ParentAccount)));
        m_account.setInstitutionId(readStringAttribute(attributeName(Attribute::Account::Institution)));
        m_account.setNumber(readStringAttribute(attributeName(Attribute::Account::Number)));
        m_account.setCurrencyId(readStringAttribute(attributeName(Attribute::Account::Currency)));

        m_account.setLastModified(readDateAttribute(attributeName(Attribute::Account::LastModified)));
        m_account.setLastReconciliationDate(readDateAttribute(attributeName(Attribute::Account::LastReconciled)));
        m_account.setOpeningDate(readDateAttribute(attributeName(Attribute::Account::Opened)));

        auto tmp = readStringAttribute(attributeName(Attribute::Account::Type));
        auto bOK = false;
        auto type = tmp.toInt(&bOK);
        if (bOK) {
            m_account.setAccountType(static_cast<eMyMoney::Account::Type>(type));
        } else {
            qWarning("XMLREADER: Account %s had invalid or no account type information.", qPrintable(m_account.name()));
        }

        bool haveNewReconciliationHistory = false;
        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (tag == elementName(Element::Account::SubAccounts)) {
                const auto accountIds = readAccountIds(tag, elementName(Element::Account::SubAccount));
                for (const auto& id : accountIds) {
                    m_account.addAccountId(id);
                }

            } else if (tag == nodeName(Node::KeyValuePairs)) {
                readKeyValueContainer();
                m_account.setPairs(m_kvp.pairs());
                // Very old versions of KMyMoney used to store the reconciliation date in
                // the KVP as "lastStatementDate". Since we don't use it anymore, we get
                // rid of it in case we read such an old file.
                m_account.deletePair(QStringLiteral("lastStatementDate"));

            } else if (tag == elementName(Element::Account::OnlineBanking)) {
                readOnlineBanking();
                m_account.setOnlineBankingSettings(m_kvp);

            } else if (tag == elementName(Element::Account::ReconciliationHistory)) {
                haveNewReconciliationHistory = true;
                readReconciliationHistory();

            } else {
                m_reader->skipCurrentElement();
            }
        }

        if (!haveNewReconciliationHistory) {
            /// @todo remove old reconciliation history storage method
            /// this part can be removed when the reconciliation
            /// history is not stored in the KVP anymore
            // force loading map in any case
            m_account.reconciliationHistory();
        }

        // Up to and including version 4.6.6 the new account dialog stored the iban in the kvp-key "IBAN".
        // But the rest of the software uses "iban". So correct this:
        if (!m_account.value("IBAN").isEmpty()) {
            // If "iban" was not set, set it now. If it is set, the user reset it already, so remove
            // the garbage.
            if (m_account.value(attributeName(Attribute::Account::IBAN)).isEmpty()) {
                m_account.setValue(attributeName(Attribute::Account::IBAN), m_account.value("IBAN"));
            }
            m_account.deletePair("IBAN");
        }
    }

    void readAccounts()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Accounts)));

        QMap<QString, MyMoneyAccount> accounts;
        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::Account)) {
                readAccount();
                if (!m_reader->hasError()) {
                    accounts[m_account.id()] = m_account;
                }

            } else {
                m_reader->skipCurrentElement();
            }
        }
        if (!accounts.isEmpty()) {
            m_file->accountsModel()->load(accounts);
        }
    }

    void readSplit()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == elementName(Element::Transaction::Split)));

        m_split = MyMoneySplit();

        m_split.setPayeeId(readStringAttribute(attributeName(Attribute::Split::Payee)));
        m_split.setReconcileDate(readDateAttribute(attributeName(Attribute::Split::ReconcileDate)));
        m_split.setAction(readStringAttribute(attributeName(Attribute::Split::Action)));
        m_split.setReconcileFlag(static_cast<eMyMoney::Split::State>(readUintAttribute(attributeName(Attribute::Split::ReconcileFlag))));
        m_split.setMemo(readStringAttribute(attributeName(Attribute::Split::Memo)));
        m_split.setValue(readValueAttribute(attributeName(Attribute::Split::Value)));
        m_split.setShares(readValueAttribute(attributeName(Attribute::Split::Shares)));
        m_split.setPrice(readValueAttribute(attributeName(Attribute::Split::Price)));
        m_split.setAccountId(readStringAttribute(attributeName(Attribute::Split::Account)));
        m_split.setCostCenterId(readStringAttribute(attributeName(Attribute::Split::CostCenter)));
        m_split.setNumber(readStringAttribute(attributeName(Attribute::Split::Number)));
        m_split.setBankID(readStringAttribute(attributeName(Attribute::Split::BankID)));

        QList<QString> tagList;
        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (tag == elementName(Element::Split::Tag)) {
                tagList << readStringAttribute(attributeName(Attribute::Split::ID));
                m_reader->skipCurrentElement();

            } else if (tag == nodeName(Node::KeyValuePairs)) {
                readKeyValueContainer();
                m_split.setPairs(m_kvp.pairs());

            } else {
                m_reader->skipCurrentElement();
            }
        }
        m_split.setTagIdList(tagList);

        // take care of matched transaction

        auto xml = m_split.value(attributeName(Attribute::Split::KMMatchedTx));
        if (!xml.isEmpty()) {
            // save some data which will be otherwise
            // overwritten in the next steps since the
            // routines are called recursively.
            const auto savedReader = m_reader;
            const auto savedTransaction = m_transaction;
            const auto savedSplit = m_split;

            // determine between the new and old method to escap the less than symbol
            if (xml.contains(QLatin1String("&#60;"))) {
                xml.replace(QLatin1String("&#60;"), QLatin1String("<"));
            } else {
                xml.replace(QLatin1String("&lt;"), QLatin1String("<"));
            }

            m_reader = new QXmlStreamReader(xml);
            while (m_reader->readNextStartElement()) {
                const auto tag = m_reader->name();
                if (tag == elementName(Element::Split::Container)) {
                    m_reader->readNextStartElement();
                    readTransaction(false, MyMoneyXmlHelper::IdRequirement::Optional);
                    m_split.addMatch(m_transaction);
                } else {
                    m_reader->skipCurrentElement();
                }
            }

            // get back the split we are working on, assign
            // the matched transaction and get back the
            // transaction the split belongs to.
            m_split = savedSplit;
            m_split.addMatch(m_transaction);
            m_transaction = savedTransaction;
            delete m_reader;
            m_reader = savedReader;
        }
    }

    void readSplits()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == elementName(Element::Transaction::Splits)));

        QMap<QString, MyMoneyTransaction> transactions;
        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (m_reader->name() == elementName(Element::Transaction::Split)) {
                readSplit();
                if (!m_reader->hasError()) {
                    if (m_split.accountId().isEmpty()) {
                        qDebug() << "Dropped split because it did not have an account id";
                    } else {
                        m_transaction.addSplit(m_split);
                    }
                }

            } else {
                m_reader->skipCurrentElement();
            }
        }
    }

    void readTransaction(bool assignEntryDateIfEmpty = true, MyMoneyXmlHelper::IdRequirement idRequirement = MyMoneyXmlHelper::IdRequirement::Required)
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == nodeName(Node::Transaction)));

        m_transaction = MyMoneyTransaction(readId(idRequirement));

        m_transaction.setPostDate(readDateAttribute(attributeName(Attribute::Transaction::PostDate)));

        auto entryDate = readDateAttribute(attributeName(Attribute::Transaction::EntryDate));
        if (!entryDate.isValid() && assignEntryDateIfEmpty)
            entryDate = QDate::currentDate();
        m_transaction.setEntryDate(entryDate);
        m_transaction.setMemo(readStringAttribute(attributeName(Attribute::Transaction::Memo)));
        m_transaction.setCommodity(readStringAttribute(attributeName(Attribute::Transaction::Commodity)));

        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (m_reader->name() == elementName(Element::Transaction::Splits)) {
                readSplits();

            } else if (tag == nodeName(Node::KeyValuePairs)) {
                readKeyValueContainer();
                m_transaction.setPairs(m_kvp.pairs());

            } else {
                m_reader->skipCurrentElement();
            }
        }
    }

    void readTransactions()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Transactions)));

        QMap<QString, QSharedPointer<MyMoneyTransaction>> transactions;
        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (m_reader->name() == nodeName(Node::Transaction)) {
                readTransaction();
                if (!m_reader->hasError()) {
                    transactions[m_transaction.uniqueSortKey()] = QSharedPointer<MyMoneyTransaction>(new MyMoneyTransaction(m_transaction));
                }

            } else {
                m_reader->skipCurrentElement();
            }
        }
        if (!transactions.isEmpty()) {
            m_file->journalModel()->load(transactions);
        }
    }

    void readSchedule()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == nodeName(Node::ScheduleTX)));

        m_schedule = MyMoneySchedule(readId());
        m_schedule.setName(readStringAttribute(attributeName(Attribute::Schedule::Name)));
        m_schedule.setStartDate(readDateAttribute(attributeName(Attribute::Schedule::StartDate)));
        m_schedule.setEndDate(readDateAttribute(attributeName(Attribute::Schedule::EndDate)));
        m_schedule.setLastPayment(readDateAttribute(attributeName(Attribute::Schedule::LastPayment)));
        m_schedule.setType(static_cast<eMyMoney::Schedule::Type>(readUintAttribute(attributeName(Attribute::Schedule::Type))));
        m_schedule.setPaymentType(static_cast<eMyMoney::Schedule::PaymentType>(readUintAttribute(attributeName(Attribute::Schedule::PaymentType))));

        auto occurrence = static_cast<eMyMoney::Schedule::Occurrence>(readUintAttribute(attributeName(Attribute::Schedule::Occurrence)));
        int multiplier = readUintAttribute(attributeName(Attribute::Schedule::OccurrenceMultiplier), 1);
        m_schedule.simpleToCompoundOccurrence(multiplier, occurrence);
        m_schedule.setOccurrencePeriod(occurrence);
        m_schedule.setOccurrenceMultiplier(multiplier);
        m_schedule.setLastDayInMonth(static_cast<bool>(readUintAttribute("lastDayInMonth")));
        m_schedule.setAutoEnter(readBoolAttribute(attributeName(Attribute::Schedule::AutoEnter)));
        m_schedule.setFixed(readBoolAttribute(attributeName(Attribute::Schedule::Fixed)));
        m_schedule.setWeekendOption(static_cast<eMyMoney::Schedule::WeekendOption>(readUintAttribute(attributeName(Attribute::Schedule::WeekendOption))));

        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (tag == nodeName(Node::Transaction)) {
                readTransaction(false, MyMoneyXmlHelper::IdRequirement::Optional);
                // some old versions did not remove the entry date and post date fields
                // in the schedule. So if this is the case, we deal with a very old transaction
                // and can't use the post date field as next due date. Hence, we wipe it out here
                if (m_transaction.entryDate().isValid()) {
                    m_transaction.setPostDate(QDate());
                    m_transaction.setEntryDate(QDate());
                }
                m_schedule.setTransaction(m_transaction, true);

            } else if (tag == elementName(Element::Schedule::Payments)) {
                /// @todo check if this code is actually used or not
                /// I have not found any trace in my data of its use
                const auto paymentDate = readDateAttribute(attributeName(Attribute::Schedule::Date));
                if (paymentDate.isValid()) {
                    m_schedule.recordPayment(paymentDate);
                }
                m_reader->skipCurrentElement();

            } else if (tag == nodeName(Node::KeyValuePairs)) {
                readKeyValueContainer();
                m_schedule.setPairs(m_kvp.pairs());

            } else {
                m_reader->skipCurrentElement();
            }
        }

        // if the next due date is not set (comes from old version)
        // then set it up the old way
        if (!m_schedule.nextDueDate().isValid() && !m_schedule.lastPayment().isValid()) {
            auto t = m_schedule.transaction();
            t.setPostDate(m_schedule.startDate());
            m_schedule.setTransaction(t, true);
            // clear it, because the schedule has never been used
            m_schedule.setStartDate(QDate());
        }

        // There are reports that lastPayment and nextDueDate are identical or
        // that nextDueDate is older than lastPayment. This could
        // be caused by older versions of the application. In this case, we just
        // clear out the nextDueDate and let it calculate from the lastPayment.
        if (m_schedule.nextDueDate().isValid() && m_schedule.nextDueDate() <= m_schedule.lastPayment()) {
            auto t = m_schedule.transaction();
            t.setPostDate(QDate());
            m_schedule.setTransaction(t, true);
        }

        if (!m_schedule.nextDueDate().isValid()) {
            auto t = m_schedule.transaction();
            t.setPostDate(m_schedule.startDate());
            m_schedule.setTransaction(t, true);
            t = m_schedule.transaction();
            t.setPostDate(m_schedule.nextPayment(m_schedule.lastPayment().addDays(1)));
            m_schedule.setTransaction(t, true);
        }
    }

    void readSchedules()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Schedules)));

        QMap<QString, MyMoneySchedule> schedules;
        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::ScheduleTX)) {
                readSchedule();
                if (!m_reader->hasError()) {
                    schedules[m_schedule.id()] = m_schedule;
                }

            } else {
                m_reader->skipCurrentElement();
            }
        }
        if (!schedules.isEmpty()) {
            m_file->schedulesModel()->load(schedules);
        }
    }

    void readSecurity()
    {
        const auto validTag =
            (m_reader->name() == nodeName(Node::Security)) || (m_reader->name() == nodeName(Node::Currency)) || (m_reader->name() == nodeName(Node::Equity));
        Q_ASSERT(validTag && m_reader->isStartElement());

        m_security = MyMoneySecurity(readId());

        m_security.setName(readStringAttribute(attributeName(Attribute::Security::Name)));
        m_security.setTradingSymbol(readStringAttribute(attributeName(Attribute::Security::Symbol)));
        m_security.setSecurityType(static_cast<eMyMoney::Security::Type>(readUintAttribute(attributeName(Attribute::Security::Type))));
        m_security.setRoundingMethod(static_cast<AlkValue::RoundingMethod>(readUintAttribute(attributeName(Attribute::Security::RoundingMethod))));
        m_security.setSmallestAccountFraction(readUintAttribute(attributeName(Attribute::Security::SAF)));
        m_security.setPricePrecision(readUintAttribute(attributeName(Attribute::Security::PP)));

        if (m_security.smallestAccountFraction() == 0)
            m_security.setSmallestAccountFraction(100);
        if (m_security.pricePrecision() == 0 || m_security.pricePrecision() > 10)
            m_security.setPricePrecision(4);

        if (m_security.isCurrency()) {
            m_security.setSmallestCashFraction(readUintAttribute(attributeName(Attribute::Security::SCF)));
            if (m_security.smallestCashFraction() == 0)
                m_security.setSmallestCashFraction(100);
        } else {
            m_security.setTradingCurrency(readStringAttribute(attributeName(Attribute::Security::TradingCurrency)));
            m_security.setTradingMarket(readStringAttribute(attributeName(Attribute::Security::TradingMarket)));
        }

        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::KeyValuePairs)) {
                readKeyValueContainer();
                m_security.setPairs(m_kvp.pairs());
            } else {
                m_reader->skipCurrentElement();
            }
        }
    }

    void readSecurities()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Securities)));

        QMap<QString, MyMoneySecurity> securities;
        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::Security)) {
                readSecurity();
                if (!m_reader->hasError()) {
                    securities[m_security.id()] = m_security;
                }

            } else {
                m_reader->skipCurrentElement();
            }
        }
        if (!securities.isEmpty()) {
            m_file->securitiesModel()->load(securities);
        }
    }

    void readCurrencies()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Currencies)));

        QMap<QString, MyMoneySecurity> currencies;
        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::Currency)) {
                readSecurity();
                if (!m_reader->hasError()) {
                    currencies[m_security.id()] = m_security;
                }

            } else {
                m_reader->skipCurrentElement();
            }
        }
        if (!currencies.isEmpty()) {
            m_file->currenciesModel()->loadCurrencies(currencies);
        }
    }

    void readPricePair(QMap<MyMoneySecurityPair, MyMoneyPriceEntries>& prices)
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == nodeName(Node::PricePair)));

        const auto fromSecurity(readStringAttribute(attributeName(Attribute::General::From)));
        const auto toSecurity(readStringAttribute(attributeName(Attribute::General::To)));

        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::Price)) {
                const auto date = readDateAttribute(attributeName(Attribute::General::Date));
                const auto rate = readValueAttribute(attributeName(Attribute::General::Price));
                const auto source = readStringAttribute(attributeName(Attribute::General::Source));
                MyMoneyPrice price(fromSecurity, toSecurity, date, rate, source);
                prices[MyMoneySecurityPair(fromSecurity, toSecurity)][date] = price;

            }
            m_reader->skipCurrentElement();
        }
    }

    void readPrices()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Prices)));

        QMap<MyMoneySecurityPair, MyMoneyPriceEntries> prices;

        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::PricePair)) {
                readPricePair(prices);

            } else {
                m_reader->skipCurrentElement();
            }
        }

        if (!prices.isEmpty()) {
            m_file->priceModel()->load(prices);
        }
    }

    void readReports()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Reports)));

        QMap<QString, MyMoneyReport> reports;

        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::Report)) {
                const auto report = MyMoneyXmlHelper::readReport(m_reader);
                reports[report.id()] = report;

            } else {
                m_reader->skipCurrentElement();
            }
        }

        if (!reports.isEmpty()) {
            m_file->reportsModel()->load(reports);
        }
    }

    void readBudgets()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::Budgets)));

        QMap<QString, MyMoneyBudget> budgets;

        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::Budget)) {
                const auto budget = MyMoneyXmlHelper::readBudget(m_reader);
                budgets[budget.id()] = budget;

            } else {
                m_reader->skipCurrentElement();
            }
        }

        if (!budgets.isEmpty()) {
            m_file->budgetsModel()->load(budgets);
        }
    }

    void readOnlineJob()
    {
        m_onlineJob = onlineJob(readId());

        m_onlineJob.clearJobMessageList();
        m_onlineJob.setLock(false);
        m_onlineJob.setJobSend(readDateTimeAttribute(attributeName(Attribute::OnlineJob::Send)));
        const auto state = readStringAttribute(attributeName(Attribute::OnlineJob::BankAnswerState));
        const auto date = readDateTimeAttribute(attributeName(Attribute::OnlineJob::BankAnswerDate));
        if (state == attributeName(Attribute::OnlineJob::AbortedByUser)) {
            m_onlineJob.setBankAnswer(eMyMoney::OnlineJob::sendingState::abortedByUser, date);
        } else if (state == attributeName(Attribute::OnlineJob::AcceptedByBank)) {
            m_onlineJob.setBankAnswer(eMyMoney::OnlineJob::sendingState::acceptedByBank, date);
        } else if (state == attributeName(Attribute::OnlineJob::RejectedByBank)) {
            m_onlineJob.setBankAnswer(eMyMoney::OnlineJob::sendingState::rejectedByBank, date);
        } else if (state == attributeName(Attribute::OnlineJob::SendingError)) {
            m_onlineJob.setBankAnswer(eMyMoney::OnlineJob::sendingState::sendingError, date);
        } else {
            m_onlineJob.setBankAnswer(eMyMoney::OnlineJob::sendingState::noBankAnswer);
        }

        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == elementName(Element::OnlineJob::OnlineTask)) {
                m_onlineJob.setTask(
                    onlineJobAdministration::instance()->createOnlineTaskByXml(m_reader, readStringAttribute(attributeName(Attribute::OnlineJob::IID))));
            }
            m_reader->skipCurrentElement();
        }
    }

    void readOnlineJobs()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::OnlineJobs)));

        QMap<QString, onlineJob> onlineJobs;

        while (m_reader->readNextStartElement()) {
            if (m_reader->name() == nodeName(Node::OnlineJob)) {
                readOnlineJob();
                onlineJobs[m_onlineJob.id()] = m_onlineJob;

            } else {
                m_reader->skipCurrentElement();
            }
        }

        if (!onlineJobs.isEmpty()) {
            m_file->onlineJobsModel()->load(onlineJobs);
        }
    }

    void readKeyValueContainer()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::KeyValuePairs)));

        m_kvp.clear();
        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (tag == elementName(Element::General::Pair)) {
                const auto key = m_reader->attributes().value(attributeName(Attribute::General::Key)).toString();
                const auto value = m_reader->attributes().value(attributeName(Attribute::General::Value)).toString();
                m_kvp.setValue(key, value);
            }
            m_reader->skipCurrentElement();
        }
    }

    void readKeyValuePairs()
    {
        readKeyValueContainer();

        if (!m_reader->hasError()) {
            auto pairs = m_kvp.pairs();

            // add in the fileinformation so that it is contained
            // in the parametersModel
            pairs.insert(m_fileInformation);
            m_file->parametersModel()->load(m_kvp.pairs());

            // loading does not count as making dirty
            m_file->parametersModel()->setDirty(false);
        }
    }

    void readKMyMoney()
    {
        Q_ASSERT(m_reader->isStartElement() && (m_reader->name() == tagName(Tag::KMMFile)));

        while (m_reader->readNextStartElement()) {
            const auto tag = m_reader->name();
            if (tag == tagName(Tag::FileInfo)) {
                readFileInfo();
            } else if (tag == tagName(Tag::User)) {
                readUser();
            } else if (tag == tagName(Tag::Institutions)) {
                readInstitutions();
            } else if (tag == tagName(Tag::Payees)) {
                readPayees();
            } else if (tag == tagName(Tag::CostCenters)) {
                readCostCenters();
            } else if (tag == tagName(Tag::Tags)) {
                readTags();
            } else if (tag == tagName(Tag::Accounts)) {
                readAccounts();
            } else if (tag == tagName(Tag::Transactions)) {
                readTransactions();
            } else if (tag == tagName(Tag::KeyValuePairs)) {
                readKeyValuePairs();
            } else if (tag == tagName(Tag::Schedules)) {
                readSchedules();
            } else if (tag == tagName(Tag::Securities)) {
                readSecurities();
            } else if (tag == tagName(Tag::Currencies)) {
                readCurrencies();
            } else if (tag == tagName(Tag::Prices)) {
                readPrices();
            } else if (tag == tagName(Tag::Reports)) {
                readReports();
            } else if (tag == tagName(Tag::Budgets)) {
                readBudgets();
            } else if (tag == tagName(Tag::OnlineJobs)) {
                readOnlineJobs();
            } else {
                m_reader->skipCurrentElement();
            }
        }
    }
};

MyMoneyXmlReader::MyMoneyXmlReader()
    : d_ptr(new MyMoneyXmlReaderPrivate)
{
}

MyMoneyXmlReader::~MyMoneyXmlReader()
{
    delete d_ptr;
}

void MyMoneyXmlReader::setFile(MyMoneyFile* file)
{
    Q_D(MyMoneyXmlReader);
    Q_ASSERT(file != nullptr);

    d->m_file = file;
}

QString MyMoneyXmlReader::errorString() const
{
    Q_D(const MyMoneyXmlReader);
    if (d->m_reader->hasError()) {
        return d->m_reader->errorString();
    }
    return {};
}

bool MyMoneyXmlReader::read(QIODevice* device)
{
    Q_D(MyMoneyXmlReader);
    Q_ASSERT(d->m_file != nullptr);
    Q_ASSERT(device->isOpen());

    return d->read(device);
}

bool MyMoneyXmlReader::read(const QString& text)
{
    Q_D(MyMoneyXmlReader);
    d->m_reader->clear();
    d->m_reader->addData(text);
    if (d->m_reader->readNextStartElement()) {
        if (d->m_reader->name() == tagName(Tag::KMMFile)) {
            d->readKMyMoney();
        } else {
            return false;
        }
    } else {
        return false;
    }
    return !d->m_reader->hasError();
}
