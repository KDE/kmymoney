/*
    SPDX-FileCopyrightText: 2014 Ralf Habacker <ralf.habacker@freenet.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../kmymoney/mymoney/mymoneyaccount.h"

#include <QDir>
#include <QFile>
#include <QStringList>
#include <QMap>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QDebug>

#include "mymoneyenums.h"

using namespace eMyMoney;

QDebug operator <<(QDebug out, const QXmlStreamNamespaceDeclaration &a)
{
    out << "QXmlStreamNamespaceDeclaration("
        << "prefix:" << a.prefix().toString()
        << "namespaceuri:" << a.namespaceUri().toString()
        << ")";
    return out;
}

QDebug operator <<(QDebug out, const QXmlStreamAttribute &a)
{
    out << "QXmlStreamAttribute("
        << "prefix:" << a.prefix().toString()
        << "namespaceuri:" << a.namespaceUri().toString()
        << "name:" << a.name().toString()
        << " value:" << a.value().toString()
        << ")";
    return out;
}

static bool debug = false;
static bool verbose = false;
static bool withID = false;
static bool noLevel1Names = false;
static bool withTax = false;
static bool prefixNameWithCode = false;

typedef QMap<QString,QString> DirNameMapType;

/**
 * map to hold differences from gnucash to kmymoney template directory
 * @return directory name map
 */
DirNameMapType &getDirNameMap()
{
    static DirNameMapType dirNameMap;
    dirNameMap["cs"] = "cs_CZ";
    dirNameMap["da"] = "dk";
    dirNameMap["ja"] = "ja_JP";
    dirNameMap["ko"] = "ko_KR";
    dirNameMap["nb"] = "nb_NO";
    dirNameMap["nl"] = "nl_NL";
    dirNameMap["ru"] = "ru_RU";
    return dirNameMap;
}

int toKMyMoneyAccountType(const QString &type)
{
    if(type == "ROOT")              return (int)Account::Type::Unknown;
    else if (type == "BANK")        return (int)Account::Type::Checkings;
    else if (type == "CASH")        return (int)Account::Type::Cash;
    else if (type == "CREDIT")      return (int)Account::Type::CreditCard;
    else if (type == "INVEST")      return (int)Account::Type::Investment;
    else if (type == "RECEIVABLE")  return (int)Account::Type::Asset;
    else if (type == "ASSET")       return (int)Account::Type::Asset;
    else if (type == "PAYABLE")     return (int)Account::Type::Liability;
    else if (type == "LIABILITY")   return (int)Account::Type::Liability;
    else if (type == "CURRENCY")    return (int)Account::Type::Currency;
    else if (type == "INCOME")      return (int)Account::Type::Income;
    else if (type == "EXPENSE")     return (int)Account::Type::Expense;
    else if (type == "STOCK")       return (int)Account::Type::Stock;
    else if (type == "MUTUAL")      return (int)Account::Type::Stock;
    else if (type == "EQUITY")      return (int)Account::Type::Equity;
    else return 99; // unknown
}

class TemplateAccount {
public:
    typedef QList<TemplateAccount> List;
    typedef QList<TemplateAccount*> PointerList;
    typedef QMap<QString,QString> SlotList;

    QString id;
    QString m_type;
    QString m_name;
    QString code;
    QString parent;
    SlotList slotList;

    TemplateAccount()
    {
    }

    TemplateAccount(const TemplateAccount &b)
        : id(b.id),
          m_type(b.m_type),
          m_name(b.m_name),
          code(b.code),
          parent(b.parent),
          slotList(b.slotList)
    {
    }

    void clear()
    {
        id = "";
        m_type = "";
        m_name = "";
        code = "";
        parent = "";
        slotList.clear();
    }

