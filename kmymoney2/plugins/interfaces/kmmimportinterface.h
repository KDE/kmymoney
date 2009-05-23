/***************************************************************************
                          kmmimportinterface.h
                             -------------------
    begin                : Mon Apr 14 2008
    copyright            : (C) 2008 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMMIMPORTINTERFACE_H
#define KMMIMPORTINTERFACE_H

#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kfile.h>
#include <kurl.h>
class KMyMoney2App;

// ----------------------------------------------------------------------------
// Project Includes

#include "importinterface.h"

namespace KMyMoneyPlugin {

/**
  * This class represents the implementation of the
  * ViewInterface.
  */
class KMMImportInterface : public ImportInterface {
  Q_OBJECT

public:
  KMMImportInterface(KMyMoney2App* app, QObject* parent, const char* name = 0);
  ~KMMImportInterface() {}

  KUrl selectFile(const QString& title, const QString& path, const QString& mask, KFile::Mode mode) const;

private:
  KMyMoney2App*    m_app;
};

} // namespace
#endif
