/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SCHEDULESTOICALENDAR_H
#define SCHEDULESTOICALENDAR_H

class QString;

class KMMSchedulesToiCalendar
{
public:
    KMMSchedulesToiCalendar();
    ~KMMSchedulesToiCalendar();
    void exportToFile(const QString& file);

private:
    struct Private;
    Private *d;
};

#endif // SCHEDULESTOICALENDAR_H
