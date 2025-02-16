/*
    SPDX-FileCopyrightText: 2010-2014 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "csvimporter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "core/csvimportercore.h"
#include "csvwizard.h"
#include "mymoneyfile.h"
#include "selectedobjects.h"
#include "statementinterface.h"
#include "viewinterface.h"

class CSVImporter::Private
{
public:
    Q_DISABLE_COPY_MOVE(Private)

    Private(CSVImporter* qq)
        : q(qq)
        , m_silent(false)
        , m_action(nullptr)
        , m_parentWidget(nullptr)
    {
    }

    void createActions()
    {
        const auto kpartgui = QStringLiteral("file_import_csv");
        m_action = q->actionCollection()->addAction(kpartgui);
        m_action->setText(i18n("CSV..."));
        q->connect(m_action, &QAction::triggered, q, [&]() {
            q->import(QString());
        });
        connect(q->viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, q->action(kpartgui.toLatin1().constData()), &QAction::setEnabled);
    }

    CSVImporter* const q;
    SelectedObjects m_selections;
    bool m_silent;
    QAction* m_action;
    QWidget* m_parentWidget;
};

CSVImporter::CSVImporter(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args)
    : KMyMoneyPlugin::Plugin(parent, metaData, args)
    , d(new Private(this))
{
    Q_INIT_RESOURCE(csvimporter);

    if (!args.isEmpty()) {
        d->m_parentWidget = args.at(0).value<QWidget*>();
    }
    const auto rcFileName = QLatin1String("csvimporter.rc");
    setXMLFile(rcFileName);

    d->createActions();
    // For information, announce that we have been loaded.
    qDebug("Plugins: csvimporter loaded");
}

CSVImporter::~CSVImporter()
{
    actionCollection()->removeAction(d->m_action);
    delete d;
    qDebug("Plugins: csvimporter unloaded");
}

QStringList CSVImporter::formatMimeTypes() const
{
    return {"text/csv", "text/tab-separated-values"};
}

bool CSVImporter::import(const QString& filename)
{
    QPointer<CSVWizard> wizard = new CSVWizard(d->m_parentWidget, this);
    wizard->presetFilename(filename);
    auto rc = false;

    if ((wizard->exec() == QDialog::Accepted) && wizard) {
        auto statement = wizard->statement();
        // if we have no information about the account, we simply use
        // the current selection as proposal and ask the user
        if (statement.m_accountId.isEmpty() && statement.m_strAccountName.isEmpty() && statement.m_strAccountNumber.isEmpty()) {
            const auto account = MyMoneyFile::instance()->accountsModel()->itemById(d->m_selections.firstSelection(SelectedObjects::Account));
            statement.m_strAccountName = account.name();
            statement.m_strAccountNumber = account.number();
        }
        rc = !statementInterface()->import(statement, false).isEmpty();
    }
    if (wizard) {
        wizard->deleteLater();
    }
    return rc;
}

QString CSVImporter::lastError() const
{
    return QString();
}

void CSVImporter::updateActions(const SelectedObjects& selections)
{
    d->m_selections = selections;
}

K_PLUGIN_CLASS_WITH_JSON(CSVImporter, "csvimporter.json")

#include "csvimporter.moc"
