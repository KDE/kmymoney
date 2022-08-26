/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "comboboxmodels.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyschedule.h"

using namespace eMyMoney::Schedule;
using namespace eMyMoney::Model;

class OccurrencesModelPrivate
{
    Q_DECLARE_PUBLIC(OccurrencesModel);
    OccurrencesModel* q_ptr;

public:
    OccurrencesModelPrivate(OccurrencesModel* qq)
        : q_ptr(qq)
    {
        Q_Q(OccurrencesModel);
        const QVector<Occurrence> occurrences{Occurrence::Once,
                                              Occurrence::Daily,
                                              Occurrence::Weekly,
                                              Occurrence::EveryOtherWeek,
                                              Occurrence::EveryHalfMonth,
                                              Occurrence::EveryThreeWeeks,
                                              Occurrence::EveryThirtyDays,
                                              Occurrence::EveryFourWeeks,
                                              Occurrence::Monthly,
                                              Occurrence::EveryEightWeeks,
                                              Occurrence::EveryOtherMonth,
                                              Occurrence::EveryThreeMonths,
                                              Occurrence::EveryFourMonths,
                                              Occurrence::TwiceYearly,
                                              Occurrence::Yearly,
                                              Occurrence::EveryOtherYear};
        q->insertRows(0, occurrences.count());
        int row = 0;
        for (const auto& occurrence : occurrences) {
            const auto text = MyMoneySchedule::occurrenceToString(occurrence);
            rowToScheduleMap.insert(row, occurrence);
            scheduleToRowMap.insert(occurrence, row);
            scheduleToStringMap.insert(occurrence, text);
            stringToScheduleMap.insert(text, occurrence);
            q->setData(q->index(row++, 0), text, Qt::DisplayRole);
        }
    }

    ~OccurrencesModelPrivate()
    {
    }

    QHash<Occurrence, QString> scheduleToStringMap;
    QHash<QString, Occurrence> stringToScheduleMap;
    QHash<Occurrence, int> scheduleToRowMap;
    QHash<int, Occurrence> rowToScheduleMap;
};

OccurrencesModel::OccurrencesModel(QObject* parent)
    : QStringListModel(parent)
    , d_ptr(new OccurrencesModelPrivate(this))
{
}

OccurrencesModel::~OccurrencesModel()
{
    Q_D(OccurrencesModel);
    delete d;
}

QVariant OccurrencesModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};
    if (idx.row() < 0 || idx.row() > (rowCount() - 1))
        return {};

    Q_D(const OccurrencesModel);
    switch (role) {
    case ScheduleFrequencyRole:
        return QVariant::fromValue(d->rowToScheduleMap.value(idx.row()));
        break;
    }
    return QStringListModel::data(idx, role);
}

QModelIndex OccurrencesModel::indexByOccurrence(eMyMoney::Schedule::Occurrence occurrence) const
{
    Q_D(const OccurrencesModel);
    if (d->scheduleToRowMap.contains(occurrence)) {
        return index(d->scheduleToRowMap.value(occurrence), 0);
    }
    return {};
}

class PaymentMethodModelPrivate
{
    Q_DECLARE_PUBLIC(PaymentMethodModel);
    PaymentMethodModel* q_ptr;

public:
    PaymentMethodModelPrivate(PaymentMethodModel* qq)
        : q_ptr(qq)
    {
        Q_Q(PaymentMethodModel);
        const QVector<PaymentType> paymentMethods{PaymentType::DirectDeposit,
                                                  PaymentType::ManualDeposit,
                                                  PaymentType::DirectDebit,
                                                  PaymentType::StandingOrder,
                                                  PaymentType::BankTransfer,
                                                  PaymentType::WriteChecque,
                                                  PaymentType::Other};

        q->insertRows(0, paymentMethods.count());
        int row = 0;
        for (const auto& paymentMethod : paymentMethods) {
            const auto text = i18n(MyMoneySchedule::paymentMethodToString(paymentMethod));
            paymentMethodToRowMap.insert(paymentMethod, row);
            rowToPaymentMethodMap.insert(row, paymentMethod);
            q->setData(q->index(row++, 0), text, Qt::DisplayRole);
        }
    }

    ~PaymentMethodModelPrivate()
    {
    }

    QHash<PaymentType, int> paymentMethodToRowMap;
    QHash<int, PaymentType> rowToPaymentMethodMap;
};

PaymentMethodModel::PaymentMethodModel(QObject* parent)
    : QStringListModel(parent)
    , d_ptr(new PaymentMethodModelPrivate(this))
{
}

PaymentMethodModel::~PaymentMethodModel()
{
    Q_D(PaymentMethodModel);
    delete d;
}

QVariant PaymentMethodModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};
    if (idx.row() < 0 || idx.row() > (rowCount() - 1))
        return {};

    Q_D(const PaymentMethodModel);
    switch (role) {
    case SchedulePaymentTypeRole:
        return QVariant::fromValue(d->rowToPaymentMethodMap.value(idx.row()));
        break;
    }
    return QStringListModel::data(idx, role);
}

QModelIndex PaymentMethodModel::indexByPaymentMethod(eMyMoney::Schedule::PaymentType type) const
{
    Q_D(const PaymentMethodModel);
    if (d->paymentMethodToRowMap.contains(type)) {
        return index(d->paymentMethodToRowMap.value(type), 0);
    }
    return {};
}
