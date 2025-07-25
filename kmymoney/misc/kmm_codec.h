/*
    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMM_CODEC_H
#define KMM_CODEC_H

#include "kmm_codec_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTextStream>

class QComboBox;

class KMM_Codec
{
public:
    KMM_Codec() = default;

    static KMM_CODEC_EXPORT QByteArray encodingForLocale();
    static KMM_CODEC_EXPORT void loadComboBox(QComboBox* cb);
    static KMM_CODEC_EXPORT QStringList availableCodecs();
    static KMM_CODEC_EXPORT QList<int> availableMibs();
    static KMM_CODEC_EXPORT QString codecNameForMib(int mib);
    static KMM_CODEC_EXPORT int mibForCodecName(const QString& name);
};

#endif // KMM_CODEC_H