    bool readSlots(QXmlStreamReader &xml)
    {
        while (!xml.atEnd()) {
            QXmlStreamReader::TokenType type = xml.readNext();
            if (type == QXmlStreamReader::StartElement) {
                QStringRef _name = xml.name();
                if (_name == "slot") {
                    type = xml.readNext();
                    if (type == QXmlStreamReader::Characters)
                        type = xml.readNext();
                    if (type == QXmlStreamReader::StartElement) {
                        QStringRef name = xml.name();
                        QString key, value;
                        if (name == "key")
                            key = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                        type = xml.readNext();
                        if (type == QXmlStreamReader::Characters)
                            type = xml.readNext();
                        if (type == QXmlStreamReader::StartElement) {
                            name = xml.name();
                            if (name == "value")
                                value = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                        }
                        if (!key.isEmpty() && !value.isEmpty())
                            slotList[key] = value;
                    }
                }
            } else if (type == QXmlStreamReader::EndElement) {
                QStringRef _name = xml.name();
                if (_name  == "slots")
                    return true;
            }
        }
        return true;
    }

    bool read(QXmlStreamReader &xml)
    {
        while (!xml.atEnd()) {
            xml.readNext();
            QStringRef _name = xml.name();
            if (xml.isEndElement() && _name == "account") {
                if (prefixNameWithCode && !code.isEmpty() && !m_name.startsWith(code))
                    m_name = code + ' ' + m_name;
                return true;
            }
            if (xml.isStartElement())
            {
                if (_name == "name")
                    m_name = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                else if (_name == "id")
                    id = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                else if (_name == "type")
                    m_type = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                else if (_name == "code")
                    code = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                else if (_name == "parent")
                    parent = xml.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
                else if (_name == "slots")
                    readSlots(xml);
                else
                {
                    xml.readElementText(QXmlStreamReader::SkipChildElements);
                    if (debug)
                        qDebug() << "skipping" << _name.toString();
                }
            }
        }
        return false;
    }
};

QDebug operator <<(QDebug out, const TemplateAccount &a)
{
    out << "TemplateAccount("
        << "name:" << a.m_name
        << "id:" << a.id
        << "type:" << a.m_type
        << "code:" << a.code
        << "parent:" << a.parent
        << "slotList:" << a.slotList
        << ")\n";
    return out;
}

QDebug operator <<(QDebug out, const TemplateAccount::PointerList &a)
{
    out << "TemplateAccount::List(";
    foreach(const TemplateAccount *account, a)
        out << *account;
    out << ")";
    return out;
}

class TemplateFile {
public:
    QString title;
    QString longDescription;
    QString shortDescription;
    QString fileName;
    TemplateAccount::List accounts;
    TemplateAccount *openingBalanceAccount{nullptr};

