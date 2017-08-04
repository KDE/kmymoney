/***************************************************************************
                             qifexporterplugin.h
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

#ifndef QIFEXPORTERPLUGIN_H
#define QIFEXPORTERPLUGIN_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class MyMoneyQifReader;

class QIFExporterPlugin : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kmymoney.plugins.qifexport" FILE "qifexport.json")
  
public:
  explicit QIFExporterPlugin();
  ~QIFExporterPlugin();

  QAction          *m_action;

  MyMoneyQifReader *m_qifReader;

public slots:
  /**
    * Called when the user wishes to export some transaction to a
    * QIF formatted file. An account must be open for this to work.
    * Uses MyMoneyQifWriter() for the actual output.
    */
  void slotQifExport();

protected:
  void createActions();
};

#endif
