/*
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

// ----------------------------------------------------------------------------
// QT Includes
#include <QQuickWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"
#include "kmymoneyviewbase.h"
#include <KLocalizedString>

class QLabel;
class HomeModel;
class MoneyFormatter;

class KQmlViewPrivate;
class KQmlView : public KMyMoneyViewBase
{
    Q_OBJECT

public:
    explicit KQmlView(QWidget* parent = nullptr);
    KQmlView(const QUrl& url, QWidget* parent = nullptr);

    void setUrl(const QUrl& url);
    void refresh();
    void delayedRefresh();

    void executeAction(eMenu::Action action, const SelectedObjects& selections) override;
    void executeCustomAction(eView::Action action) override;

protected:
    void showEvent(QShowEvent* event) override;

public Q_SLOTS:
    void slotSettingsChanged() override;
    void slotDisableRefresh();
    void slotEnableRefresh();

private Q_SLOTS:
    void qmlStatusChanged(QQuickWidget::Status status);

private:
    Q_DECLARE_PRIVATE(KQmlView)
    KQmlView(KQmlViewPrivate& dd, QWidget* parent);
};
