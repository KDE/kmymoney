/*
 * SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
