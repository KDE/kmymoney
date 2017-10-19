/***************************************************************************
                          ofximporterplugin.h
                             -------------------
    begin                : Sat Jan 01 2005
    copyright            : (C) 2005 by Ace Jones
    email                : Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OFXIMPORTERPLUGIN_H
#define OFXIMPORTERPLUGIN_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Library Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"

/**
@author Ace Jones
*/
class MyMoneyStatement;
class OfxImporterPlugin : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::ImporterPlugin, public KMyMoneyPlugin::OnlinePlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.kmymoney.plugins.ofximport" FILE "ofximport.json")
  Q_INTERFACES(KMyMoneyPlugin::ImporterPlugin)
  Q_INTERFACES(KMyMoneyPlugin::OnlinePlugin)
  
public:
  explicit OfxImporterPlugin();
  ~OfxImporterPlugin();

  /**
    * This method returns the english-language name of the format
    * this plugin imports, e.g. "OFX"
    *
    * @return QString Name of the format
    */
  virtual QString formatName() const;

  /**
    * This method returns the filename filter suitable for passing to
    * KFileDialog::setFilter(), e.g. "*.ofx *.qfx" which describes how
    * files of this format are likely to be named in the file system
    *
    * @return QString Filename filter string
    */
  virtual QString formatFilenameFilter() const;

  /**
    * This method returns whether this plugin is able to import
    * a particular file.
    *
    * @param filename Fully-qualified pathname to a file
    *
    * @return bool Whether the indicated file is importable by this plugin
    */
  virtual bool isMyFormat(const QString& filename) const;

  /**
    * Import a file
    *
    * @param filename File to import
    *
    * @return bool Whether the import was successful.
    */
  virtual bool import(const QString& filename);

  /**
    * Returns the error result of the last import
    *
    * @return QString English-language name of the error encountered in the
    *  last import, or QString() if it was successful.
    *
    */
  virtual QString lastError() const;

  /**
    * Returns a pointer to a widget that will be added as tab to
    * the account edit dialog. @sa KNewAccountDlg. The data of the
    * current account is passed as const reference @a acc. @a name references
    * a QString that will receive the name of the tab to be shown in the dialog.
    */
  QWidget* accountConfigTab(const MyMoneyAccount& acc, QString& name);

  /**
    * Retrieves the online banking settings and updates the password in the KDE wallet.
    * The caller has the choice to pass a MyMoneyKeyValueContainer with the @a current
    * settings. Only those are modified that are used by the plugin.
    */
  MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current);

  const MyMoneyAccount& account(const QString& key, const QString& value) const;

  void protocols(QStringList& protocolList) const;

  bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& settings);
  bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts);

protected slots:
  void slotImportFile();
  void slotImportFile(const QString& url);

protected:
  void createActions();
  void addnew();
  MyMoneyStatement& back();
  bool isValid() const;
  void setValid();
  void addInfo(const QString& _msg);
  void addWarning(const QString& _msg);
  void addError(const QString& _msg);
  const QStringList& infos() const;       // krazy:exclude=spelling
  const QStringList& warnings() const;
  const QStringList& errors() const;

  bool storeStatements(QList<MyMoneyStatement>& statements);
  bool importStatement(const MyMoneyStatement& s);


  static int ofxTransactionCallback(struct OfxTransactionData, void*);
  static int ofxStatementCallback(struct OfxStatementData, void*);
  static int ofxAccountCallback(struct OfxAccountData, void*);
  static int ofxStatusCallback(struct OfxStatusData, void*);
  static int ofxSecurityCallback(struct OfxSecurityData, void*);

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

#endif
