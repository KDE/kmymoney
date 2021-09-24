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

#include "kmymoneyviewbase.h"

class QLabel;

class KQmlView : public KMyMoneyViewBase
{
    Q_OBJECT

public:
    explicit KQmlView(QWidget* parent = nullptr);
    KQmlView(const QUrl& url, QWidget* parent = nullptr);

    void setUrl(const QUrl& url);

private Q_SLOTS:
    void qmlStatusChanged(QQuickWidget::Status status);

private:
    QLabel* m_errorLabel;
    QQuickWidget* m_quickWidget;
};
