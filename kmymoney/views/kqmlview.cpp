/*
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kqmlview.h"
#include "kqmlview_p.h"

// ----------------------------------------------------------------------------
// QT Includes
#include "homemodel.h"
#include <QDebug>
#include <QFile>
#include <QLabel>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>
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
    qWarning() << "KQmlView::KQmlView(QWidget*)";
}

KQmlView::KQmlView(KQmlViewPrivate& dd, QWidget* parent)
    : KMyMoneyViewBase(dd, parent)
{
    Q_D(KQmlView);
    qWarning() << "KQmlView::KQmlView(KQmlViewPrivate&, QWidget*)";
    d->init();
}

KQmlView::KQmlView(const QUrl& url, QWidget* parent)
    : KQmlView(parent)
{
    qWarning() << "KQmlView::KQmlView(const QUrl&, QWidget*) - url:" << url;
    setUrl(url);
}

void KQmlView::setUrl(const QUrl& url)
{
    Q_D(KQmlView);
    qWarning() << "Setting QML source to" << url;
    if (d->m_quickWidget) {
        d->m_quickWidget->setSource(url);
        qWarning() << "Immediate status after setSource:" << d->m_quickWidget->status();
    } else {
        qWarning() << "m_quickWidget is NULL in setUrl!";
    }
}

void KQmlView::delayedRefresh()
{
    Q_D(KQmlView);
    qWarning() << "KQmlView::delayedRefresh()";
    d->m_refreshDelayTimer.start();
}

void KQmlView::refresh()
{
    Q_D(KQmlView);

    qWarning() << "KQmlView::refresh() - isVisible:" << isVisible() << "m_skipRefresh:" << d->m_skipRefresh << "Status:" << d->m_quickWidget->status();

    if (d->m_skipRefresh) {
        qWarning() << "KQmlView::refresh() - Skipping refresh (m_skipRefresh is true)";
        d->m_needsRefresh = true;
        return;
    }

    d->m_homeModel->clear();

    const auto baseCurrency = MyMoneyFile::instance()->baseCurrency();
    if (baseCurrency.id().isEmpty()) {
        qWarning() << "KQmlView::refresh() - No file open (base currency empty)";
        d->m_homeModel->setReady(false);
        return;
    }

    qWarning() << "KQmlView::refresh() - Populating model";

    auto settings = KMyMoneySettings::self();
    QStringList items = settings->listOfItems();
    qWarning() << "Home View items from settings:" << items;

    if (items.isEmpty()) {
        qWarning() << "Items list is empty, using defaults";
        items << QStringLiteral("2") << QStringLiteral("3") << QStringLiteral("8") << QStringLiteral("1");
    }

    for (const QString& item : items) {
        int type = item.toInt();
        qWarning() << "Processing item type:" << type;
        if (type == 2) { // Preferred accounts (matches KHomeView case 2)
            qWarning() << "Adding Preferred Accounts section";
            d->m_homeModel->addSection(new AccountsSection(i18n("Preferred Accounts"), AccountsSection::PreferredAccounts));
        } else if (type == 3) { // Payment accounts (matches KHomeView case 3)
            qWarning() << "Adding Payment Accounts section";
            d->m_homeModel->addSection(new AccountsSection(i18n("Payment Accounts"), AccountsSection::PaymentAccounts));
        } else if (type == 8) { // Assets & Liabilities (matches KHomeView case 8)
            qWarning() << "Adding Assets & Liabilities section";
            d->m_homeModel->addSection(new AssetsLiabilitiesSection(i18n("Assets & Liabilities")));
        } else if (type == 1) { // Schedules (matches KHomeView case 1)
            qWarning() << "Adding Scheduled Payments section";
            d->m_homeModel->addSection(new SchedulesSection(i18n("Scheduled Payments")));
        }
    }

    d->m_homeModel->setReady(true);
    d->m_homeModel->refresh();

    d->m_needsRefresh = false;
}

void KQmlView::showEvent(QShowEvent* event)
{
    Q_D(KQmlView);

    if (d->m_needLoad)
        d->init();

    if (d->m_needsRefresh)
        refresh();

    QWidget::showEvent(event);
    qWarning() << "KQmlView::showEvent() - size:" << size() << "visible:" << isVisible() << "objectName:" << objectName();

    // Check parents
    QWidget* p = parentWidget();
    while (p) {
        qWarning() << "  Parent:" << p << "class:" << p->metaObject()->className() << "visible:" << p->isVisible() << "geometry:" << p->geometry();
        p = p->parentWidget();
    }
}

void KQmlView::resizeEvent(QResizeEvent* event)
{
    KMyMoneyViewBase::resizeEvent(event);
    qWarning() << "KQmlView::resizeEvent() - oldSize:" << event->oldSize() << "newSize:" << event->size();
}
void KQmlView::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    Q_UNUSED(selections)

    switch (action) {
    case eMenu::Action::FileNew:
    case eMenu::Action::FileClose:
        refresh();
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
    qWarning() << "QML STATUS CHANGED: status=" << status << "for" << d->m_quickWidget->source().toString();
    if (status == QQuickWidget::Status::Error) {
        qCritical() << "Error loading QML item from" << d->m_quickWidget->source().toString();
        for (const QQmlError& error : d->m_quickWidget->errors()) {
            qCritical() << error.toString();
        }
    } else if (status == QQuickWidget::Status::Ready) {
        qWarning() << "QML is READY. Model rowCount:" << d->m_homeModel->rowCount() << "Ready state:" << d->m_homeModel->isReady();
        if (d->m_quickWidget->rootObject()) {
            qWarning() << "Root object:" << d->m_quickWidget->rootObject() << "visible:" << d->m_quickWidget->rootObject()->property("visible")
                       << "width:" << d->m_quickWidget->rootObject()->property("width") << "height:" << d->m_quickWidget->rootObject()->property("height")
                       << "opacity:" << d->m_quickWidget->rootObject()->property("opacity");
            qWarning() << "Root object children count:" << d->m_quickWidget->rootObject()->childItems().count();
            for (auto* child : d->m_quickWidget->rootObject()->childItems()) {
                qWarning() << "  Child:" << child << "visible:" << child->isVisible() << "opacity:" << child->opacity() << "z:" << child->z();
            }
        } else {
            qWarning() << "No root object!";
        }
    }
}
