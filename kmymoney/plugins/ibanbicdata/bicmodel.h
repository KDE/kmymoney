/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BICMODEL_H
#define BICMODEL_H

#include <QSqlQueryModel>

#include "iban_bic_identifier_export.h"

class IBAN_BIC_IDENTIFIER_EXPORT bicModel : public QSqlQueryModel
{
    Q_OBJECT

public:
    enum DisplayRole {
        InstitutionNameRole = Qt::UserRole,
    };

    explicit bicModel(QObject* parent = 0);
    QVariant data(const QModelIndex& item, int role = Qt::DisplayRole) const final override;

};

#endif // BICMODEL_H
