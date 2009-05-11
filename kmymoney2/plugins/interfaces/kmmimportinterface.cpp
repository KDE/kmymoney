/***************************************************************************
                          kmmimportinterface.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../../kmymoney2.h"
#include "kmmimportinterface.h"

KMyMoneyPlugin::KMMImportInterface::KMMImportInterface(KMyMoney2App* app, QObject* parent, const char* name) :
  ImportInterface(parent, name),
  m_app(app)
{
}

KUrl KMyMoneyPlugin::KMMImportInterface::selectFile(const QString& title, const QString& path, const QString& mask, KFile::Mode mode) const
{
  return m_app->selectFile(title, path, mask, mode);
}

#include "kmmimportinterface.moc"
