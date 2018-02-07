/***************************************************************************
                             csvimporter.h
                             -------------------
    begin                : Sat Jan 01 2010
    copyright            : (C) 2010 by Allan Anderson
    email                : agander93@gmail.com
    copyright            : (C) 2016-2017 by Łukasz Wojniłowicz
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

#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class CSVImporterCore;
class CSVWizard;
class MyMoneyStatement;

class CSVImporter : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::ImporterPlugin
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::ImporterPlugin)

public:
  explicit CSVImporter(QObject *parent, const QVariantList &args);
  ~CSVImporter() override;

  QAction*          m_action;
  CSVWizard*        m_wizard;
  CSVImporterCore*  m_importer;

  /**
    * This method returns the english-language name of the format
    * this plugin imports, e.g. "OFX"
    *
    * @return QString Name of the format
    */
  virtual QString formatName() const override;

  /**
    * This method returns the filename filter suitable for passing to
    * KFileDialog::setFilter(), e.g. "*.csv" which describes how
    * files of this format are likely to be named in the file system
    *
    * @return QString Filename filter string
    */
  virtual QString formatFilenameFilter() const override;

  /**
    * This method returns whether this plugin is able to import
    * a particular file.
    *
    * @param filename Fully-qualified pathname to a file
    *
    * @return bool Whether the indicated file is importable by this plugin
    */
  virtual bool isMyFormat(const QString& filename) const override;

  /**
    * Import a file
    *
    * @param filename File to import
    *
    * @return bool Whether the import was successful.
    */
  virtual bool import(const QString& filename) override;

  /**
    * Returns the error result of the last import
    *
    * @return QString English-language name of the error encountered in the
    *  last import, or QString() if it was successful.
    *
    */
  virtual QString lastError() const override;

  void injectExternalSettings(KMyMoneySettings* p) override;

private:
  bool              m_silent;
public Q_SLOTS:
  bool slotGetStatement(MyMoneyStatement& s);

protected Q_SLOTS:
  void startWizardRun();

protected:
  void createActions();
};

#endif
