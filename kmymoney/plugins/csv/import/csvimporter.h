/*
 * Copyright 2010-2014  Allan Anderson <agander93@gmail.com>
 * Copyright 2016-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSVIMPORTER_H
#define CSVIMPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class CSVImporter : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::ImporterPlugin
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::ImporterPlugin)

public:
  explicit CSVImporter(QObject *parent, const QVariantList &args);
  ~CSVImporter() override;

  /**
    * This method returns the english-language name of the format
    * this plugin imports, e.g. "CSV"
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

private:
  bool              m_silent;

protected Q_SLOTS:
  void startWizardRun();

protected:
  void createActions();
};

#endif
