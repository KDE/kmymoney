/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "config-kmymoney.h"

#include "templateloader.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QDir>
#include <QDomElement>
#include <QList>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTimer>
#include <QTreeView>
#include <QTreeWidget>
#include <QUrl>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KXmlGuiWindow>
#include <KMessageBox>
#include <KTextEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "templatesmodel.h"
#include "mymoneytemplate.h"
#include "kmymoneyutils.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"

class TemplateLoaderPrivate
{
    Q_DISABLE_COPY(TemplateLoaderPrivate)

public:
    TemplateLoaderPrivate(TemplateLoader* qq)
        : q_ptr(qq)
        , model(nullptr)
        , countryRow(0)
    {
    }

    ~TemplateLoaderPrivate()
    {
    }

    bool loadTemplate(const QUrl &url, MyMoneyTemplate& tmpl)
    {
        QString filename;
        bool downloadedFile = false;
        if (!url.isValid()) {
            qDebug() << "Invalid template URL" << url.url();
            return false;
        }

        if (url.isLocalFile()) {
            filename = url.toLocalFile();

        } else {
            downloadedFile = true;
            KIO::StoredTransferJob *transferjob = KIO::storedGet (url);
            KJobWidgets::setWindow(transferjob, KMyMoneyUtils::mainWindow());
            if (! transferjob->exec()) {
                KMessageBox::detailedError(KMyMoneyUtils::mainWindow(),
                                           i18n("Error while loading file '%1'.", url.url()),
                                           transferjob->errorString(),
                                           i18n("File access error"));
                return false;
            }
            QTemporaryFile file;
            file.setAutoRemove(false);
            file.open();
            file.write(transferjob->data());
            filename = file.fileName();
            file.close();
        }

        bool rc = true;
        QFile file(filename);
        QFileInfo info(file);
        if (!info.isFile()) {
            QString msg = i18n("<p><b>%1</b> is not a template file.</p>", filename);
            KMessageBox::error(KMyMoneyUtils::mainWindow(), msg, i18n("Filetype Error"));
            return false;
        }

        if (file.open(QIODevice::ReadOnly)) {
            tmpl.setSource(url);
            const auto result = tmpl.setAccountTree(&file);
            if (!result.isOK()) {
                QString msg = i18n("<p>Error while reading template file <b>%1</b> in line %2, column %3</p>", filename, result.errorLine, result.errorColumn);
                KMessageBox::detailedError(KMyMoneyUtils::mainWindow(), msg, result.errorMsg, i18nc("@title:window", "Template Loading Error"));
                rc = false;
            }
            file.close();
        } else {
            KMessageBox::error(KMyMoneyUtils::mainWindow(), i18n("File '%1' not found.", filename));
            rc = false;
        }

        // if a temporary file was downloaded, then it will be removed
        // with the next call. Otherwise, it stays untouched on the local
        // filesystem.
        if (downloadedFile) {
            QFile::remove(filename);
        }
        return rc;
    }

    bool createAccounts(const MyMoneyTemplate& tmpl, MyMoneyAccount& parent, QDomNode account)
    {
        bool rc = true;
        while (rc == true && !account.isNull()) {
            MyMoneyAccount acc;
            if (account.isElement()) {
                QDomElement accountElement = account.toElement();
                if (accountElement.tagName() == "account") {
                    QList<MyMoneyAccount> subAccountList;
                    QList<MyMoneyAccount>::ConstIterator it;
                    it = subAccountList.constEnd();
                    if (!parent.accountList().isEmpty()) {
                        MyMoneyFile::instance()->accountList(subAccountList, parent.accountList());
                        for (it = subAccountList.constBegin(); it != subAccountList.constEnd(); ++it) {
                            if ((*it).name() == accountElement.attribute("name")) {
                                acc = *it;
                                QString id = accountElement.attribute("id");
                                if (!id.isEmpty())
                                    m_vatAccountMap[id] = acc.id();
                                break;
                            }
                        }
                    }
                    if (it == subAccountList.constEnd()) {
                        // not found, we need to create it
                        acc.setName(accountElement.attribute("name"));
                        acc.setAccountType(static_cast<eMyMoney::Account::Type>(accountElement.attribute("type").toUInt()));
                        setFlags(tmpl, acc, account.firstChild());
                        try {
                            MyMoneyFile::instance()->addAccount(acc, parent);
                        } catch (const MyMoneyException &) {
                        }
                        QString id = accountElement.attribute("id");
                        if (!id.isEmpty())
                            m_vatAccountMap[id] = acc.id();
                    }
                    createAccounts(tmpl, acc, account.firstChild());
                }
            }
            account = account.nextSibling();
        }
        return rc;
    }