    bool read(QXmlStreamReader &xml)
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == "gnc-account-example");

        while (xml.readNextStartElement()) {
            QStringRef name = xml.name();
            if (xml.name() == "title")
                title = xml.readElementText().trimmed();
            else if (xml.name() == "short-description")
                shortDescription = xml.readElementText().trimmed().replace("  ", " ");
            else if (xml.name() == "long-description")
                longDescription = xml.readElementText().trimmed().replace("  ", " ");
            else if (xml.name() == "account")
            {
                TemplateAccount account;
                if (account.read(xml))
                    accounts.append(account);
            }
            else
            {
                if (debug)
                    qDebug() << "skipping" << name.toString();
                xml.skipCurrentElement();
            }
        }
        return true;
    }

    bool writeAsXml(QXmlStreamWriter &xml)
    {
        xml.writeStartElement("","title");
        xml.writeCharacters(title);
        xml.writeEndElement();
        xml.writeStartElement("","shortdesc");
        xml.writeCharacters(shortDescription);
        xml.writeEndElement();
        xml.writeStartElement("","longdesc");
        xml.writeCharacters(longDescription);
        xml.writeEndElement();
        xml.writeStartElement("","accounts");
        bool result = writeAccountsAsXml(xml);
        xml.writeEndElement();
        return result;
    }

    bool writeAccountsAsXml(QXmlStreamWriter &xml, const QString &id="", int index=0)
    {
        TemplateAccount::PointerList list;

        if (index == 0)
            list = accountsByType("ROOT");
        else
            list = accountsByParentID(id);

        foreach(TemplateAccount *account, list)
        {
            if (account->m_type != "ROOT")
            {
                xml.writeStartElement("","account");
                xml.writeAttribute("type", QString::number(toKMyMoneyAccountType(account->m_type)));
                xml.writeAttribute("name", noLevel1Names && index < 2 ? "" : account->m_name);
                if (withID)
                    xml.writeAttribute("id", account->id);
                if (withTax) {
                    if (account->slotList.contains("tax-related")) {
                        xml.writeStartElement("flag");
                        xml.writeAttribute("name","Tax");
                        xml.writeAttribute("value",account->slotList["tax-related"] == "1" ? "Yes" : "No");
                        xml.writeEndElement();
                    }
                }
                if (account->slotList.contains("equity-type") && account->slotList["equity-type"] == "opening-balance") {
                    if (openingBalanceAccount) {
                        qWarning() << "template" << fileName << "already has specified"
                                   << openingBalanceAccount->m_name
                                   << "as opening balance account,"
                                   << "ignoring account" << account->m_name;
                        continue;
                    }
                    xml.writeStartElement("flag");
                    xml.writeAttribute("name","OpeningBalanceAccount");
                    xml.writeAttribute("value","Yes");
                    xml.writeEndElement();
                    openingBalanceAccount = account;
                }
            }
            index++;
            writeAccountsAsXml(xml, account->id, index);
            index--;
            xml.writeEndElement();
        }
        return true;
    }

    TemplateAccount *account(const QString &id)
    {
        for(int i=0; i < accounts.size(); i++)
        {
            TemplateAccount &account = accounts[i];
            if (account.id == id)
                return &account;
        }
        return 0;
    }

    TemplateAccount::PointerList accountsByType(const QString &type)
    {
        TemplateAccount::PointerList list;
        for(int i=0; i < accounts.size(); i++)
        {
            TemplateAccount &account = accounts[i];
            if (account.m_type == type)
                list.append(&account);
        }
        return list;
    }


    static bool nameLessThan(TemplateAccount *a1, TemplateAccount *a2)
    {
        return a1->m_name < a2->m_name;
    }

    TemplateAccount::PointerList accountsByParentID(const QString &parentID)
    {
        TemplateAccount::PointerList list;

        for(int i=0; i < accounts.size(); i++)
        {
            TemplateAccount &account = accounts[i];
            if (account.parent == parentID)
                list.append(&account);
        }
        std::sort(list.begin(), list.end(), nameLessThan);
        return list;
    }

    bool dumpTemplates(const QString &id="", int index=0)
    {
        TemplateAccount::PointerList list;

        if (index == 0)
            list = accountsByType("ROOT");
        else
            list = accountsByParentID(id);

        foreach(TemplateAccount *account, list)
        {
            QString a;
            a.fill(' ', index);
            qDebug() << a << account->m_name << toKMyMoneyAccountType(account->m_type);
            index++;
            dumpTemplates(account->id, index);
            index--;
        }
        return true;
    }
};

QDebug operator <<(QDebug out, const TemplateFile &a)
{
    out << "TemplateFile("
        << "title:" << a.title
        << "short description:" << a.shortDescription
        << "long description:" << a.longDescription
        << "accounts:";
    foreach(const TemplateAccount &account, a.accounts)
        out << account;
    out << ")";
    return out;
}

class GnuCashAccountTemplateReader {
public:
    GnuCashAccountTemplateReader()
    {
    }

    bool read(const QString &filename)
    {
        QFile file(filename);
        QTextStream in(&file);
        in.setCodec("utf-8");

        if(!file.open(QIODevice::ReadOnly))
            return false;
        inFileName = filename;
        return read(in.device());
    }

