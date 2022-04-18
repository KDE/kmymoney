/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "passstore.h"
#include "config-kmymoney.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QLineEdit>
#include <QTextStream>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "kgpgfile.h"

using namespace Icons;

class PassStorePrivate
{
public:
    PassStorePrivate(PassStore* qq)
        : q_ptr(qq)
        , m_lineEdit(nullptr)
        , m_loadPasswordAction(nullptr)
    {
    }

    PassStore* q_ptr;
    QLineEdit* m_lineEdit;
    QAction* m_loadPasswordAction;
    QString m_applicationPrefix;
    QString m_passwordId;

    QString passwordFile()
    {
        return QStringLiteral("%1/.password-store/%2/%3.gpg").arg(QDir::home().absolutePath(), m_applicationPrefix, m_passwordId);
    }
};

PassStore::PassStore(QLineEdit* parent, const QString& applicationPrefix, const QString& id)
    : QObject(parent)
    , d_ptr(new PassStorePrivate(this))
{
    Q_D(PassStore);
    d->m_lineEdit = parent;
    d->m_applicationPrefix = applicationPrefix;
    d->m_loadPasswordAction = d->m_lineEdit->addAction(Icons::get(Icon::Vault), QLineEdit::TrailingPosition);
    d->m_loadPasswordAction->setToolTip(i18n("Read the password from pass store"));

    setPasswordId(id);

#if ENABLE_GPG
    connect(d->m_loadPasswordAction, &QAction::triggered, this, [&]() {
        Q_D(PassStore);
        KGPGFile passwordFile(d->passwordFile());
        if (passwordFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&passwordFile);
            const auto pwd = stream.readLine();
            d->m_lineEdit->clear();
            d->m_lineEdit->setText(pwd);
        }
    });
#endif
}

PassStore::~PassStore()
{
    Q_D(PassStore);
    delete d;
}

void PassStore::setPasswordId(const QString& id)
{
    Q_D(PassStore);
    d->m_passwordId = id;

    // control visibility of icon
    bool visible = false;
#if ENABLE_GPG
    if (KGPGFile::GPGAvailable() && !id.isEmpty()) {
        QFileInfo fi(d->passwordFile());
        visible = (fi.exists() && fi.isReadable());
    }
#endif
    d->m_loadPasswordAction->setVisible(visible);
}

bool PassStore::isActionVisible() const
{
    Q_D(const PassStore);
    return d->m_loadPasswordAction->isVisible();
}
