/*
    SPDX-FileCopyrightText: 2016-2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCM_CSVIMPORTER_H
#define KCM_CSVIMPORTER_H

// ----------------------------------------------------------------------------
// QT Includes

class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

#include <KCModule>

// ----------------------------------------------------------------------------
// Project Includes

class KCMCSVImporterPrivate;
class KCMCSVImporter : public KCModule
{
public:
    explicit KCMCSVImporter(QWidget* parent, const QVariantList& args);
    ~KCMCSVImporter();

private:
    Q_DECLARE_PRIVATE(KCMCSVImporter)
    KCMCSVImporterPrivate* d_ptr;
};

#endif // KCM_CSVIMPORT_H