    TemplateFile &result()
    {
        return _template;
    }

    bool dumpTemplates()
    {
        return _template.dumpTemplates();
    }

    bool writeAsXml(const QString &filename=QString())
    {
        if (filename.isEmpty())
        {
            QTextStream stream(stdout);
            return writeAsXml(stream.device());
        }
        else
        {
            QFile file(filename);
            if(!file.open(QIODevice::WriteOnly))
                return false;
            return writeAsXml(&file);
        }
    }

protected:

    bool checkAndUpdateAvailableNamespaces(QXmlStreamReader &xml)
    {
        if (xml.namespaceDeclarations().size() < 5)
        {
            qWarning() << "gnucash template file is missing required name space declarations; adding by self";
        }
        xml.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("act", "http://www.gnucash.org/XML/act"));
        xml.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("gnc", "http://www.gnucash.org/XML/gnc"));
        xml.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("gnc-act", "http://www.gnucash.org/XML/gnc-act"));
        xml.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("cmdty","http://www.gnucash.org/XML/cmdty"));
        xml.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("slot","http://www.gnucash.org/XML/slot"));
        return true;
    }

    bool read(QIODevice *device)
    {
        m_xml.setDevice(device);
        while(!m_xml.atEnd())
        {
            m_xml.readNext();
            if (m_xml.isStartElement())
            {
                if (m_xml.name() == "gnc-account-example")
                {
                    checkAndUpdateAvailableNamespaces(m_xml);
                    _template.read(m_xml);
                }
                else
                    m_xml.raiseError(QObject::tr("The file is not an gnucash account template file."));
            }
        }
        if (m_xml.error() != QXmlStreamReader::NoError)
            qWarning() << m_xml.errorString();
        return !m_xml.error();
    }

    bool writeAsXml(QIODevice *device)
    {
        QXmlStreamWriter xml(device);
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(1);
        xml.setCodec("utf-8");
        xml.writeStartDocument();

        QString fileName = inFileName;
        fileName.replace(QRegExp(".*/accounts"),"accounts");
        xml.writeComment(QString("\n"
                                 "     Converted using xea2kmt from GnuCash sources\n"
                                 "\n"
                                 "        %1\n"
                                 "\n"
                                 "     Please check the source file for possible copyright\n"
                                 "     and license information.\n"
                                ).arg(fileName));
        xml.writeDTD("<!DOCTYPE KMYMONEY-TEMPLATE>");
        xml.writeStartElement("","kmymoney-account-template");
        _template.fileName = fileName;
        bool result = _template.writeAsXml(xml);
        xml.writeEndElement();
        xml.writeEndDocument();
        return result;
    }

    QXmlStreamReader m_xml;
    TemplateFile _template;
    QString inFileName;
};

void scanDir(QDir dir, QStringList &files)
{
    dir.setNameFilters(QStringList("*.gnucash-xea"));
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    if (debug)
        qDebug() << "Scanning: " << dir.path();

    QStringList fileList = dir.entryList();
    for (int i=0; i<fileList.count(); i++)
    {
        if (debug)
            qDebug() << "Found file: " << fileList[i];
        files.append(QString("%1/%2").arg(dir.absolutePath()).arg(fileList[i]));
    }

    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList dirList = dir.entryList();
    for (int i=0; i<dirList.size(); ++i)
    {
        QString newPath = QString("%1/%2").arg(dir.absolutePath()).arg(dirList.at(i));
        scanDir(QDir(newPath), files);
    }
}

bool convertFile(const QString &inFile, const QString &outFile)
{
    GnuCashAccountTemplateReader reader;
    if (!reader.read(inFile))
        return false;
    return reader.writeAsXml(outFile);
}

