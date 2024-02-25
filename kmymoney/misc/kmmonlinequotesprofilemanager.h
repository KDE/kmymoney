/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMONLINEQUOTESPROFILEMANAGER_H
#define KMMONLINEQUOTESPROFILEMANAGER_H

#include <kmm_webconnect_export.h>

#include <alkimia/alkonlinequotesprofilemanager.h>

class KMM_WEBCONNECT_EXPORT KMMOnlineQuotesProfileManager : public AlkOnlineQuotesProfileManager
{
private:
    KMMOnlineQuotesProfileManager();

public:
    static KMMOnlineQuotesProfileManager& instance();

    /**
     * Returns a description than can be used as tooltip
     * in case the profile named @a profileNmae is not
     * available for usage. In case it is available,
     * the returned QString is empty.
     *
     * @sa AlkOnlineQuotesProfileManager::profile()
     */
    QString availabilityHint(const QString& profileName);
};

#endif // KMMONLINEQUOTESPROFILEMANAGER_H
