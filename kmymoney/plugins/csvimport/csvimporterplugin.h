/***************************************************************************
                             csvimporterplugin.h
                             -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Allan Anderson
    email                : agander93@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CSVIMPORTERPLUGIN_H
#define CSVIMPORTERPLUGIN_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QStringList>

// Project Includes

#include "kmymoneyplugin.h"

class CSVDialog;

class CsvImporterPlugin : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT

public:
  explicit CsvImporterPlugin(QObject* parent, const QVariantList&);
  ~CsvImporterPlugin();

  KAction*          m_action;

public slots:
  bool slotGetStatement(MyMoneyStatement& s);

protected slots:
  void slotImportFile();

protected:
  void createActions();
  bool importStatement(const MyMoneyStatement& s);
  void processStatement(const MyMoneyStatement& s);
};

#endif
