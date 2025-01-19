/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "passstore.h"
#include "config-kmymoney.h"

#include <algorithm>

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QLineEdit>
#include <QMouseEvent>
#include <QTextStream>
#include <QToolButton>

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
    explicit PassStorePrivate(PassStore* qq)
        : q_ptr(qq)
        , m_lineEdit(nullptr)
        , m_loadPasswordAction(nullptr)
        , m_passwordButton(nullptr)
    {
    }

    PassStore* q_ptr;
    QLineEdit* m_lineEdit;
    QAction* m_loadPasswordAction;
    QToolButton* m_passwordButton;
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

    const auto buttons = d->m_lineEdit->findChildren<QToolButton*>();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const auto actionButtons = d->m_loadPasswordAction->associatedObjects();
#else
    const auto actionButtons = d->m_loadPasswordAction->associatedWidgets();
#endif
    std::for_each(buttons.cbegin(), buttons.cend(), [&](QToolButton* button) {
        if (actionButtons.contains(button)) {
            d->m_passwordButton = button;
            button->installEventFilter(this);
        }
    });

#ifdef ENABLE_GPG
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

bool PassStore::eventFilter(QObject* o, QEvent* event)
{
    Q_D(PassStore);
    if ((o == d->m_passwordButton) && (event->type() == QEvent::MouseButtonDblClick)) {
        Q_EMIT doubleClicked();
        return true;
    }
    return QObject::eventFilter(o, event);
}

void PassStore::setPasswordId(const QString& id)
{
    Q_D(PassStore);
    d->m_passwordId = id;

    // replace directory separators
    d->m_passwordId.replace(QLatin1Char('/'), QLatin1Char('_'));
    d->m_passwordId.replace(QLatin1Char('\\'), QLatin1Char('_'));

    // control visibility of icon
    bool visible = false;
#ifdef ENABLE_GPG
    if (KGPGFile::GPGAvailable() && !id.isEmpty()) {
        QFileInfo fi(d->passwordFile());
        visible = (fi.exists() && fi.isReadable());
    }
#endif
    d->m_loadPasswordAction->setVisible(visible);
}

QString PassStore::passwordId() const
{
    Q_D(const PassStore);
    return d->m_passwordId;
}

bool PassStore::isActionVisible() const
{
    Q_D(const PassStore);
    return d->m_loadPasswordAction->isVisible();
}
