/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMCONCATENATETABLEMODEL_H
#define KMMCONCATENATETABLEMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QConcatenateTablesProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class LedgerConcatenateModelPrivate;
class KMM_MODELS_EXPORT LedgerConcatenateModel : public QConcatenateTablesProxyModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LedgerConcatenateModel)
    Q_DISABLE_COPY(LedgerConcatenateModel)

public:
    explicit LedgerConcatenateModel(QObject* parent);
    ~LedgerConcatenateModel();

protected:
private:
    LedgerConcatenateModelPrivate* d_ptr;
};

#endif // KMMCONCATENATETABLEMODEL_H
