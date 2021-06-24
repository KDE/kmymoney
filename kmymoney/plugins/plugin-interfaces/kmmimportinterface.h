/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMIMPORTINTERFACE_H
#define KMMIMPORTINTERFACE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrl>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "importinterface.h"

namespace KMyMoneyPlugin {

/**
 * This class represents the implementation of the
 * ImportInterface.
 */
class KMMImportInterface : public ImportInterface
{
    Q_OBJECT

public:
    explicit KMMImportInterface(QObject* parent, const char* name = 0);
    ~KMMImportInterface() override
    {
    }

    QUrl selectFile(const QString& title, const QString& path, const QString& mask, QFileDialog::FileMode mode, QWidget* widget) const final override;
};

} // namespace
#endif
