/***************************************************************************
                          kmmstatementinterface.cpp
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

#include "kmmstatementinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney.h"
#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"

KMyMoneyPlugin::KMMStatementInterface::KMMStatementInterface(KMyMoneyApp* app, QObject* parent, const char* name) :
    StatementInterface(parent, name),
    m_app(app)
{
}

bool KMyMoneyPlugin::KMMStatementInterface::import(const MyMoneyStatement& s, bool silent)
{
  qDebug("KMyMoneyPlugin::KMMStatementInterface::import start");
  return m_app->slotStatementImport(s, silent);
}

const MyMoneyAccount& KMyMoneyPlugin::KMMStatementInterface::account(const QString& key, const QString& value) const
{
  return m_app->account(key, value);
}

void KMyMoneyPlugin::KMMStatementInterface::setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const
{
  m_app->setAccountOnlineParameters(acc, kvps);
}
