/*
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kqmlview.h"
#include "kqmlview_p.h"

// ----------------------------------------------------------------------------
// QT Includes
#include "homemodel.h"
#include <QLabel>
#include <QQmlContext>
#include <QTimer>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneysettings.h"
#include "mymoneyfile.h"

KQmlView::KQmlView(QWidget* parent)
    : KQmlView(*new KQmlViewPrivate(this), parent)
{
}

KQmlView::KQmlView(KQmlViewPrivate& dd, QWidget* parent)
    : KMyMoneyViewBase(dd, parent)
{
    Q_D(KQmlView);
    d->init();
}

KQmlView::KQmlView(const QUrl& url, QWidget* parent)
    : KQmlView(parent)
{
    setUrl(url);
}

void KQmlView::setUrl(const QUrl& url)
{
    Q_D(KQmlView);
    d->m_quickWidget->setSource(url);
}

void KQmlView::delayedRefresh()
{
    Q_D(KQmlView);
    d->m_refreshDelayTimer.start();
}

void KQmlView::refresh()
{
    Q_D(KQmlView);

    if (isVisible() && !d->m_skipRefresh) {
        // TODO: move to d->loadView();
        if (d->m_fileOpen) {
            d->m_homeModel->clear();

            // TODO: handle this
            // const auto baseCurrency = MyMoneyFile::instance()->baseCurrency();
            // if (baseCurrency.id().isEmpty()) {
            //     d->m_homeModel->setReady(false);
            //     return;
            // }
            d->m_homeModel->setReady(true);

            auto settings = KMyMoneySettings::self();
            QStringList items = settings->itemList().split(',', Qt::SkipEmptyParts);

            if (items.isEmpty()) {
                items << QStringLiteral("1") << QStringLiteral("2") << QStringLiteral("8") << QStringLiteral("3");
            }

            for (const QString& item : items) {
                int type = item.toInt();
                if (type == 1) { // Preferred accounts
                    d->m_homeModel->addSection(new AccountsSection(i18n("Preferred Accounts"), AccountsSection::PreferredAccounts));
                } else if (type == 2) { // Payment accounts
                    d->m_homeModel->addSection(new AccountsSection(i18n("Payment Accounts"), AccountsSection::PaymentAccounts));
                } else if (type == 8) { // Assets & Liabilities
                    d->m_homeModel->addSection(new AssetsLiabilitiesSection(i18n("Assets & Liabilities")));
                } else if (type == 3) { // Schedules
                    d->m_homeModel->addSection(new SchedulesSection(i18n("Scheduled Payments")));
                }
            }

            d->m_homeModel->refresh();

            d->m_needsRefresh = false;
        }
    } else {
        d->m_needsRefresh = true;
    }
}

void KQmlView::showEvent(QShowEvent* event)
{
    Q_D(KQmlView);

    if (d->m_needLoad)
        d->init();

    if (d->m_needsRefresh)
        refresh();

    QWidget::showEvent(event);
}
void KQmlView::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    Q_UNUSED(selections)

    Q_D(KQmlView);
    switch (action) {
    case eMenu::Action::FileNew:
        d->m_fileOpen = true;
        break;
    case eMenu::Action::FileClose:
        d->m_fileOpen = false;
        // TODO: d->loadView();
        break;
    default:
        break;
    }
}

void KQmlView::executeCustomAction(eView::Action action)
{
    switch (action) {
    case eView::Action::BlockViewDuringFileOpen:
        slotDisableRefresh();
        break;
    case eView::Action::UnblockViewAfterFileOpen:
        slotEnableRefresh();
        break;
    default:
        break;
    }
}

void KQmlView::slotSettingsChanged()
{
    delayedRefresh();
}

void KQmlView::slotDisableRefresh()
{
    Q_D(KQmlView);
    d->m_skipRefresh = true;
}

void KQmlView::slotEnableRefresh()
{
    Q_D(KQmlView);
    d->m_skipRefresh = false;
    refresh();
}

void KQmlView::qmlStatusChanged(QQuickWidget::Status status)
{
    Q_D(KQmlView);
    if (status == QQuickWidget::Status::Error) {
        qWarning() << "Error loading QML item from" << d->m_quickWidget->source().toString();
        for (const QQmlError& error : d->m_quickWidget->errors()) {
            qWarning() << error.toString();
        }
    }
}
