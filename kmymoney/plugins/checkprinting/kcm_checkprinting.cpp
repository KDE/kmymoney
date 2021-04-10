/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_checkprinting.h"
#include <config-kmymoney-version.h>

// Qt includes
#include <QFrame>
#ifdef ENABLE_WEBENGINE
#include <QWebEngineView>
#else
#include <KWebView>
#endif

// KDE includes
#include <KPluginFactory>
#include <KAboutData>

#include "pluginsettings.h"

PluginSettingsWidget::PluginSettingsWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
#ifdef ENABLE_WEBENGINE
    m_checkTemplatePreviewHTMLPart = new QWebEngineView(m_previewFrame);
#else
    m_checkTemplatePreviewHTMLPart = new KWebView(m_previewFrame);
#endif

    QVBoxLayout *layout = new QVBoxLayout;
    m_previewFrame->setLayout(layout);
    layout->addWidget(m_checkTemplatePreviewHTMLPart);

    connect(kcfg_checkTemplateFile, &KUrlRequester::urlSelected, this, &PluginSettingsWidget::urlSelected);
    connect(kcfg_checkTemplateFile, QOverload<const QString&>::of(&KUrlRequester::returnPressed), this, &PluginSettingsWidget::returnPressed);
}

void PluginSettingsWidget::urlSelected(const QUrl &url)
{
    if (!url.isEmpty())
        m_checkTemplatePreviewHTMLPart->load(url);
}

void PluginSettingsWidget::returnPressed(const QString& url)
{
    if (!url.isEmpty())
        m_checkTemplatePreviewHTMLPart->load(QUrl::fromUserInput(url));
}

KCMCheckPrinting::KCMCheckPrinting(QWidget *parent, const QVariantList& args)
    : KCModule(parent, args)
{
    PluginSettingsWidget* w = new PluginSettingsWidget(this);
    addConfig(PluginSettings::self(), w);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(w);
    load();
    w->urlSelected(QUrl::fromUserInput(PluginSettings::checkTemplateFile()));
}

KCMCheckPrinting::~KCMCheckPrinting()
{
}

K_PLUGIN_CLASS_WITH_JSON(KCMCheckPrinting, "kcm_checkprinting.json")

#include "kcm_checkprinting.moc"
