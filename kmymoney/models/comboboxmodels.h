/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMBOBOXMODELS_H
#define COMBOBOXMODELS_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringListModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class OccurrencesModelPrivate;
class KMM_MODELS_EXPORT OccurrencesModel : public QStringListModel
{
    Q_OBJECT
public:
    OccurrencesModel(QObject* parent = nullptr);
    ~OccurrencesModel();

    QVariant data(const QModelIndex& idx, int role) const override;

    QModelIndex indexByOccurrence(eMyMoney::Schedule::Occurrence occurrence) const;

private:
    OccurrencesModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(OccurrencesModel);
};

class PaymentMethodModelPrivate;
class KMM_MODELS_EXPORT PaymentMethodModel : public QStringListModel
{
    Q_OBJECT
public:
    PaymentMethodModel(QObject* parent = nullptr);
    ~PaymentMethodModel();

    QVariant data(const QModelIndex& idx, int role) const override;

    QModelIndex indexByPaymentMethod(eMyMoney::Schedule::PaymentType paymentType) const;

private:
    PaymentMethodModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(PaymentMethodModel);
};

#endif // COMBOBOXMODELS_H
