/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>

#include "kmymoneywebpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#ifdef ENABLE_WEBENGINE
bool MyQWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool)
{
  if (type == NavigationTypeLinkClicked) {
    emit urlChanged(url);
    return false;
  }
  return true;
}
#else
#include <QNetworkRequest>
bool MyQWebEnginePage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
  Q_UNUSED(frame);
  if (type == NavigationTypeLinkClicked) {
    emit linkClicked(request.url());
    return false;
  }
  return true;
}
#endif
