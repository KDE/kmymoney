/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYWEBPAGE_H
#define KMYMONEYWEBPAGE_H

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#ifdef ENABLE_WEBENGINE
#include <QWebEnginePage>
#else
#include <KWebPage>
#endif

// ----------------------------------------------------------------------------
// Project Includes


#ifdef ENABLE_WEBENGINE
class MyQWebEnginePage : public QWebEnginePage
#else
class MyQWebEnginePage : public KWebPage
#endif
{
  Q_OBJECT

public:
#ifdef ENABLE_WEBENGINE
  explicit MyQWebEnginePage(QObject* parent = nullptr) : QWebEnginePage(parent){}
#else
  explicit MyQWebEnginePage(QObject* parent = nullptr) : KWebPage(parent){}
#endif

protected:
#ifdef ENABLE_WEBENGINE
  bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool) final override;
#else
  bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type) final override;
#endif

};
#endif
