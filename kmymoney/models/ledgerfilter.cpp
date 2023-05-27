/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgerfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QComboBox>
#include <QLineEdit>
#include <QTimer>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "journalmodel.h"
#include "ledgersortproxymodel_p.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneymodelbase.h"
#include "mymoneymoney.h"
#include "mymoneytag.h"
#include "reconciliationmodel.h"
#include "schedulesjournalmodel.h"

using namespace Icons;

class LedgerFilterPrivate : public LedgerSortProxyModelPrivate
{
public:
    LedgerFilterPrivate(LedgerFilter* qq)
        : LedgerSortProxyModelPrivate(qq)
        , lineEdit(nullptr)
        , comboBox(nullptr)
        , state(LedgerFilter::State::Any)
    {
        delayTimer.setSingleShot(true);
    }

    QLineEdit* lineEdit;
    QComboBox* comboBox;
    LedgerFilter::State state;
    QString filterString;
    QTimer delayTimer;
    QDate endDate;
};

LedgerFilter::LedgerFilter(QObject* parent)
    : LedgerSortProxyModel(new LedgerFilterPrivate(this), parent)
{
    Q_D(LedgerFilter);
    setObjectName("LedgerFilter");
    connect(&d->delayTimer, &QTimer::timeout, this, [&]() {
        invalidateFilter();
    });
}

void LedgerFilter::setComboBox(QComboBox* filterBox)
{
    Q_D(LedgerFilter);
    filterBox->clear();
    filterBox->insertItem(static_cast<int>(State::Any), Icons::get(Icon::TransactionStateAny), i18n("Any status"));
    filterBox->insertItem(static_cast<int>(State::Imported), Icons::get(Icon::TransactionStateImported), i18n("Imported"));
    filterBox->insertItem(static_cast<int>(State::Matched), Icons::get(Icon::TransactionStateMatched), i18n("Matched"));
    filterBox->insertItem(static_cast<int>(State::Erroneous), Icons::get(Icon::TransactionStateErroneous), i18n("Erroneous"));
    filterBox->insertItem(static_cast<int>(State::Scheduled), Icons::get(Icon::TransactionStateScheduled), i18n("Scheduled"));
    filterBox->insertItem(static_cast<int>(State::NotMarked), Icons::get(Icon::TransactionStateNotMarked), i18n("Not marked"));
    filterBox->insertItem(static_cast<int>(State::NotReconciled), Icons::get(Icon::TransactionStateNotReconciled), i18n("Not reconciled"));
    filterBox->insertItem(static_cast<int>(State::Cleared), Icons::get(Icon::TransactionStateCleared), i18nc("Reconciliation state 'Cleared'", "Cleared"));
    filterBox->setCurrentIndex(static_cast<int>(d->state));

    // connect(d->ui->m_filterBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &RegisterSearchLine::slotStatusChanged);
    connect(filterBox, QOverload<int>::of(&QComboBox::activated), this, [&](int idx) {
        setStateFilter(static_cast<LedgerFilter::State>(idx));
    });
    connect(filterBox, &QComboBox::destroyed, this, [&]() {
        Q_D(LedgerFilter);
        d->comboBox = nullptr;
    });

    d->comboBox = filterBox;
}

void LedgerFilter::setLineEdit(QLineEdit* lineEdit)
{
    Q_D(LedgerFilter);
    Q_ASSERT(lineEdit != nullptr);
    lineEdit->setClearButtonEnabled(true);
    connect(lineEdit, &QLineEdit::textChanged, this, [&](const QString& text) {
        Q_D(LedgerFilter);
        d->filterString = text;
        d->delayTimer.start(200);
    });
    connect(lineEdit, &QLineEdit::destroyed, this, [&]() {
        Q_D(LedgerFilter);
        d->lineEdit = nullptr;
    });
    d->lineEdit = lineEdit;
}

