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

#include "kmmimportinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney.h"

KMyMoneyPlugin::KMMImportInterface::KMMImportInterface(KMyMoneyApp* app, QObject* parent, const char* name) :
    ImportInterface(parent, name),
    m_app(app)
{
}

QUrl KMyMoneyPlugin::KMMImportInterface::selectFile(const QString& title, const QString& path, const QString& mask, QFileDialog::FileMode mode, QWidget *widget) const
{
  return m_app->selectFile(title, path, mask, mode, widget);
}
