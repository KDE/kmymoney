/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PASSSTORE_H
#define PASSSTORE_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QLineEdit;
class QAction;

class PassStorePrivate;
/**
 * This class implements a mechanism to enrich a QLineEdit with an action
 * icon to load its content by extracting from a password store maintained
 * by pass (see https://www.passwordstore.org). The path to the password
 * file is identified by the @c applicationPrefix and @c id provided
 * at construction time or the @c id passed with setPasswordId().
 *
 * If GPG is not available or a password file identified by
 * @c applicationPrefix and @c id is not available or not
 * readable, the icon to load the password is not visible
 * inside the QLineEdit.
 *
 * The file for the password is expected to be in
 * ~/.password-store/<applicationPrefix>/<id>.gpg
 *
 * To use this class, the following is needed (example provided for
 * KMyMoney and KBanking plugin):
 *
 * @code
 *
 *    extern QString accountRef;
 *    auto lineedit = new QLineEdit;
 *    new PassStore(lineedit, QLatin1String("KMyMoney/KBanking"), accountRef);
 *
 * @endcode
 *
 * @note relies on kmm_gpgfile
 *
 * @author Thomas Baumgart
 */
class KMM_BASE_WIDGETS_EXPORT PassStore : public QObject
{
    Q_DECLARE_PRIVATE(PassStore);
    Q_OBJECT
public:
    explicit PassStore(QLineEdit* parent, const QString& applicationPrefix, const QString& id = QString());
    ~PassStore();

    /**
     * Update the password id to @a id. This will check for
     * the password in the store and update the icon accordingly.
     *
     * @note The characters '/' and '\\' will be converted to '_'
     */
    void setPasswordId(const QString& id);

    /**
     * Return the 'converted' passwordId.
     *
     * @sa setPasswordId() for possible conversions.
     */
    QString passwordId() const;

    /**
     * Return if the action is visible or not. This can be used
     * by code to check if a stored password is available or not.
     */
    bool isActionVisible() const;

private:
    PassStorePrivate* d_ptr;
};

#endif // PASSSTORE_H
