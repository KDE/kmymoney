/*
    SPDX-FileCopyrightText: 2002 Ace Jones <acejones@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYOFXCONNECTOR_H
#define MYMONEYOFXCONNECTOR_H

// ----------------------------------------------------------------------------
// Library Includes

#include <libofx/libofx.h>

// if OFX has a major version number defined, we'll take it
// if not, we assume 0.9.4. since that's the minimum version
#ifdef LIBOFX_MAJOR_VERSION
#define LIBOFX_VERSION KDE_MAKE_VERSION(LIBOFX_MAJOR_VERSION, LIBOFX_MINOR_VERSION, LIBOFX_MICRO_VERSION)
#else
#define LIBOFX_VERSION KDE_MAKE_VERSION(0,9,4)
#endif
#define LIBOFX_IS_VERSION(a,b,c) (LIBOFX_VERSION >= KDE_MAKE_VERSION(a,b,c))

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

class QDate;

// ----------------------------------------------------------------------------
// KDE Includes
class KComboBox;
class KLineEdit;

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneykeyvaluecontainer.h"

class MyMoneyAccount;

#define OFX_PASSWORD_KEY(url, id) QString("KMyMoney-OFX-%1-%2").arg(url, id)

/**
 * @author Thomas Baumgart
 */
class OfxAppVersion
{
public:
    OfxAppVersion(KComboBox* combo, KLineEdit* versionEdit, const QString& appId);
    /**
     * This method returns the currently selected application id
     * as a colon separated value consisting of the application
     * and version (eg. "QWIN:1700").  If current value is the
     * default, an empty string is returned.
     */
    const QString appId() const;

    /**
     * This method returns @c true in case the current selected application
     * version is valid (contains app and version) or @c false otherwise.
     */
    bool isValid() const;

private:
    QMap<QString, QString> m_appMap;
    KComboBox*             m_combo;
    KLineEdit*             m_versionEdit;
};

/**
 * @author Thomas Baumgart
 */
class OfxHeaderVersion
{
public:
    OfxHeaderVersion(KComboBox* combo, const QString& headerVersion);
    QString headerVersion() const;

private:
    KComboBox*             m_combo;
};

/**
@author ace jones
*/
class MyMoneyOfxConnector
{
public:
    explicit MyMoneyOfxConnector(const MyMoneyAccount& _account);
    QString url() const;

    /**
     * Constructs the request for a statement. The first date
     * for which transactions will be requested is determined
     * by statementStartDate()
     */
    QString statementRequest() const;

    const QByteArray statementResponse(const QDate& _dtstart) const;

    // returns the user agent string to be used or empty to use system default
    QString userAgent() const;

    /**
     * This method adjusts a request to specific requirements.
     */
    static void institutionSpecificRequestAdjustment(QString& request);

private:
    void initRequest(OfxFiLogin* fi) const;
    QDate statementStartDate() const;
    QString iban() const;
    QString fiorg() const;
    QString fiid() const;
    QString clientUid() const;
    QString username() const;
    QString password() const;
    QString accountnum() const;
    OfxAccountData::AccountType accounttype() const;

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
