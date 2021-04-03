/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEMPLATEWRITER_H
#define TEMPLATEWRITER_H

#include "kmm_templates_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyTemplate;

/**
 * @author Thomas Baumgart
 */

class TemplateWriterPrivate;
class KMM_TEMPLATES_EXPORT TemplateWriter : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TemplateWriter)
    Q_DECLARE_PRIVATE(TemplateWriter)

public:
    explicit TemplateWriter(QWidget* parent = nullptr);
    ~TemplateWriter();

    bool exportTemplate(const MyMoneyTemplate& tmpl, const QUrl &url);
    QString errorMessage() const;

private:
    TemplateWriterPrivate * const d_ptr;
};

#endif
