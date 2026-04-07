/*
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "kmymoneyviewbase_p.h"
#include "kqmlview.h"

#include <ki18n_version.h>
#if KI18N_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <KLocalizedQmlContext>
#else
#include <KLocalizedContext>
#endif
#include <QDebug>
#include <QLabel>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSurfaceFormat>
#include <QTimer>
#include <QVBoxLayout>
#include <QtQml>

#include "homemodel.h"

class HomeModel;
class MoneyFormatter;

class KQmlViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KQmlView)

public:
    explicit KQmlViewPrivate(KQmlView* parent)
        : KMyMoneyViewBasePrivate(parent)
        , m_skipRefresh(false)
        , m_fileOpen(false)
        , m_needLoad(true)
        , m_quickWidget(nullptr)
        , m_homeModel(nullptr)
        , m_moneyFormatter(nullptr)
    {
    }

    void init()
    {
        Q_Q(KQmlView);
        if (!m_needLoad)
            return;
        m_needLoad = false;
        qWarning() << "KQmlViewPrivate::init() - initializing QQuickWidget";

        // Enable verbose QML import tracing and debugging
        qputenv("QML_IMPORT_TRACE", "1");
        qputenv("QT_LOGGING_RULES", "qt.qml.import.debug=true;qt.quick.dirty=true;qt.quick.renderloop=true");

        // Force software rendering as a diagnostic to rule out OpenGL/context issues
        // qWarning() << "Forcing software rendering for QML";
        // QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);

        QFile qmlFile(":/qml/HomeView.qml");
        qWarning() << "Checking file :/qml/HomeView.qml - exists:" << qmlFile.exists() << "size:" << qmlFile.size();

        auto layout = new QVBoxLayout(q);
        q->setLayout(layout);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        m_quickWidget = new QQuickWidget(q);
        m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
        m_quickWidget->setClearColor(Qt::green); // Keep green for diagnostic purposes
        m_quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
        m_quickWidget->engine()->setOutputWarningsToStandardError(true);
        m_quickWidget->setFocusPolicy(Qt::StrongFocus);
        m_quickWidget->setFormat(QSurfaceFormat::defaultFormat()); // Ensure surface format is standard
        // Ensure the widget is visible and has a layout
        m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        q->connect(m_quickWidget, &QQuickWidget::statusChanged, q, &KQmlView::qmlStatusChanged);

        layout->addWidget(m_quickWidget);

        m_homeModel = new HomeModel(q);
        m_moneyFormatter = new MoneyFormatter(q);

        qWarning() << "QQmlEngine import paths:" << m_quickWidget->engine()->importPathList();

        // TODO: anchor handling
        // q->connect(m_view, &QTextBrowser::anchorClicked, q, &KHomeView::slotOpenUrl);
#if KI18N_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        m_quickWidget->rootContext()->setContextObject(KLocalization::setupLocalizedContext(m_quickWidget->engine()));
#else
        m_quickWidget->rootContext()->setContextObject(new KLocalizedContext(m_quickWidget->engine()));
#endif
        m_quickWidget->rootContext()->setContextProperty("homeModel", m_homeModel);
        m_quickWidget->rootContext()->setContextProperty("moneyFormatter", m_moneyFormatter);

        // If a source was already set (via setUrl before init), re-set it now that context is ready
        if (!m_quickWidget->source().isEmpty()) {
            QUrl source = m_quickWidget->source();
            m_quickWidget->setSource(QUrl());
            m_quickWidget->setSource(source);
        }

        q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KQmlView::delayedRefresh);

        m_refreshDelayTimer.setSingleShot(true);
        m_refreshDelayTimer.setInterval(100);
        q->connect(&m_refreshDelayTimer, &QTimer::timeout, q, &KQmlView::refresh);

        m_needsRefresh = false;
    }

    bool m_skipRefresh;
    bool m_fileOpen;
    bool m_needLoad;

    QQuickWidget* m_quickWidget;
    HomeModel* m_homeModel;
    MoneyFormatter* m_moneyFormatter;

    QTimer m_refreshDelayTimer;
};
