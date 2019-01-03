/***************************************************************************
                             gncimporter.h
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz
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

#ifndef GNCIMPORTER_H
#define GNCIMPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class MyMoneyGncReader;

class GNCImporter : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::StoragePlugin
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::StoragePlugin)

public:
  explicit GNCImporter(QObject *parent, const QVariantList &args);
  ~GNCImporter() override;

  MyMoneyStorageMgr *open(const QUrl &url) override;
  bool save(const QUrl &url) override;
  bool saveAs() override;

  eKMyMoney::StorageType storageType() const override;
  QString fileExtension() const override;
  QUrl openUrl() const override;
};

#endif