    bool setFlags(const MyMoneyTemplate& tmpl, MyMoneyAccount& acc, QDomNode flags)
    {
        bool rc = true;
        while (rc == true && !flags.isNull()) {
            if (flags.isElement()) {
                QDomElement flagElement = flags.toElement();
                if (flagElement.tagName() == "flag") {
                    // make sure, we only store flags we know!
                    QString value = flagElement.attribute("name");
                    if (value == "Tax") {
                        acc.setValue(value, "Yes");
                    } else if (value == "VatRate") {
                        acc.setValue(value, flagElement.attribute("value"));
                    } else if (value == "VatAccount") {
                        // will be resolved later in importTemplate()
                        acc.setValue("UnresolvedVatAccount", flagElement.attribute("value"));
                    } else if (value == "OpeningBalanceAccount") {
                        acc.setValue("OpeningBalanceAccount", "Yes");
                    } else {
                        KMessageBox::error(KMyMoneyUtils::mainWindow(), i18n("<p>Invalid flag type <b>%1</b> for account <b>%3</b> in template file <b>%2</b></p>", flagElement.attribute("name"), tmpl.source().toDisplayString(), acc.name()));
                        rc = false;
                    }
                    QString currency = flagElement.attribute("currency");
                    if (!currency.isEmpty())
                        acc.setCurrencyId(currency);
                }
            }
            flags = flags.nextSibling();
        }
        return rc;
    }

public:
    TemplateLoader*                         q_ptr;
    TemplatesModel*                         model;
    // a map of country name or country name (language name) -> localeId (lang_country) so be careful how you use it
    QMap<QString, QString>                  countries;
    QString                                 currentLocaleId;

    QStringList                             dirlist;          ///< list of directories to scan for templates
    QMap<QString, QString>::const_iterator  it_m;
    int                                     countryRow;
    QMap<QString,QString>                   m_vatAccountMap;
};

TemplateLoader::TemplateLoader(QWidget* parent) :
    QObject(parent),
    d_ptr(new TemplateLoaderPrivate(this))
{
    Q_INIT_RESOURCE(templates);
}

TemplateLoader::~TemplateLoader()
{
    Q_D(TemplateLoader);
    delete d;
}

void TemplateLoader::load(TemplatesModel* model)
{
    Q_D(TemplateLoader);
    d->model = model;
    model->unload();
    d->currentLocaleId.clear();

    QStringList dirs;

    if (d->model == nullptr) {
        return;
    }
    d->dirlist = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, "templates", QStandardPaths::LocateDirectory);
    d->dirlist.append(":/templates");

    QStringList::iterator it;
    for (it = d->dirlist.begin(); it != d->dirlist.end(); ++it) {
        QDir dir(*it);
        dirs = dir.entryList(QStringList("*"), QDir::Dirs | QDir::NoDotAndDotDot);
        QStringList::iterator it_d;

        // note: the logic for multiple languages seems to work only for two
        // a third one will be entered without the language in parenthesis
        for (it_d = dirs.begin(); it_d != dirs.end(); ++it_d) {
            QLocale templateLocale(*it_d);
            if (templateLocale.language() != QLocale::C) {
                QString country = QLocale().countryToString(templateLocale.country());
                QString lang = QLocale().languageToString(templateLocale.language());
                if (d->countries.contains(country)) {
                    if (d->countries[country] != *it_d) {
                        QString otherName = d->countries[country];
                        QLocale otherTemplateLocale(otherName);
                        QString otherCountry = QLocale().countryToString(otherTemplateLocale.country());
                        QString otherLang = QLocale().languageToString(otherTemplateLocale.language());
                        d->countries.remove(country);
                        d->countries[QString("%1 (%2)").arg(otherCountry, otherLang)] = otherName;
                        d->countries[QString("%1 (%2)").arg(country, lang)] = *it_d;
                        // retain the item corresponding to the current locale
                        if (QLocale().country() == templateLocale.country()) {
                            d->currentLocaleId = *it_d;
                        }
                    }
                } else {
                    d->countries[country] = *it_d;
                    // retain the item corresponding to the current locale
                    if (QLocale().country() == templateLocale.country()) {
                        d->currentLocaleId = *it_d;
                    }
                }
            } else {
                qDebug("'%s/%s' not scanned", qPrintable(*it), qPrintable(*it_d));
            }
        }
    }

    // now that we know, what we can get at max, we scan everything
    // and parse the templates into the model

    d->model->insertRows(0, d->countries.count());
    d->countryRow = 0;
    d->it_m = d->countries.constBegin();

    // in case we have found countries, we load them
    if (d->countryRow < d->countries.count()) {
        QMetaObject::invokeMethod(this, "slotLoadCountry", Qt::QueuedConnection);
    } else {
        Q_EMIT loadingFinished();
    }
}

