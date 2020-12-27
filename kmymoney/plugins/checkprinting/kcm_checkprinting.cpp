/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

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

K_PLUGIN_FACTORY_WITH_JSON(KCMCheckPrintingFactory, "kcm_checkprinting.json", registerPlugin<KCMCheckPrinting>();)

#include "kcm_checkprinting.moc"
