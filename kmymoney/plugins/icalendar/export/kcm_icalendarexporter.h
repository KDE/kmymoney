/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KCM_ICALENDAREXPORTER_H
#define KCM_ICALENDAREXPORTER_H

#include "kmm_kcmodule.h"

#include "qcontainerfwd.h"

class KCMiCalendarExporter : public KMMKCModule
{
public:
    explicit KCMiCalendarExporter(QObject* parent = nullptr, const QVariantList& args = QVariantList());
    ~KCMiCalendarExporter();
};

#endif // KCM_ICALENDAREXPORT_H

