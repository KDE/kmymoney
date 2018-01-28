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

class GNCImporter : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT

public:
  explicit GNCImporter(QObject *parent, const QVariantList &args);
  ~GNCImporter() override;

  QAction          *m_action;

private:
  MyMoneyGncReader *m_gncReader;

private Q_SLOTS:

  /**
    * Called when the user wishes to import tab delimeted transactions
    * into the current account.  An account must be open for this to
    * work.  Calls KMyMoneyView::slotAccountImportAscii.
    *
    * @see MyMoneyAccount
    */
  void slotGNCImport();

protected:
  void createActions();
};

#endif
