/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "qifexporter.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KActionCollection>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kexportdlg.h"

#include "mymoneyqifwriter.h"
#include "viewinterface.h"

#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
QIFExporter::QIFExporter(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, args)
#else
QIFExporter::QIFExporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, metaData, args)
#endif
{
    Q_INIT_RESOURCE(qifexporter);

    const auto rcFileName = QLatin1String("qifexporter.rc");
    setXMLFile(rcFileName);

    createActions();
    // For information, announce that we have been loaded.
    qDebug("Plugins: qifexporter loaded");
}

QIFExporter::~QIFExporter()
{
    actionCollection()->removeAction(m_action);
    qDebug("Plugins: qifexporter unloaded");
}


void QIFExporter::createActions()
{
    m_action = actionCollection()->addAction(QStringLiteral("file_export_qif"));
    m_action->setText(i18n("QIF..."));
    connect(m_action, &QAction::triggered, this, &QIFExporter::slotQifExport);
    connect(viewInterface(), &KMyMoneyPlugin::ViewInterface::viewStateChanged, m_action, &QAction::setEnabled);
}


void QIFExporter::slotQifExport()
{
    m_action->setEnabled(false);
    QPointer<KExportDlg> dlg = new KExportDlg(nullptr);
    if (dlg->exec() == QDialog::Accepted && dlg != nullptr) {
//    if (okToWriteFile(QUrl::fromLocalFile(dlg->filename()))) {
        MyMoneyQifWriter writer;

        writer.write(dlg->filename(), dlg->profile(), dlg->accountId(),
                     dlg->accountSelected(), dlg->categorySelected(),
                     dlg->startDate(), dlg->endDate());
//    }
    }
    delete dlg;
    m_action->setEnabled(true);
}

K_PLUGIN_CLASS_WITH_JSON(QIFExporter, "qifexporter.json")

#include "qifexporter.moc"
