/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "qifimporter.h"

// ----------------------------------------------------------------------------
// QT Includes
#ifdef IS_APPIMAGE
#include <QCoreApplication>
#include <QStandardPaths>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "kimportdlg.h"
#include "mymoneyqifreader.h"
#include "statementinterface.h"
#include "viewinterface.h"

class MyMoneyStatement;

#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
QIFImporter::QIFImporter(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, args),
#else
QIFImporter::QIFImporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, metaData, args),
#endif
    m_qifReader(nullptr)
{
    const auto rcFileName = QLatin1String("qifimporter.rc");

#ifdef IS_APPIMAGE
    const QString rcFilePath = QString("%1/../share/kxmlgui5/%2/%3").arg(QCoreApplication::applicationDirPath(), objectName(), rcFileName);
    setXMLFile(rcFilePath);

    const QString localRcFilePath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + QLatin1Char('/') + rcFileName;
    setLocalXMLFile(localRcFilePath);
#else
    setXMLFile(rcFileName);
#endif

    createActions();
    // For information, announce that we have been loaded.
    qDebug("Plugins: qifimporter loaded");
}

QIFImporter::~QIFImporter()
{
    delete m_qifReader;
    actionCollection()->removeAction(m_action);
    qDebug("Plugins: qifimporter unloaded");
}

void QIFImporter::createActions()
{
    m_action = actionCollection()->addAction(QStringLiteral("file_import_qif"));
    m_action->setText(i18n("QIF..."));
    connect(m_action, &QAction::triggered, this, &QIFImporter::slotQifImport);
    connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, m_action, &QAction::setEnabled);
}

void QIFImporter::slotQifImport()
{
    QPointer<KImportDlg> dlg = new KImportDlg(nullptr);

    if (dlg->exec() == QDialog::Accepted && dlg != nullptr) {
        m_action->setEnabled(false);
        delete m_qifReader;
        m_qifReader = new MyMoneyQifReader;
        statementInterface()->resetMessages();
        connect(m_qifReader, &MyMoneyQifReader::statementsReady, this, &QIFImporter::slotGetStatements);

        m_qifReader->setURL(dlg->file());
        m_qifReader->setProfile(dlg->profile());
        m_qifReader->setCategoryMapping(dlg->m_typeComboBox->currentIndex() == 0);
        if (!m_qifReader->startImport()) {
            delete m_qifReader;
            statementInterface()->showMessages(0);
            m_action->setEnabled(true);
        }
    }
    delete dlg;
}

bool QIFImporter::slotGetStatements(const QList<MyMoneyStatement> &statements)
{
    auto ret = true;
    for (const auto& statement : statements) {
        const auto singleImportSummary = statementInterface()->import(statement);
        if (singleImportSummary.isEmpty())
            ret = false;
    }

    // inform the user about the result of the operation
    statementInterface()->showMessages(statements.count());

    // allow further QIF imports
    m_action->setEnabled(true);
    return ret;
}

K_PLUGIN_CLASS_WITH_JSON(QIFImporter, "qifimporter.json")

#include "qifimporter.moc"
