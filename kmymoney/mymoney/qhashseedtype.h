/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QHASHSEEDTYPE_H
#define QHASHSEEDTYPE_H

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
typedef uint qHashSeedType;
#else
typedef size_t qHashSeedType;
#endif

#endif // QHASHSEEDTYPE_H
