/*
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "daterangedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneytransactionfilter.h"
#include "widgethintframe.h"

#include "ui_daterangedlg.h"

using namespace eMyMoney;

class DateRangeDlgPrivate
{
    Q_DISABLE_COPY(DateRangeDlgPrivate)
    Q_DECLARE_PUBLIC(DateRangeDlg)

public:
    explicit DateRangeDlgPrivate(DateRangeDlg* qq)
        : q_ptr(qq)
        , ui(new Ui::DateRangeDlg)
        , m_frameCollection(new WidgetHintFrameCollection(qq))
    {
    }

    ~DateRangeDlgPrivate()
    {
        delete ui;
    }

    void setupDatePage()
    {
        Q_Q(DateRangeDlg);
        for (auto i = (int)TransactionFilter::Date::All; i < (int)TransactionFilter::Date::LastDateItem; ++i) {
            MyMoneyTransactionFilter::translateDateRange(static_cast<TransactionFilter::Date>(i), m_startDates[i], m_endDates[i]);
        }

        q->connect(ui->m_dateRange, static_cast<void (KMyMoneyPeriodCombo::*)(int)>(&KMyMoneyPeriodCombo::currentIndexChanged), q, &DateRangeDlg::slotDateRangeSelectedByUser);

        q->setDateRange(TransactionFilter::Date::All);

        m_frameCollection->addFrame(new WidgetHintFrame(ui->m_fromDate));
        m_frameCollection->addFrame(new WidgetHintFrame(ui->m_toDate));
    }

    void updateFrameStates()
    {
        WidgetHintFrame::hide(ui->m_fromDate);
        WidgetHintFrame::hide(ui->m_toDate);
        if (!ui->m_fromDate->isValid()) {
            WidgetHintFrame::show(ui->m_fromDate, i18nc("@info:tooltip", "The date is invalid."));
        }
        if (!ui->m_toDate->isValid()) {
            WidgetHintFrame::show(ui->m_toDate, i18nc("@info:tooltip", "The date is invalid."));
        }
        if (ui->m_fromDate->date().isValid() && ui->m_toDate->date().isValid() && (ui->m_fromDate->date() > ui->m_toDate->date())) {
            WidgetHintFrame::show(ui->m_fromDate,
                                  i18nc("@info:tooltip Date range error",
                                        "The date provided as start lies past the one provided for the end of the search. This will result in an empty set."));
        }
    }

    void changingDatesAdjustsRangeSelector(bool adjust)
    {
        Q_Q(DateRangeDlg);
        if (adjust) {
            q->connect(ui->m_fromDate, &KMyMoneyDateEdit::dateChanged, q, &DateRangeDlg::slotDateChanged, Qt::UniqueConnection);
            q->connect(ui->m_toDate, &KMyMoneyDateEdit::dateChanged, q, &DateRangeDlg::slotDateChanged, Qt::UniqueConnection);
        } else {
            q->disconnect(ui->m_fromDate, &KMyMoneyDateEdit::dateChanged, q, &DateRangeDlg::slotDateChanged);
            q->disconnect(ui->m_toDate, &KMyMoneyDateEdit::dateChanged, q, &DateRangeDlg::slotDateChanged);
        }
    }

    DateRangeDlg* q_ptr;
    Ui::DateRangeDlg *ui;
    WidgetHintFrameCollection* m_frameCollection;
    QDate m_startDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];
    QDate m_endDates[(int)eMyMoney::TransactionFilter::Date::LastDateItem];
};

DateRangeDlg::DateRangeDlg(QWidget *parent) :
    QWidget(parent),
    d_ptr(new DateRangeDlgPrivate(this))
{
    Q_D(DateRangeDlg);
    d->ui->setupUi(this);
    d->ui->m_fromDate->setAllowEmptyDate(true);
    d->ui->m_toDate->setAllowEmptyDate(true);
    d->setupDatePage();

    connect(d->ui->m_fromDate, &KMyMoneyDateEdit::dateValidityChanged, [&]() {
        Q_D(DateRangeDlg);
        d->updateFrameStates();
    });
    connect(d->ui->m_toDate, &KMyMoneyDateEdit::dateValidityChanged, [&]() {
        Q_D(DateRangeDlg);
        d->updateFrameStates();
    });
}

DateRangeDlg::~DateRangeDlg()
{
    Q_D(DateRangeDlg);
    delete d;
}

void DateRangeDlg::slotReset()
{
    Q_D(DateRangeDlg);
    d->ui->m_dateRange->setCurrentItem(TransactionFilter::Date::All);
    setDateRange(TransactionFilter::Date::All);
}

void DateRangeDlg::slotDateRangeSelectedByUser()
{
    Q_D(DateRangeDlg);
    setDateRange(d->ui->m_dateRange->currentItem());
}

void DateRangeDlg::setDateRange(const QDate& from, const QDate& to)
{
    Q_D(DateRangeDlg);
    d->ui->m_fromDate->setDate(from);
    d->ui->m_toDate->setDate(to);
    d->ui->m_dateRange->setCurrentItem(TransactionFilter::Date::UserDefined);
    d->updateFrameStates();
    emit rangeChanged();
}

void DateRangeDlg::setDateRange(TransactionFilter::Date idx)
{
    Q_D(DateRangeDlg);
    d->ui->m_dateRange->setCurrentItem(idx);

    d->changingDatesAdjustsRangeSelector(false);
    switch (idx) {
    case TransactionFilter::Date::All:
        d->ui->m_fromDate->setDate(QDate());
        d->ui->m_toDate->setDate(QDate());
        break;
    case TransactionFilter::Date::UserDefined:
        break;
    default:
        d->ui->m_fromDate->setDate(d->m_startDates[(int)idx]);
        d->ui->m_toDate->setDate(d->m_endDates[(int)idx]);
        break;
    }
    d->changingDatesAdjustsRangeSelector(true);

    emit rangeChanged();
}

TransactionFilter::Date DateRangeDlg::dateRange() const
{
    Q_D(const DateRangeDlg);
    return d->ui->m_dateRange->currentItem();
}

void DateRangeDlg::slotDateChanged()
{
    Q_D(DateRangeDlg);
    QSignalBlocker blocker(d->ui->m_dateRange);
    d->ui->m_dateRange->setCurrentItem(TransactionFilter::Date::UserDefined);
}

QDate DateRangeDlg::fromDate() const
{
    Q_D(const DateRangeDlg);
    return d->ui->m_fromDate->date();
}

QDate DateRangeDlg::toDate() const
{
    Q_D(const DateRangeDlg);
    return d->ui->m_toDate->date();
}
