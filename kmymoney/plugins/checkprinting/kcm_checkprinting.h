/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KCM_CHECKPRINTING_H
#define KCM_CHECKPRINTING_H

#include <config-kmymoney.h>
#include "ui_pluginsettingsdecl.h"

// Override QUrl
#include <misc/kmmurl.h>

#include <KCModule>
#include <QWidget>

class QTextEdit;
class QTextDocument;

class PluginSettingsWidget : public QWidget, public Ui::PluginSettingsDecl
{
Q_OBJECT

public:
    explicit PluginSettingsWidget(QWidget* parent = 0);
    ~PluginSettingsWidget();

public Q_SLOTS:
    void urlSelected();
    void urlSelected(const QUrl &url);
    void urlSelected(const QString& url);
    void textChanged(const QString& text);

private:
    QTextEdit *m_checkTemplatePreviewHTMLPart;
    void restoreDefaultSettings() const;
};

class KCMCheckPrinting : public KCModule
{
public:
    KCMCheckPrinting(QWidget* parent, const QVariantList& args);
    ~KCMCheckPrinting();
};

#endif

