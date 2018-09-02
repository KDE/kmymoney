/*
 * Copyright 2005-2018  Thomas Baumgart <tbaumgart@kde.org>
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

  virtual void resetMessages() const = 0;
  virtual void showMessages(int statementCount) const = 0;

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