bool LedgerFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const LedgerFilter);

    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    const bool isJournalItem = d->isJournalModel(idx);
    const bool isScheduleItem = d->isSchedulesJournalModel(idx);

    // a transaction is in date range if no date range is set or the post date is older than the end of the range
    const auto inDateRange = !d->endDate.isValid() || (idx.data(eMyMoney::Model::TransactionPostDateRole).toDate() <= d->endDate);

    if (d->state != State::Any) {
        if (isJournalItem) {
            const auto splitState = idx.data(eMyMoney::Model::SplitReconcileFlagRole).value<eMyMoney::Split::State>();
            switch (d->state) {
            case State::NotMarked:
                if ((splitState != eMyMoney::Split::State::NotReconciled) && inDateRange) {
                    return false;
                }
                break;
            case State::Cleared:
                if ((splitState != eMyMoney::Split::State::Cleared) && inDateRange) {
                    return false;
                }
                break;
            case State::NotReconciled:
                if (((splitState == eMyMoney::Split::State::Reconciled) || (splitState == eMyMoney::Split::State::Frozen)) && inDateRange) {
                    return false;
                }
                break;
            case State::Erroneous:
                if (!idx.data(eMyMoney::Model::TransactionErroneousRole).toBool() && inDateRange) {
                    return false;
                }
                break;
            case State::Imported:
                if (!idx.data(eMyMoney::Model::TransactionIsImportedRole).toBool() && inDateRange) {
                    return false;
                }
                break;
            case State::Matched:
                if (!idx.data(eMyMoney::Model::JournalSplitIsMatchedRole).toBool() && inDateRange) {
                    return false;
                }
                break;

            case State::Scheduled:
                return false;

            default:
                break;
            }
        }

        if (isScheduleItem) {
            if (d->state != State::Scheduled) {
                return false;
            }
        }
    }

    if (isJournalItem || isScheduleItem) {
        if (!d->filterString.isEmpty()) {
            const auto file = MyMoneyFile::instance();
            auto rc = idx.data(eMyMoney::Model::SplitMemoRole).toString().contains(d->filterString, Qt::CaseInsensitive);
            if (!rc)
                rc = idx.data(eMyMoney::Model::SplitNumberRole).toString().contains(d->filterString, Qt::CaseInsensitive);
            if (!rc)
                rc = idx.data(eMyMoney::Model::SplitPayeeRole).toString().contains(d->filterString, Qt::CaseInsensitive);
            if (!rc) {
                const auto tagIdList = idx.data(eMyMoney::Model::SplitTagIdRole).toStringList();
                for (const auto& tagId : tagIdList) {
                    const auto tagName = file->tag(tagId).name();
                    rc = tagName.contains(d->filterString, Qt::CaseInsensitive);
                    if (rc)
                        break;
                }
            }
            if (!rc) {
                const auto accId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
                if (!accId.isEmpty()) {
                    const auto acc = file->account(accId);
                    if (d->filterString.contains(MyMoneyFile::AccountSeparator)) {
                        QStringList names;
                        MyMoneyAccount current = acc;
                        QString accountId;
                        do {
                            names.prepend(current.name());
                            accountId = current.parentAccountId();
                            current = file->account(accountId);
                        } while (current.accountType() != eMyMoney::Account::Type::Unknown && !MyMoneyFile::instance()->isStandardAccount(accountId));
                        if (names.size() > 1 && names.join(MyMoneyFile::AccountSeparator).contains(d->filterString, Qt::CaseInsensitive))
                            rc = true;
                    }

                    if (!rc) {
                        rc = acc.name().contains(d->filterString, Qt::CaseInsensitive);
                    }
                }
            }

            if (!rc) {
                QString s(d->filterString);
                s.replace(MyMoneyMoney::thousandSeparator(), QChar());
                if (!s.isEmpty()) {
                    rc = idx.data(eMyMoney::Model::SplitFormattedValueRole).toString().contains(s, Qt::CaseInsensitive);
                    if (!rc)
                        rc = idx.data(eMyMoney::Model::SplitFormattedSharesRole).toString().contains(s, Qt::CaseInsensitive);
                }
            }
            if (!rc)
                return false;
        }
    }

    // Don't call base class here on purpose.
    // We've done all the filtering that need's to be done.
    return true;
}

QVariant LedgerFilter::data(const QModelIndex& index, int role) const
{
    Q_D(const LedgerFilter);
    switch (role) {
    case eMyMoney::Model::ActiveFilterRole:
        return !d->filterString.isEmpty() || (d->state != State::Any);
    case eMyMoney::Model::ActiveFilterTextRole:
        return !d->filterString.isEmpty();
    case eMyMoney::Model::ActiveFilterStateRole:
        return QVariant::fromValue<LedgerFilter::State>(d->state);
    }
    return LedgerSortProxyModel::data(index, role);
}

void LedgerFilter::setStateFilter(LedgerFilter::State state)
{
    Q_D(LedgerFilter);
    d->state = state;
    invalidateFilter();
}

void LedgerFilter::setFilterFixedString(const QString& pattern)
{
    Q_D(LedgerFilter);
    d->filterString = pattern;
    invalidateFilter();
}

void LedgerFilter::clearFilter()
{
    Q_D(LedgerFilter);
    d->filterString.clear();
    d->state = State::Any;
    d->endDate = QDate();
    if (d->lineEdit)
        d->lineEdit->clear();
    if (d->comboBox)
        d->comboBox->setCurrentIndex(0);
    invalidateFilter();
}

void LedgerFilter::setEndDate(const QDate& endDate)
{
    Q_D(LedgerFilter);
    if (d->endDate != endDate) {
        d->endDate = endDate;
        invalidateFilter();
    }
}
