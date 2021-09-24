/*
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kqmlview.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QLabel>
#include <QLayout>
#include <QQuickItem>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

KQmlView::KQmlView(QWidget* parent)
    : KMyMoneyViewBase(parent)
    , m_errorLabel(nullptr)
    , m_quickWidget(nullptr)
{
    auto layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setText(QStringLiteral("No QML item is yet loaded in this view"));

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setVisible(false);

    layout->addWidget(m_errorLabel);
    layout->addWidget(m_quickWidget);

    connect(m_quickWidget, &QQuickWidget::statusChanged, this, &KQmlView::qmlStatusChanged);
}

KQmlView::KQmlView(const QUrl& url, QWidget* parent)
    : KQmlView(parent)
{
    setUrl(url);
}

void KQmlView::setUrl(const QUrl& url) {
    m_quickWidget->setSource(url);
}

void KQmlView::qmlStatusChanged(QQuickWidget::Status status)
{
    QString statusText;
    const bool isQmlReady = (status == QQuickWidget::Status::Ready);

    switch(status)
    {
    case QQuickWidget::Status::Ready:
        statusText = QStringLiteral("Correctly loaded QML item from %1").arg(m_quickWidget->source().toString());
        qDebug() << "KQmlView::qmlStatusChanged():" << statusText;
        break;
    case QQuickWidget::Status::Null:
        statusText = QStringLiteral("No QML item is loaded");
        break;
    case QQuickWidget::Status::Loading:
        statusText = QStringLiteral("Loading QML item from %1").arg(m_quickWidget->source().toString());
        break;
    case QQuickWidget::Status::Error:
        statusText = QStringLiteral("Error loading QML item from %1\n").arg(m_quickWidget->source().toString());
        for (const QQmlError& error : m_quickWidget->errors()) {
            statusText += QLatin1Char('\n') + error.toString();
        }
        qDebug() << "KQmlView::qmlStatusChanged():" << statusText;
        break;
    }

    m_errorLabel->setText(statusText);
    m_errorLabel->setVisible(!isQmlReady);
    m_quickWidget->setVisible(isQmlReady);
}
