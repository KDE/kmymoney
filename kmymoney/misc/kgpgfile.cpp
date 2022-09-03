/*
    SPDX-FileCopyrightText: 2004, 2005, 2009 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kgpgfile.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QSaveFile>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <qglobal.h>
#include <vector>

// ----------------------------------------------------------------------------
// KDE Includes

#ifdef ENABLE_GPG
#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/encryptionresult.h>
#include <gpgme++/engineinfo.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>

class GPGConfig
{
private:
    GPGConfig()
        : m_isInitialized(false)
    {
        GpgME::initializeLibrary();

        auto ctx = GpgME::Context::createForProtocol(GpgME::OpenPGP);
        if (!ctx) {
            qDebug("Failed to create the GpgME context for the OpenPGP protocol");
            return;
        }

        // we search the directory that GPG provides as default
        if (ctx->engineInfo().homeDirectory() == nullptr) {
            m_homeDir = QString::fromUtf8(GpgME::dirInfo("homedir"));
        } else {
            m_homeDir = QString::fromUtf8(ctx->engineInfo().homeDirectory());
        }

        // search secret keys location
        // GPG >= 2.1     - private-keys-v1.d subdirectory
        // older versions - secring.pgp
        const auto secKeyDir = QStringLiteral("%1/private-keys-v1.d").arg(m_homeDir);
        if (!QFileInfo::exists(secKeyDir)) {
            const auto fileName = QString("%1/%2").arg(m_homeDir, "secring.gpg");
            qDebug() << "GPG search" << fileName;
            if (!QFileInfo::exists(fileName)) {
                qDebug() << "GPG no secure keyring found.";
            }
        }

        m_homeDir = QDir::toNativeSeparators(m_homeDir);
        /// FIXME This might be nasty if the underlying gpgme lib does not work on UTF-8
        auto lastError = ctx->setEngineHomeDirectory(m_homeDir.toUtf8());
        if (lastError.encodedError()) {
            qDebug() << "Failure while setting GPG home directory to" << m_homeDir << "\n" << QLatin1String(lastError.asString());
        }

        qDebug() << "GPG Home directory located in" << ctx->engineInfo().homeDirectory();
        qDebug() << "GPG binary located in" << ctx->engineInfo().fileName();

        m_isInitialized = true;
    }

    QString m_homeDir;
    bool m_isInitialized;

public:
    static GPGConfig* instance()
    {
        static GPGConfig* gpgConfig = nullptr;
        if (!gpgConfig) {
            gpgConfig = new GPGConfig;
        }
        return gpgConfig;
    }

    bool isInitialized() const
    {
        return m_isInitialized;
    }

    QString homeDir() const
    {
        return m_homeDir;
    }
};

class KGPGFile::Private
{
public:
    Private()
        : m_fileRead(nullptr)
        , m_fileWrite(nullptr)
        , m_ctx(nullptr)
    {
        const auto gpgConfig(GPGConfig::instance());

        if (!gpgConfig->isInitialized()) {
            qDebug() << "GPGConfig not initialized";
            return;
        }

        m_ctx = GpgME::Context::createForProtocol(GpgME::OpenPGP);
        if (!m_ctx) {
            qDebug("Failed to create the GpgME context for the OpenPGP protocol");
            return;
        }

        /// FIXME This might be nasty if the underlying gpgme lib does not work on UTF-8
        m_lastError = m_ctx->setEngineHomeDirectory(QDir::toNativeSeparators(gpgConfig->homeDir()).toUtf8());
        if (m_lastError.encodedError()) {
            qDebug() << "Failure while setting GPG home directory to" << gpgConfig->homeDir() << "\n" << QLatin1String(m_lastError.asString());
        }
    }

    ~Private()
    {
        delete m_ctx;
    }

    QString m_fn;
    QFile* m_fileRead;
    QSaveFile* m_fileWrite;

    GpgME::Error m_lastError;

    GpgME::Context* m_ctx;
    GpgME::Data m_data;

    std::vector<GpgME::Key> m_recipients;

    // the result set of the last key list job
    std::vector<GpgME::Key> m_keys;
};

KGPGFile::KGPGFile(const QString& fn, const QString& homedir, const QString& options)
    : d(new Private)
{
    // only kept for interface compatibility
    Q_UNUSED(homedir);
    Q_UNUSED(options);

    KGPGFile::setFileName(fn);
}

KGPGFile::~KGPGFile()
{
    close();
    delete d;
}

void KGPGFile::setFileName(const QString& fn)
{
    d->m_fn = fn;
    if (!fn.isEmpty() && fn[0] == '~') {
        d->m_fn = QDir::homePath() + fn.mid(1);

    } else if (QDir::isRelativePath(d->m_fn)) {
        QDir dir(fn);
        d->m_fn = dir.absolutePath();
    }
    // qDebug("setName: '%s'", d->m_fn.toLatin1().data());
}

void KGPGFile::flush()
{
    // no functionality
}

void KGPGFile::addRecipient(const QString& recipient)
{
    // skip a possible leading 0x in the id
    QString cmp = recipient;
    if (cmp.startsWith(QLatin1String("0x")))
        cmp = cmp.mid(2);

    QStringList keylist;
    keyList(keylist, false, cmp);

    if (d->m_keys.size() > 0)
        d->m_recipients.push_back(d->m_keys.front());
}

bool KGPGFile::open(OpenMode mode)
{
    if (isOpen()) {
        return false;
    }

    if (d->m_fn.isEmpty()) {
        setOpenMode(NotOpen);
        return false;
    }

    if (!d->m_ctx) {
        setOpenMode(NotOpen);
        return false;
    }

    setOpenMode(mode);

    if (!(isReadable() || isWritable())) {
        setOpenMode(NotOpen);
        return false;
    }

    if (isWritable()) {
        if (d->m_recipients.empty()) {
            setOpenMode(NotOpen);
            return false;
        }

        // write out in ASCII armor mode
        d->m_ctx->setArmor(true);
        d->m_fileWrite = new QSaveFile;

    } else if (isReadable()) {
        d->m_fileRead = new QFile;
    }

    // open the 'physical' file
    // Since some of the methods in QFile are not virtual, we need to
    // differentiate here between the QFile* and the QSaveFile* case
    if (isReadable()) {
        d->m_fileRead->setFileName(d->m_fn);
        if (!d->m_fileRead->open(mode)) {
            setOpenMode(NotOpen);
            return false;
        }
        GpgME::Data dcipher(d->m_fileRead->handle());
        d->m_lastError = d->m_ctx->decrypt(dcipher, d->m_data).error();
        if (d->m_lastError.encodedError()) {
            return false;
        }
        d->m_data.seek(0, SEEK_SET);

    } else if (isWritable()) {
        d->m_fileWrite->setFileName(d->m_fn);
        if (!d->m_fileWrite->open(mode)) {
            setOpenMode(NotOpen);
            return false;
        }
    }

    return true;
}

void KGPGFile::close()
{
    if (!isOpen()) {
        return;
    }

    if (!d->m_ctx)
        return;

    if (isWritable()) {
        d->m_data.seek(0, SEEK_SET);
        GpgME::Data dcipher(d->m_fileWrite->handle());
        d->m_lastError = d->m_ctx->encrypt(d->m_recipients, d->m_data, dcipher, GpgME::Context::AlwaysTrust).error();
        if (d->m_lastError.encodedError()) {
            setErrorString(QLatin1String("Failure while writing temporary file for file: '") + QLatin1String(d->m_lastError.asString()) + QLatin1String("'"));
        } else if (!d->m_fileWrite->commit()) {
            setErrorString("Failure while committing file changes.");
        }
    }

    delete d->m_fileWrite;
    delete d->m_fileRead;
    d->m_fileWrite = 0;
    d->m_fileRead = 0;
    d->m_recipients.clear();
    setOpenMode(NotOpen);
}

qint64 KGPGFile::writeData(const char* data, qint64 maxlen)
{
    if (!isOpen())
        return EOF;

    if (!isWritable())
        return EOF;

    // qDebug("write %d bytes", qint32(maxlen & 0xFFFFFFFF));

    // write out the data and make sure that we do not cross
    // size_t boundaries.
    qint64 bytesWritten = 0;
    while (maxlen) {
        qint64 len = 1LL << 31;
        if (len > maxlen)
            len = maxlen;
        bytesWritten += d->m_data.write(data, len);
        data = &data[len];
        maxlen -= len;
    }
    // qDebug("%d bytes written", qint32(bytesWritten & 0xFFFFFFFF));
    return bytesWritten;
}

qint64 KGPGFile::readData(char* data, qint64 maxlen)
{
    if (maxlen == 0)
        return 0;

    if (!isOpen())
        return EOF;
    if (!isReadable())
        return EOF;

    // read requested block of data and make sure that we do not cross
    // size_t boundaries.
    qint64 bytesRead = 0;
    while (maxlen) {
        qint64 len = 1LL << 31;
        if (len > maxlen)
            len = maxlen;
        bytesRead += d->m_data.read(data, len);
        data = &data[len];
        maxlen -= len;
    }
    return bytesRead;
}

QString KGPGFile::errorToString() const
{
    return QString::fromUtf8(d->m_lastError.asString());
}

bool KGPGFile::GPGAvailable()
{
    GpgME::initializeLibrary();
    const auto engineCheck = GpgME::checkEngine(GpgME::OpenPGP);
    if (engineCheck.code() != 0) {
        qDebug() << "GpgME::checkEngine returns" << engineCheck.code() << engineCheck.asString();
        return false;
    }
    return true;
}

bool KGPGFile::keyAvailable(const QString& name)
{
    KGPGFile file;
    QStringList keys;
    file.keyList(keys, false, name);
    // qDebug("keyAvailable returns %d for '%s'", keys.count(), qPrintable(name));
    return keys.count() != 0;
}

void KGPGFile::publicKeyList(QStringList& list)
{
    // qDebug("Reading public keys");
    KGPGFile file;
    file.keyList(list);
}

void KGPGFile::secretKeyList(QStringList& list)
{
    // qDebug("Reading secrect keys");
    KGPGFile file;
    file.keyList(list, true);
}

QDateTime KGPGFile::keyExpires(const QString& name)
{
    QDateTime expirationDate;

    // skip a possible leading 0x in the id
    QString cmp = name;
    if (cmp.startsWith(QLatin1String("0x")))
        cmp = cmp.mid(2);

    QStringList keylist;
    keyList(keylist, false, cmp);

    // in case we have no or more than one matching key
    // or the key does not have subkeys, we return an invalid date
    if (d->m_keys.size() == 1 && d->m_keys[0].subkeys().size() > 0 && !d->m_keys[0].subkeys()[0].neverExpires()) {
        expirationDate.setSecsSinceEpoch(d->m_keys[0].subkeys()[0].expirationTime());
    }
    return expirationDate;
}

void KGPGFile::keyList(QStringList& list, bool secretKeys, const QString& pattern)
{
    d->m_keys.clear();
    list.clear();
    if (d->m_ctx && !d->m_ctx->startKeyListing(pattern.toUtf8().constData(), secretKeys)) {
        GpgME::Error error;
        for (;;) {
            GpgME::Key key;
            key = d->m_ctx->nextKey(error);
            if (error.encodedError() != GPG_ERR_NO_ERROR)
                break;

            bool needPushBack = true;

            std::vector<GpgME::UserID> userIDs = key.userIDs();
            std::vector<GpgME::Subkey> subkeys = key.subkeys();
            for (unsigned int i = 0; i < userIDs.size(); ++i) {
                if (subkeys.size() > 0) {
                    for (unsigned int j = 0; j < subkeys.size(); ++j) {
                        const GpgME::Subkey& skey = subkeys[j];

                        if (((skey.canEncrypt() && !secretKeys) || (skey.isSecret() && secretKeys))

                            && !(skey.isRevoked() || skey.isExpired() || skey.isInvalid() || skey.isDisabled())) {
                            QString entry = QString("%1:%2").arg(key.shortKeyID()).arg(userIDs[i].id());
                            list += entry;
                            if (needPushBack) {
                                d->m_keys.push_back(key);
                                needPushBack = false;
                            }
                        } else {
                            // qDebug("Skip key '%s'", key.shortKeyID());
                        }
                    }
                } else {
                    // we have no subkey, so we operate on the main key
                    if (((key.canEncrypt() && !secretKeys) || (key.hasSecret() && secretKeys))
                        && !(key.isRevoked() || key.isExpired() || key.isInvalid() || key.isDisabled())) {
                        QString entry = QString("%1:%2").arg(key.shortKeyID()).arg(userIDs[i].id());
                        list += entry;
                        if (needPushBack) {
                            d->m_keys.push_back(key);
                            needPushBack = false;
                        }
                    } else {
                        // qDebug("Skip key '%s'", key.shortKeyID());
                    }
                }
            }
        }
        d->m_ctx->endKeyListing();
    }
}

#else // not ENABLE_GPG

// NOOP implementation
KGPGFile::KGPGFile(const QString& fn, const QString& homedir, const QString& options)
    : d(0)
{
    Q_UNUSED(fn);
    Q_UNUSED(homedir);
    Q_UNUSED(options);
}

KGPGFile::~KGPGFile()
{
}

bool KGPGFile::open(OpenMode mode)
{
    Q_UNUSED(mode);
    return false;
}

void KGPGFile::close()
{
}

void KGPGFile::flush()
{
}

qint64 KGPGFile::readData(char* data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0;
}

qint64 KGPGFile::writeData(const char* data, qint64 maxlen)
{
    Q_UNUSED(data);
    Q_UNUSED(maxlen);
    return 0;
}

void KGPGFile::addRecipient(const QString& recipient)
{
    Q_UNUSED(recipient);
}

QString KGPGFile::errorToString() const
{
    return QString();
}

bool KGPGFile::GPGAvailable(void)
{
    return false;
}

bool KGPGFile::keyAvailable(const QString& name)
{
    Q_UNUSED(name);
    return false;
}

void KGPGFile::secretKeyList(QStringList& list)
{
    Q_UNUSED(list);
}

void KGPGFile::publicKeyList(QStringList& list)
{
    Q_UNUSED(list);
}

QDateTime KGPGFile::keyExpires(const QString& name)
{
    Q_UNUSED(name);
    return QDateTime();
}

#endif