void TemplateLoader::slotLoadCountry()
{
    Q_D(TemplateLoader);

    const auto parentIdx = d->model->index(d->countryRow, 0);
    d->model->setData(parentIdx, d->it_m.key(), eMyMoney::Model::TemplatesCountryRole);
    d->model->setData(parentIdx, d->it_m.value(), eMyMoney::Model::TemplatesLocaleRole);

    // now scan all directories for that country
    for (QStringList::iterator it = d->dirlist.begin(); it != d->dirlist.end(); ++it) {
        QDir dir(QString("%1/%2").arg(*it).arg(d->it_m.value()));
        if (dir.exists()) {
            const QStringList files = dir.entryList(QStringList("*.kmt"), QDir::Files);
            for (const auto& file : qAsConst(files)) {
                const auto url = QUrl::fromUserInput(QString("%1/%2").arg(dir.canonicalPath(), file));
                MyMoneyTemplate tmpl;
                if (d->loadTemplate(url, tmpl)) {
                    d->model->addItem(tmpl, parentIdx);
                }
            }
        }
    }

    // next item in list
    ++d->it_m;
    ++d->countryRow;
    if (d->countryRow < d->countries.count()) {
        QMetaObject::invokeMethod(this, "slotLoadCountry", Qt::QueuedConnection);
    } else {
        Q_EMIT loadingFinished();
    }
}

bool TemplateLoader::importTemplate(const MyMoneyTemplate& tmpl)
{
    Q_D(TemplateLoader);
    d->m_vatAccountMap.clear();
    auto accounts = tmpl.accountTree();
    bool rc = !accounts.isNull();

    MyMoneyFile* file = MyMoneyFile::instance();
    while (rc == true && !accounts.isNull() && accounts.isElement()) {
        QDomElement childElement = accounts.toElement();
        if (childElement.tagName() == "account") {
            MyMoneyAccount parent;
            switch (childElement.attribute("type").toUInt()) {
            case (uint)eMyMoney::Account::Type::Asset:
                parent = file->asset();
                break;
            case (uint)eMyMoney::Account::Type::Liability:
                parent = file->liability();
                break;
            case (uint)eMyMoney::Account::Type::Income:
                parent = file->income();
                break;
            case (uint)eMyMoney::Account::Type::Expense:
                parent = file->expense();
                break;
            case (uint)eMyMoney::Account::Type::Equity:
                parent = file->equity();
                break;

            default:
                KMessageBox::error(KMyMoneyUtils::mainWindow(), i18n("<p>Invalid top-level account type <b>%1</b> in template file <b>%2</b></p>", childElement.attribute("type"), tmpl.source().toDisplayString()));
                rc = false;
            }

            if (rc == true) {
                if (childElement.attribute("name").isEmpty())
                    rc = d->createAccounts(tmpl, parent, childElement.firstChild());
                else
                    rc = d->createAccounts(tmpl, parent, childElement);
            }
        } else {
            rc = false;
        }
        accounts = accounts.nextSibling();
    }

    /*
     * Resolve imported vat account assignments
     *
     * The template account id of the assigned vat account
     * is stored temporarily in the account key/value pair
     * 'UnresolvedVatAccount' and resolved below.
     */
    QList<MyMoneyAccount> accountList;
    file->accountList(accountList);
    Q_FOREACH (MyMoneyAccount acc, accountList) {
        if (!acc.pairs().contains("UnresolvedVatAccount"))
            continue;
        QString id = acc.value("UnresolvedVatAccount");
        acc.setValue("VatAccount", d->m_vatAccountMap[id]);
        acc.deletePair("UnresolvedVatAccount");
        MyMoneyFile::instance()->modifyAccount(acc);
    }

    return rc;
}
