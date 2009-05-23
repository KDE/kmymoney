/***************************************************************************
                          ofxiimporterplugin.h
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

#include <qstringlist.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// Library Includes

#include <libofx/libofx.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
class KOnlineBankingStatus;

/**
@author Ace Jones
*/
class OfxImporterPlugin : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::ImporterPlugin, public KMyMoneyPlugin::OnlinePlugin
{
Q_OBJECT
public:
    OfxImporterPlugin(QObject *parent = 0, const char *name = 0, const QStringList& = QStringList());

    ~OfxImporterPlugin();

  /**
    * This method returns the english-language name of the format
    * this plugin imports, e.g. "OFX"
    *
    * @return QString Name of the format
    */
  virtual QString formatName(void) const;

  /**
    * This method returns the filename filter suitable for passing to
    * KFileDialog::setFilter(), e.g. "*.ofx *.qfx" which describes how
    * files of this format are likely to be named in the file system
    *
    * @return QString Filename filter string
    */
  virtual QString formatFilenameFilter(void) const;

  /**
    * This method returns whether this plugin is able to import
    * a particular file.
    *
    * @param filename Fully-qualified pathname to a file
    *
    * @return bool Whether the indicated file is importable by this plugin
    */
  virtual bool isMyFormat( const QString& filename ) const;

  /**
    * Import a file
    *
    * @param filename File to import
    *
    * @return bool Whether the import was successful.
    */
  virtual bool import( const QString& filename );

  /**
    * Returns the error result of the last import
    *
    * @return QString English-language name of the error encountered in the
    *  last import, or QString() if it was successful.
    *
    */
  virtual QString lastError(void) const;

  QWidget* accountConfigTab(const MyMoneyAccount& acc, QString& name);

  MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current);

  const MyMoneyAccount& account(const QString& key, const QString& value) const;

  void protocols(QStringList& protocolList) const;

  bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& settings);
  bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts);

protected slots:
  void slotImportFile(void);
  void slotImportFile(const QString& url);

protected:
    void createActions(void);
    void addnew(void) { m_statementlist.push_back(MyMoneyStatement()); }
    MyMoneyStatement& back(void) { return m_statementlist.back(); }
    bool isValid(void) const { return m_valid; }
    void setValid(void) { m_valid = true; }
    void addInfo(const QString& _msg ) { m_infos+=_msg; }
    void addWarning(const QString& _msg )  { m_warnings+=_msg; }
    void addError(const QString& _msg )  { m_errors+=_msg; }
    const QStringList& infos(void) const { return m_infos; }
    const QStringList& warnings(void) const { return m_warnings; }
    const QStringList& errors(void) const { return m_errors; }
    bool storeStatements(Q3ValueList<MyMoneyStatement>& statements);
    bool importStatement(const MyMoneyStatement& s);


    static int ofxTransactionCallback( struct OfxTransactionData, void* );
    static int ofxStatementCallback( struct OfxStatementData, void* );
    static int ofxAccountCallback( struct OfxAccountData, void* );
    static int ofxStatusCallback( struct OfxStatusData, void* );
    static int ofxSecurityCallback( struct OfxSecurityData, void* );

private:
  bool m_valid;
  bool m_preferName;
  Q3ValueList<MyMoneyStatement> m_statementlist;
  Q3ValueList<MyMoneyStatement::Security> m_securitylist;
  QString m_fatalerror;
  QStringList m_infos;
  QStringList m_warnings;
  QStringList m_errors;
  KOnlineBankingStatus* m_statusDlg;
};

#endif
