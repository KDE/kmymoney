/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz lukasz.wojnilowicz @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KCM_REPORTSVIEW_H
#define KCM_REPORTSVIEW_H

#include <QWidget>
#include <QScopedPointer>

#include "kmm_kcmodule.h"

class ReportsViewSettingsWidgetPrivate;
class ReportsViewSettingsWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ReportsViewSettingsWidget)

public:
    explicit ReportsViewSettingsWidget(QWidget* parent = nullptr);
    ~ReportsViewSettingsWidget();

private:
    ReportsViewSettingsWidgetPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(ReportsViewSettingsWidget)

private Q_SLOTS:
    void slotCssUrlSelected(const QUrl&);
    void slotEditingFinished();
};

class KCMReportsView : public KMMKCModule
{
public:
    explicit KCMReportsView(QObject* parent, const QVariantList& args = QVariantList());
    ~KCMReportsView();
};

#endif

