/***************************************************************************
                          statementinterface.h
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
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

#ifndef STATEMENTINTERFACE_H
#define STATEMENTINTERFACE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmm_plugin_export.h>

class MyMoneyAccount;
class MyMoneyStatement;
class MyMoneyKeyValueContainer;

namespace KMyMoneyPlugin
{

/**
  * This abstract class represents the interface to import statements
  * into the KMyMoney application
  */
class KMM_PLUGIN_EXPORT StatementInterface : public QObject
{
  Q_OBJECT

public:
  explicit StatementInterface(QObject* parent, const char* name = 0);
  virtual ~StatementInterface();

  /**
    * This method imports a MyMoneyStatement into the engine
    */
  virtual QStringList import(const MyMoneyStatement& s, bool silent = false) = 0;

  /**
   * This method returns the account for a given @a key - @a value pair.
   * If the account is not found in the list of accounts, MyMoneyAccount()
   * is returned.
   */
  virtual MyMoneyAccount account(const QString& key, const QString& value) const = 0;

  /**
   */
  virtual void setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const = 0;

};

} // namespace
#endif
