/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KCM_CHECKPRINTING_H
#define KCM_CHECKPRINTING_H

#include <config-kmymoney.h>

#include <KCModule>
#include <QWidget>
#include "ui_pluginsettingsdecl.h"

#ifdef ENABLE_WEBENGINE
class QWebEngineView;
#else
class KWebView;
#endif

class PluginSettingsWidget : public QWidget, public Ui::PluginSettingsDecl
{
  Q_OBJECT

public:
  explicit PluginSettingsWidget(QWidget* parent = 0);

public Q_SLOTS:
  void urlSelected(const QUrl &url);
  void returnPressed(const QString& url);

private:
  #ifdef ENABLE_WEBENGINE
  QWebEngineView *m_checkTemplatePreviewHTMLPart;
  #else
  KWebView       *m_checkTemplatePreviewHTMLPart;
  #endif
};

class KCMCheckPrinting : public KCModule
{
public:
  KCMCheckPrinting(QWidget* parent, const QVariantList& args);
  ~KCMCheckPrinting();
};

#endif

