/***************************************************************************
                         mymoneyofxconnector.cpp
                             -------------------
    begin                : Sat Nov 13 2004
    copyright            : (C) 2002 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYOFXCONNECTOR_H
#define MYMONEYOFXCONNECTOR_H

#ifdef HAVE_CONFIG_H
#include <config-kmymoney.h>
#endif


// ----------------------------------------------------------------------------
// Library Includes

#include <libofx/libofx.h>

// if OFX has a major version number defined, we'll take it
// if not, we assume 0.8.3. 0.8.3 was the last version w/o version number info
#ifdef LIBOFX_MAJOR_VERSION
#define LIBOFX_VERSION KDE_MAKE_VERSION(LIBOFX_MAJOR_VERSION, LIBOFX_MINOR_VERSION, LIBOFX_MICRO_VERSION)
#else
#define LIBOFX_VERSION KDE_MAKE_VERSION(0,8,3)
#endif
#define LIBOFX_IS_VERSION(a,b,c) (LIBOFX_VERSION >= KDE_MAKE_VERSION(a,b,c))

// ----------------------------------------------------------------------------
// QT Includes

class QDate;

// ----------------------------------------------------------------------------
// KDE Includes
class KComboBox;

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneykeyvaluecontainer.h>

class MyMoneyAccount;

#define OFX_PASSWORD_KEY(url, id) QString("KMyMoney-OFX-%1-%2").arg(url, id)

/**
 * @author Thomas Baumgart
 */
class OfxAppVersion
{
public:
  OfxAppVersion(KComboBox* combo, const QString& appId);
  /**
   * This method returns the currently selected application id
   * as a colon separated value consisting of the application
   * and version (eg. "QWIN:1700").  If current value is the
   * default, an empty string is returned.
   */
  const QString appId(void) const;

private:
  QMap<QString, QString> m_appMap;
  KComboBox*             m_combo;
};

/**
 * @author Thomas Baumgart
 */
class OfxHeaderVersion
{
public:
  OfxHeaderVersion(KComboBox* combo, const QString& headerVersion);
  QString headerVersion(void) const;

private:
  KComboBox*             m_combo;
};

/**
@author ace jones
*/
class MyMoneyOfxConnector
{
public:
  MyMoneyOfxConnector(const MyMoneyAccount& _account);
  QString url(void) const;

  /**
   * Constructs the request for a statement. The first date
   * for which transactions will be requested is determined
   * by statementStartDate()
   */
  const QByteArray statementRequest(void) const;
  const QByteArray statementResponse(const QDate& _dtstart) const;

private:
  void initRequest(OfxFiLogin* fi) const;
  QDate statementStartDate(void) const;
  QString iban(void) const;
  QString fiorg(void) const;
  QString fiid(void) const;
  QString username(void) const;
  QString password(void) const;
  QString accountnum(void) const;
  OfxAccountData::AccountType accounttype(void) const;

private:
  const MyMoneyAccount& m_account;
  MyMoneyKeyValueContainer m_fiSettings;
};

// open a synchronous wallet in a safe way (the function is here because the wallet is only used in the OFX plugin)
namespace KWallet
{
class Wallet;
}
KWallet::Wallet *openSynchronousWallet();

#endif // OFXCONNECTOR_H
