/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KCM_CHECKPRINTING_H
#define KCM_CHECKPRINTING_H

#include "ui_checkprintingsettingsdecl.h"
#include <config-kmymoney.h>

#include <QWidget>

// Override QUrl
#include "kmm_kcmodule.h"
#include "kmmurl.h"

class QTextEdit;
class QTextDocument;

class CheckPrintingSettingsWidget : public QWidget, public Ui::CheckPrintingSettingsDecl
{
Q_OBJECT

public:
    explicit CheckPrintingSettingsWidget(QWidget* parent = 0);
    ~CheckPrintingSettingsWidget();

public Q_SLOTS:
    void urlSelected();
    void urlSelected(const QUrl &url);
    void urlSelected(const QString& url);
    void textChanged(const QString& text);

private:
    QTextEdit *m_checkTemplatePreviewHTMLPart;
    void restoreDefaultSettings() const;
};

class KCMCheckPrinting : public KMMKCModule
{
public:
    KCMCheckPrinting(QObject* parent, const QVariantList& args = QVariantList());
    ~KCMCheckPrinting();
};

#endif

