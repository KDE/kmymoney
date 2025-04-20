/*
 *    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef IMPORTSUMMARYDIALOG_H
#define IMPORTSUMMARYDIALOG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

class StatementModel;

class ImportSummaryDialogPrivate;
class KMM_BASE_DIALOGS_EXPORT ImportSummaryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImportSummaryDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~ImportSummaryDialog();

    void setModel(StatementModel* model) const;
    void setLabel(const QString& text) const;

public Q_SLOTS:
    int exec() final override;

private:
    ImportSummaryDialogPrivate* const d;
};

#endif // IMPORTSUMMARYDIALOG_H
