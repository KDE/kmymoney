/***************************************************************************
                          xea2kmt.cpp
                          -------------------
    copyright            : (C) 2014 by Ralf Habacker <ralf.habacker@freenet.de>

****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QtDebug>

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

class TemplateAccount {
public:
    typedef QList<TemplateAccount> TemplateAccountMap;
    QString id;
    QString type;
    QString name;
    QString code;
    QString parent;

    TemplateAccount()
    {
    }

    TemplateAccount(const TemplateAccount &b)
      : id(b.id),
        type(b.type),
        name(b.name),
        code(b.code),
        parent(b.parent)
    {
    }

    void clear()
    {
        id = "";
        type = "";
        name = "";
        code = "";
        parent = "";
    }

    bool read(QXmlStreamReader &xml)
    {
        while (!xml.atEnd()) {
            xml.readNext();
            QStringRef _name = xml.name();
            if (xml.isEndElement() && _name == "account")
                return true;
            if (xml.isStartElement())
            {
                QString value = xml.readElementText(QXmlStreamReader::SkipChildElements);
                if (_name == "name")
                    name = value;
                else if (_name == "id")
                    id = value;
                else if (_name == "type")
                    type = value;
                else if (_name == "code")
                    code = value;
                else if (_name == "parent")
                    parent = value;
                else
                {
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
        << "name:" << a.name
        << "id:" << a.id
        << "type:" << a.type
        << "code:" << a.code
        << "parent:" << a.parent
        << ")\n";
    return out;
}

class TemplateFile {
public:
    QString title;
    QString longDescription;
    QString shortDescription;
    TemplateAccount::TemplateAccountMap accounts;

    bool read(QXmlStreamReader &xml)
    {
        Q_ASSERT(xml.isStartElement() && xml.name() == "gnc-account-example");

        while (xml.readNextStartElement()) {
            QStringRef name = xml.name();
            if (xml.name() == "title")
                title = xml.readElementText();
            else if (xml.name() == "short-description")
                shortDescription = xml.readElementText();
            else if (xml.name() == "long-description")
                longDescription = xml.readElementText();
            else if (xml.name() == "account")
            {
                TemplateAccount account;
                if (account.read(xml))
                    accounts.append(account);
            }
            else
            {
                qDebug() << "skipping" << name.toString();
                xml.skipCurrentElement();
            }
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
        in.setCodec("UTF-8");

        if(!file.open(QIODevice::ReadOnly))
            return false;
        return read(in.device());
    }

    TemplateFile &result()
    {
        return _template;
    }

protected:
    bool read(QIODevice *device)
    {
        xml.setDevice(device);

        if (xml.readNextStartElement()) {
            QStringRef name = xml.name();
            if (xml.name() == "gnc-account-example")
                _template.read(xml);
            else
                xml.raiseError(QObject::tr("The file is not an gnucash account template file."));
        }
        return !xml.error();
    }

    QXmlStreamReader xml;
    TemplateFile _template;
};

int main(int argc, char *argv[])
{
    GnuCashAccountTemplateReader reader;
    bool result = reader.read(argv[1]);
    qDebug() << reader.result();

    // create account hierachy
    // export kmt file
    return result;
}
