/*

    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gncimporter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFileDialog>
#include <QRegularExpression>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KMessageBox>
#include <KCompressionDevice>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneygncreader.h"
#include "viewinterface.h"
#include "appinterface.h"
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "kmymoneyenums.h"

class MyMoneyStatement;

static constexpr KCompressionDevice::CompressionType const& COMPRESSION_TYPE = KCompressionDevice::GZip;

GNCImporter::GNCImporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, metaData, args)
{
    // For information, announce that we have been loaded.
    qDebug("Plugins: gncimporter loaded");
}

GNCImporter::~GNCImporter()
{
    qDebug("Plugins: gncimporter unloaded");
}

bool GNCImporter::open(const QUrl &url)
{
    if (url.scheme() == QLatin1String("sql"))
        return false;

    if (!url.isLocalFile())
        return false;

    const auto fileName = url.toLocalFile();
    const auto sFileToShort = QString::fromLatin1("File %1 is too short.").arg(fileName);

    QScopedPointer<QIODevice> qfile(new QFile(fileName));
    if (!qfile->open(QIODevice::ReadOnly))
        throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot read the file: %1").arg(fileName));

    QByteArray qbaFileHeader(2, '\0');
    if (qfile->read(qbaFileHeader.data(), 2) != 2)
        throw MYMONEYEXCEPTION(sFileToShort);

    if (qbaFileHeader == QByteArray("\037\213")) { // gzipped?
        qfile->close();

        qfile.reset(new KCompressionDevice(fileName, COMPRESSION_TYPE));
        if (!qfile->open(QIODevice::ReadOnly))
            throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot read the file: %1").arg(fileName));
        qfile->read(qbaFileHeader.data(), 2);
    }

    if (qbaFileHeader == QByteArray("SQ"))
        throw MYMONEYEXCEPTION(QString::fromLatin1("GnuCash SQLite file format is not supported. Please save it using XML format in GnuCash and try again."));

    // Scan the first 70 bytes to see if we find something
    // we know. For now, we support our own XML format and
    // GNUCash XML format. If the file is smaller, then it
    // contains no valid data and we reject it anyway.
    qbaFileHeader.resize(70);
    if (qfile->read(qbaFileHeader.data(), 70) != 70)
        throw MYMONEYEXCEPTION(sFileToShort);

    const QRegularExpression gncVersionExp("<gnc-v(\\d+)");
    if (!(gncVersionExp.match(qbaFileHeader).hasMatch())) {
        return false;
    }

    MyMoneyGncReader pReader;
    qfile->seek(0);

    pReader.setProgressCallback(appInterface()->progressCallback());
    pReader.readFile(qfile.data(), MyMoneyFile::instance());
    pReader.setProgressCallback(0);

    qfile->close();
    return true;
}

QUrl GNCImporter::openUrl() const
{
    return QUrl();
}

bool GNCImporter::save(const QUrl &url)
{
    Q_UNUSED(url)
    return false;
}

bool GNCImporter::saveAs()
{
    return false;
}

eKMyMoney::StorageType GNCImporter::storageType() const
{
    return eKMyMoney::StorageType::GNC;
}

QString GNCImporter::fileExtension() const
{
    return i18n("GnuCash files (*.gnucash *.xac *.gnc)");
}

K_PLUGIN_CLASS_WITH_JSON(GNCImporter, "gncimporter.json")

#include "gncimporter.moc"
