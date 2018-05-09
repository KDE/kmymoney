/***************************************************************************
                             reportsview.h
                             -------------------
    copyright            : (C) 2018 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef REPORTSVIEW_H
#define REPORTSVIEW_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class KReportsView;

class ReportsView : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT

public:
  explicit ReportsView(QObject *parent, const QVariantList &args);
  ~ReportsView() final;

  void plug() final override;
  void unplug() final override;

private:
  KReportsView* m_view;
};

#endif