int convertFileStructure(const QString &indir, const QString &outdir)
{
    DirNameMapType &dirNameMap = getDirNameMap();
    // get gnucash account files
    QDir d(indir);
    QStringList files;
    scanDir(d, files);

    QString inPath = d.absolutePath();
    QDir outDir(outdir);
    QString outPath = outDir.absolutePath();
    QStringList mapKeys = dirNameMap.keys();
    int result = 0;

    // process templates
    foreach (const QString &file, files)
    {
        if (debug || verbose)
            qDebug() << "processing" << file;

        // create output file dir
        QFileInfo fi(file);
        auto outFileName = fi.canonicalFilePath();
        outFileName.replace(inPath, outPath);
        outFileName.remove("acctchrt_");
        outFileName.replace(".gnucash-xea", ".kmt");
        foreach(const QString &key, mapKeys)
        {
            if (outFileName.contains('/' + key + '/'))
                outFileName = outFileName.replace('/' + key + '/', '/' + dirNameMap[key] + '/');
        }
        fi.setFile(outFileName);

        d = fi.absolutePath();
        if (!d.exists())
        {
            if  (debug)
                qDebug() << "creating path " << fi.absolutePath();
            d.mkpath(fi.absolutePath());
        }
        if (debug)
            qDebug() << "writing to " << outFileName;
        if (!convertFile(file, outFileName))
        {
            qWarning() << "could not create" << outFileName;
            result = 1;
        }
    }
    return result;
}

int main(int argc, char *argv[])
{
    if (argc < 2 || (argc == 2 && QLatin1String(argv[1]) == "--help"))
    {
        qWarning() << "xea2kmt: convert gnucash template file to kmymoney template file";
        qWarning() << argv[0] << "<options> <gnucash-template-file> [<kmymoney-template-output-file>]";
        qWarning() << argv[0] << "<options> --in-dir <gnucash-template-files-root> --out-dir <kmymoney-template-files-root>";
        qWarning() << "options:";
        qWarning() << "          --debug                   - output debug information";
        qWarning() << "          --help                    - this page";
        qWarning() << "          --no-level1-names         - do not export account names for top level accounts";
        qWarning() << "          --prefix-name-with-code   - prefix account name with account code if present";
        qWarning() << "          --verbose                 - output processing information";
        qWarning() << "          --with-id                 - write account id attribute";
        qWarning() << "          --with-tax-related        - parse and export gnucash 'tax-related' flag";
        qWarning() << "          --in-dir <dir>            - search for gnucash templates files in <dir>";
        qWarning() << "          --out-dir <dir>           - generate kmymoney templates below <dir";
        return -1;
    }

    QString inFileName;
    QString outFileName;
    QString inDir;
    QString outDir;
    for(int i = 1; i < argc; i++)
    {
        QString arg = QLatin1String(argv[i]);
        if (arg == "--debug")
            debug = true;
        else if (arg == "--verbose")
            verbose = true;
        else if (arg == "--with-id")
            withID = true;
        else if (arg == "--no-level1-names")
            noLevel1Names = true;
        else if (arg == "--with-tax-related")
            withTax = true;
        else if (arg == "--prefix-name-with-code")
            prefixNameWithCode = true;
        else if (arg == "--in-dir")
            inDir = argv[++i];
        else if (arg == "--out-dir")
            outDir = argv[++i];
        else if (!arg.startsWith(QLatin1String("--")))
        {
            if (inFileName.isEmpty())
                inFileName = arg;
            else
                outFileName = arg;
        }
        else
        {
            qWarning() << "invalid command line parameter'" << arg << "'";
            return -1;
        }
    }

    if (!inDir.isEmpty() && !outDir.isEmpty())
    {
        return convertFileStructure(inDir, outDir);
    }

    GnuCashAccountTemplateReader reader;
    bool result = reader.read(inFileName);
    if (debug)
    {
        qDebug() << reader.result();
        reader.dumpTemplates();
    }
    reader.writeAsXml(outFileName);
    return result ? 0 : -2;
}
