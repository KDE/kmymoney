/***************************************************************************
                          kmmstatementinterface.h
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMMSTATEMENTINTERFACE_H
#define KMMSTATEMENTINTERFACE_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;
class MyMoneyKeyValueContainer;

#include "statementinterface.h"

namespace KMyMoneyPlugin
{

/**
  * This class represents the implementation of the
  * StatementInterface.
  */
class KMMStatementInterface : public StatementInterface
{
  Q_OBJECT

public:
  explicit KMMStatementInterface(QObject* parent, const char* name = 0);
  ~KMMStatementInterface() {}

  /**
    * This method imports a MyMoneyStatement into the engine
    */
  QStringList import(const MyMoneyStatement& s, bool silent = false) final override;

  /**
   * This method returns the account for a given @a key - @a value pair.
   * If the account is not found in the list of accounts, MyMoneyAccount()
   * is returned. The @a key - @a value pair can be in the account's kvp
   * container or the account's online settings kvp container.
   */
  MyMoneyAccount account(const QString& key, const QString& value) const final override;

  /**
   * This method stores the online parameters in @a kvps used by the plugin
   * with the account @a acc.
   */
  void setAccountOnlineParameters(const MyMoneyAccount&acc, const MyMoneyKeyValueContainer& kvps) const final override;
};

} // namespace
#endif
