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

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_kcmodule.h"

class KCMCSVImporterPrivate;
class KCMCSVImporter : public KMMKCModule
{
public:
    explicit KCMCSVImporter(QObject* parent, const QVariantList& args = QVariantList());
    ~KCMCSVImporter();

private:
    Q_DECLARE_PRIVATE(KCMCSVImporter)
    KCMCSVImporterPrivate* d_ptr;
};

#endif // KCM_CSVIMPORT_H

