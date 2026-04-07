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
#include <QLabel>
#include <QQmlContext>
#include <QQuickWidget>
#include <QTimer>
#include <QVBoxLayout>

#include "homemodel.h"

class HomeModel;
class MoneyFormatter;

class KQmlViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KQmlView)

public:
    explicit KQmlViewPrivate(KQmlView* parent)
        : KMyMoneyViewBasePrivate(parent)
        , m_quickWidget(nullptr)
        , m_homeModel(nullptr)
        , m_moneyFormatter(nullptr)
        , m_skipRefresh(false)
        , m_fileOpen(false)
        , m_needLoad(true)
    {
    }

    void init()
    {
        Q_Q(KQmlView);
        m_needLoad = false;

        auto layout = new QVBoxLayout(q);
        q->setLayout(layout);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        m_quickWidget = new QQuickWidget(q);
        m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

        layout->addWidget(m_quickWidget);

        m_homeModel = new HomeModel(q);
        m_moneyFormatter = new MoneyFormatter(q);

        // TODO: anchor handling
        // q->connect(m_view, &QTextBrowser::anchorClicked, q, &KHomeView::slotOpenUrl);
#if KI18N_VERSION >= QT_VERSION_CHECK(6, 8, 0)
        m_quickWidget->rootContext()->setContextObject(KLocalization::setupLocalizedContext(m_quickWidget->engine()));
#else
        m_quickWidget->rootContext()->setContextObject(new KLocalizedContext(m_quickWidget->engine()));
#endif
        m_quickWidget->rootContext()->setContextProperty("homeModel", m_homeModel);
        m_quickWidget->rootContext()->setContextProperty("moneyFormatter", m_moneyFormatter);

        q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KQmlView::delayedRefresh);

        m_refreshDelayTimer.setSingleShot(true);
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
