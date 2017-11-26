/***************************************************************************
                             qifimporterplugin.h
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

#ifndef QIFIMPORTERPLUGIN_H
#define QIFIMPORTERPLUGIN_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"
#include "mymoneystatement.h"

class MyMoneyQifReader;

class QIFImporterPlugin : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kmymoney.plugins.qifimport" FILE "qifimport.json")
  
public:
  explicit QIFImporterPlugin();
  ~QIFImporterPlugin();

  QAction          *m_action;

private:
  MyMoneyQifReader *m_qifReader;

private Q_SLOTS:

  /**
    * Called when the user wishes to import tab delimeted transactions
    * into the current account.  An account must be open for this to
    * work.  Calls KMyMoneyView::slotAccountImportAscii.
    *
    * @see MyMoneyAccount
    */
  void slotQifImport();

  bool slotGetStatements(QList<MyMoneyStatement> &statements);

protected:
  void createActions();
};

#endif
