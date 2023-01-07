/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "specialdatesmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"

struct SpecialDatesModel::Private
{
    Private(SpecialDatesModel* qq)
        : q(qq)
        , showDateHeaders(false)
    {
    }

    SpecialDatesModel*  q;
    bool                showDateHeaders;
    QDate               firstFiscalDate;
};

SpecialDatesModel::SpecialDatesModel(QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<SpecialDateEntry>(parent, QStringLiteral("SD"), SpecialDatesModel::ID_SIZE, undoStack)
    , d(new Private(this))
{
    setObjectName(QLatin1String("SpecialDatesModel"));
}

SpecialDatesModel::~SpecialDatesModel()
{
}

int SpecialDatesModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return JournalModel::Column::MaxColumns;
}

Qt::ItemFlags SpecialDatesModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::NoItemFlags;
}

void SpecialDatesModel::load()
{
    beginResetModel();
    // first get rid of any existing entries
    clearModelItems();

    QMap<QDate, QString> entries;
    if (d->showDateHeaders || d->firstFiscalDate.isValid()) {
        if (d->showDateHeaders) {
            const QDate today = QDate::currentDate();
            const QDate thisMonth(today.year(), today.month(), 1);
            const QDate lastMonth = thisMonth.addMonths(-1);
            const QDate yesterday = today.addDays(-1);
            // a = QDate::dayOfWeek()         todays weekday (1 = Monday, 7 = Sunday)
            // b = QLocale().firstDayOfWeek() first day of week (1 = Monday, 7 = Sunday)
            int weekStartOfs = today.dayOfWeek() - QLocale().firstDayOfWeek();
            if (weekStartOfs < 0) {
                weekStartOfs = 7 + weekStartOfs;
            }
            const QDate thisWeek = today.addDays(-weekStartOfs);
            const QDate lastWeek = thisWeek.addDays(-7);
            const QDate thisYear(today.year(), 1, 1);

            entries[lastMonth] = i18n("Last month");
            entries[thisMonth] = i18n("This month");
            entries[lastWeek] = i18n("Last week");
            entries[thisWeek] = i18n("This week");
            entries[yesterday] = i18n("Yesterday");
            entries[today] = i18n("Today");
            entries[thisYear] = i18n("This year");
            entries[today.addDays(1)] = i18n("Future transactions");
            entries[thisWeek.addDays(7)] = i18n("Next week");
            entries[thisMonth.addMonths(1)] = i18n("Next month");
        }
        if (d->firstFiscalDate.isValid()) {
            entries[d->firstFiscalDate] = i18n("Current fiscal year");
            entries[d->firstFiscalDate.addYears(-1)] = i18n("Previous fiscal year");
            entries[d->firstFiscalDate.addYears(1)] = i18n("Next fiscal year");
        }
        insertRows(0, entries.count());

        m_nextId = 0;

        int row = 0;
        QMap<QDate, QString>::const_iterator it;
        for (it = entries.constBegin(); it != entries.constEnd(); ++it) {
            SpecialDateEntry entry(nextId(), it.key(), *it);
            static_cast<TreeItem<SpecialDateEntry>*>(index(row, 0).internalPointer())->dataRef() = entry;
            ++row;
        }
    }

    endResetModel();
    setDirty(false);
}

QVariant SpecialDatesModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    // we report to have the same number of columns as the
    // journal model but we only react on the first column
    if (idx.column() < 0 || idx.column() >= JournalModel::Column::MaxColumns)
        return {};

    const SpecialDateEntry& dateEntry = static_cast<TreeItem<SpecialDateEntry>*>(idx.internalPointer())->constDataRef();

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return dateEntry.txt();

    case eMyMoney::Model::IdRole:
        return dateEntry.id();

    case eMyMoney::Model::TransactionEntryDateRole:
    case eMyMoney::Model::TransactionPostDateRole:
        return dateEntry.date();

    default:
        break;
    }
    return {};
}

void SpecialDatesModel::setOptions(bool showDateHeaders, const QDate& firstFiscalDate)
{
    if ((d->showDateHeaders != showDateHeaders) || (d->firstFiscalDate != firstFiscalDate)) {
        d->showDateHeaders = showDateHeaders;
        d->firstFiscalDate = firstFiscalDate;
        load();
    }
}
