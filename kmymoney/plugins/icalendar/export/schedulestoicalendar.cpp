/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "schedulestoicalendar.h"

#include <QDateTime>
#include <QLocale>
#include <QFile>

// KDE includes
#include <KPluginFactory>
#include <KLocalizedString>
#include <KFile>

// libical includes
#include <libical/ical.h>

// KMyMoney includes
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyutils.h"
#include "schedulesmodel.h"

// plugin includes
#include "icalendarsettings.h"

using namespace eMyMoney;

int timeUnitsInSeconds(int optionValue)
{
    // see how the items are added in the combobox of the settings editor widget
    static const int minute = 0;
    static const int hour = 1;
    static const int day = 2;

    switch (optionValue) {
    case minute:
        return 60;
    case hour:
        return 60*60;
    case day:
        return 24*60*60;
    default:
        return 1;
    }
}

int beforeAfterToInt(int optionValue)
{
    // see how the items are added in the combobox of the settings editor widget
    static const int before = 0;
    static const int after = 1;

    switch (optionValue) {
    case before:
        return -1;
    case after:
        return 1;
    default:
        return -1;
    }
}

struct icaltimetype qdateToIcalTimeType(const QDate& date) {
    struct icaltimetype icalDate = icaltime_null_date();
    icalDate.year = date.year();
    icalDate.month = date.month();
    icalDate.day = date.day();
    icalDate.is_date = 1;
    return icalDate;
}

struct icaltimetype qdateTimeToIcalTimeType(const QDateTime& dateTime) {
    struct icaltimetype icalDateTime = icaltime_null_date();
    icalDateTime.year = dateTime.date().year();
    icalDateTime.month = dateTime.date().month();
    icalDateTime.day = dateTime.date().day();
    icalDateTime.hour = dateTime.time().hour();
    icalDateTime.minute = dateTime.time().minute();
    icalDateTime.second = dateTime.time().second();
    icalDateTime.is_date = 0;
    return icalDateTime;
}

struct icalrecurrencetype scheduleToRecurenceRule(const MyMoneySchedule& schedule) {
    struct icalrecurrencetype recurrence;
    icalrecurrencetype_clear(&recurrence);
    if (schedule.willEnd())
        recurrence.until = qdateToIcalTimeType(schedule.endDate());
    recurrence.week_start = icalrecurrencetype_day_day_of_week(QLocale().firstDayOfWeek());
    int frequencyFactor = 1; // used to translate kmymoney frequency to icalendar frequency

    switch (schedule.baseOccurrence()) {
    case Schedule::Occurrence::Daily:
        recurrence.freq = ICAL_DAILY_RECURRENCE;
        break;
    case Schedule::Occurrence::Weekly:
        recurrence.freq = ICAL_WEEKLY_RECURRENCE;
        break;
    case Schedule::Occurrence::Fortnightly:
        recurrence.freq = ICAL_WEEKLY_RECURRENCE;
        frequencyFactor = 2;
        break;
    case Schedule::Occurrence::EveryOtherWeek:
        recurrence.freq = ICAL_WEEKLY_RECURRENCE;
        frequencyFactor = 2;
        break;
    case Schedule::Occurrence::EveryHalfMonth:
        recurrence.freq = ICAL_WEEKLY_RECURRENCE;
        frequencyFactor = 2;
        break;
    case Schedule::Occurrence::EveryThreeWeeks:
        recurrence.freq = ICAL_WEEKLY_RECURRENCE;
        frequencyFactor = 3;
        break;
    case Schedule::Occurrence::EveryThirtyDays:
        recurrence.freq = ICAL_DAILY_RECURRENCE;
        frequencyFactor = 30;
        break;
    case Schedule::Occurrence::Monthly:
        recurrence.freq = ICAL_MONTHLY_RECURRENCE;
        break;
    case Schedule::Occurrence::EveryFourWeeks:
        recurrence.freq = ICAL_WEEKLY_RECURRENCE;
        frequencyFactor = 4;
        break;
    case Schedule::Occurrence::EveryEightWeeks:
        recurrence.freq = ICAL_WEEKLY_RECURRENCE;
        frequencyFactor = 8;
        break;
    case Schedule::Occurrence::EveryOtherMonth:
        recurrence.freq = ICAL_MONTHLY_RECURRENCE;
        frequencyFactor = 2;
        break;
    case Schedule::Occurrence::EveryThreeMonths:
        recurrence.freq = ICAL_MONTHLY_RECURRENCE;
        frequencyFactor = 3;
        break;
    case Schedule::Occurrence::TwiceYearly:
        recurrence.freq = ICAL_MONTHLY_RECURRENCE;
        frequencyFactor = 6;
        break;
    case Schedule::Occurrence::EveryOtherYear:
        recurrence.freq = ICAL_YEARLY_RECURRENCE;
        frequencyFactor = 2;
        break;
    case Schedule::Occurrence::Quarterly:
        recurrence.freq = ICAL_MONTHLY_RECURRENCE;
        frequencyFactor = 3;
        break;
    case Schedule::Occurrence::EveryFourMonths:
        recurrence.freq = ICAL_MONTHLY_RECURRENCE;
        frequencyFactor = 4;
        break;
    case Schedule::Occurrence::Yearly:
        recurrence.freq = ICAL_YEARLY_RECURRENCE;
        break;
    case Schedule::Occurrence::Once:
    case Schedule::Occurrence::Any:
    default:
        qWarning() << "Once, any or unknown recurrence returned recurrence is invalid" << Qt::endl;
        recurrence.freq = ICAL_NO_RECURRENCE;
        break;
    }
    recurrence.interval = frequencyFactor*schedule.occurrenceMultiplier();
    return recurrence;
}

QString scheduleToDescription(const MyMoneySchedule& schedule)
{
    auto file = MyMoneyFile::instance();
    const MyMoneyAccount& account = schedule.account();

    const MyMoneyTransaction& transaction = schedule.transaction();
    QString payeeName;

    MyMoneyMoney amount;
    QString category;
    bool isTransfer = false;
    bool isIncome = false;
    Q_FOREACH (const auto split, transaction.splits()) {
        if (split.accountId() != account.id()) {
            if (!category.isEmpty())
                category += ", "; // this is a split transaction
            const MyMoneyAccount& splitAccount = file->account(split.accountId());
            category = splitAccount.name();

            isTransfer = splitAccount.accountGroup() == Account::Type::Asset ||
                         splitAccount.accountGroup() == Account::Type::Liability;
            isIncome = splitAccount.accountGroup() == Account::Type::Income;
        } else {
            payeeName = file->payee(split.payeeId()).name();
            // make the amount positive since the message makes it clear if this is an income or expense
            amount = split.shares().abs();
        }
    }

    QString description =
        isTransfer ? i18n("Transfer from %1 to %2, Payee %3, amount %4", account.name(), category, payeeName, MyMoneyUtils::formatMoney(amount, file->currency(account.currencyId())))
        : (
            isIncome ? i18n("From %1 into %2, Category %3, sum of %4", payeeName, account.name(), category, MyMoneyUtils::formatMoney(amount, file->currency(account.currencyId())))
            : i18n("From account %1, Pay to %2, Category %3, sum of %4", account.name(), payeeName, category, MyMoneyUtils::formatMoney(amount, file->currency(account.currencyId())))
        );
    if (!transaction.memo().isEmpty())
        description = i18nc<QString, QString>("The first string is the schedules details", "%1, memo %2", description, transaction.memo());
    return description;
}

struct KMMSchedulesToiCalendar::Private {
    QString m_icalendarAsString;
};

KMMSchedulesToiCalendar::KMMSchedulesToiCalendar() : d(new Private)
{
}

KMMSchedulesToiCalendar::~KMMSchedulesToiCalendar()
{
    delete d;
}

void KMMSchedulesToiCalendar::exportToFile(const QString& filePath, bool writeEventVsTodo)
{
    QFile icsFile(filePath);

    const icalcomponent_kind newEntryKind = writeEventVsTodo ? ICAL_VEVENT_COMPONENT : ICAL_VTODO_COMPONENT;
    icsFile.open(QIODevice::ReadOnly);
    QTextStream stream(&icsFile);
    d->m_icalendarAsString = stream.readAll();
    icsFile.close();

    // create the calendar
    bool newCalendar = false;
    icalcomponent* vCalendar = nullptr;
    struct icaltimetype atime = icaltime_from_timet_with_zone(time(0), 0, icaltimezone_get_utc_timezone());

    if (d->m_icalendarAsString.isEmpty()) {
        newCalendar = true;
        vCalendar = icalcomponent_new_vcalendar();
    } else {
        vCalendar = icalcomponent_new_from_string(d->m_icalendarAsString.toUtf8());
        if (vCalendar == nullptr) {
            qDebug() << "Error parsing the following string into an icalendar:" << Qt::endl;
            qDebug() << d->m_icalendarAsString << Qt::endl;
            qDebug() << "so we will overwrite this with a new calendar" << Qt::endl;
            newCalendar = true;
            vCalendar = icalcomponent_new_vcalendar();
        } else {
            // we have a calendar and must check if the entry kind was changed
            icalcomponent* itTodo = icalcomponent_get_first_component(vCalendar, ICAL_VTODO_COMPONENT);
            icalcomponent* itEvent = icalcomponent_get_first_component(vCalendar, ICAL_VEVENT_COMPONENT);
            if (((itTodo != nullptr) && writeEventVsTodo) || (((itEvent != nullptr) && !writeEventVsTodo))) {
                icalcomponent_free(vCalendar);
                newCalendar = true;
                vCalendar = icalcomponent_new_vcalendar();
            }
        }
    }

    if (vCalendar == nullptr) {
        // one way or the other we must have a calendar by now
        qDebug() << "Unable to create vcalendar component" << Qt::endl;
        return;
    }

    if (newCalendar) {
        // set proid and version
        icalcomponent_add_property(vCalendar, icalproperty_new_prodid("icalendarexport"));
        icalcomponent_add_property(vCalendar, icalproperty_new_version("2.0"));
    }

    // export schedules as TODOs
    auto file = MyMoneyFile::instance();
    QList<MyMoneySchedule> schedules = file->scheduleList();
    for (QList<MyMoneySchedule>::const_iterator itSchedule = schedules.constBegin(); itSchedule != schedules.constEnd(); ++itSchedule) {
        const MyMoneySchedule& myMoneySchedule = *itSchedule;

        if (myMoneySchedule.isFinished())
            continue;  // skip this schedule if it is already finished

        icalcomponent* schedule = nullptr;
        bool newEntry = false;
        if (!newCalendar) {
            // try to find the schedule to update it if we do not use a new calendar
            icalcomponent* itEntry = icalcomponent_get_first_component(vCalendar, newEntryKind);
            for (; itEntry != nullptr; itEntry = icalcomponent_get_next_component(vCalendar, newEntryKind)) {
                if (icalcomponent_get_uid(itEntry) == myMoneySchedule.id()) {
                    // we found our todo stop searching
                    schedule = itEntry;
                    break;
                }
            }
            if (schedule == nullptr) {
                schedule = writeEventVsTodo ? icalcomponent_new_vevent() : icalcomponent_new_vtodo();
                newEntry = true;
            }
        } else {
            schedule = writeEventVsTodo ? icalcomponent_new_vevent() : icalcomponent_new_vtodo();
            newEntry = true;
        }

        // description
        icalcomponent_set_description(schedule, scheduleToDescription(myMoneySchedule).toUtf8());
        // summary
        icalcomponent_set_summary(schedule, myMoneySchedule.name().toUtf8());
        // uid
        icalcomponent_set_uid(schedule, myMoneySchedule.id().toUtf8());
        // dtstart
        icalcomponent_set_dtstart(schedule, qdateToIcalTimeType(myMoneySchedule.startDate()));

        // due (only supported for VTODO)
        if (newEntryKind == ICAL_VTODO_COMPONENT) {
            icalcomponent_set_due(schedule, qdateToIcalTimeType(myMoneySchedule.nextDueDate()));
        }
        // dtstamp
        icalproperty* dtstamp = icalcomponent_get_first_property(schedule, ICAL_DTSTAMP_PROPERTY);
        if (dtstamp != nullptr) {
            icalcomponent_remove_property(schedule, dtstamp);
        }
        icalcomponent_add_property(schedule, icalproperty_new_dtstamp(atime));

        if (newEntry) {
            // created
            icalcomponent_add_property(schedule, icalproperty_new_created(qdateTimeToIcalTimeType(QDateTime::currentDateTime())));
        } else {
            // last modified
            icalproperty* pLastMod = icalcomponent_get_first_property(schedule, ICAL_LASTMODIFIED_PROPERTY);
            if (pLastMod != nullptr) {
                // set the current property
                icalproperty_set_lastmodified(pLastMod, qdateTimeToIcalTimeType(QDateTime::currentDateTime()));
            } else {
                // create a new property
                icalcomponent_add_property(schedule, icalproperty_new_lastmodified(qdateTimeToIcalTimeType(QDateTime::currentDateTime())));
            }
        }
        // recurrence
        icalproperty* pRRule = icalcomponent_get_first_property(schedule, ICAL_RRULE_PROPERTY);
        if (pRRule != nullptr) {
            icalcomponent_remove_property(schedule, pRRule);
        }
        if (myMoneySchedule.occurrence() != Schedule::Occurrence::Once && myMoneySchedule.baseOccurrence() != Schedule::Occurrence::Any)
            icalcomponent_add_property(schedule, icalproperty_new_rrule(scheduleToRecurenceRule(myMoneySchedule)));

        icalcomponent* alarm = icalcomponent_get_first_component(schedule, ICAL_VALARM_COMPONENT);
        if (alarm)
            icalcomponent_remove_component(schedule, alarm);

        if (ICalendarSettings::createAlarm()) {
            // alarm: beginning with one day before the todo is due every one hour
            alarm = icalcomponent_new_valarm();
            // alarm: action
            icalcomponent_add_property(alarm, icalproperty_new_action(ICAL_ACTION_DISPLAY));
            // alarm: description
            icalcomponent_set_description(alarm, scheduleToDescription(myMoneySchedule).toUtf8());
            // alarm: trigger
            int triggerInterval = beforeAfterToInt(ICalendarSettings::beforeAfter()) * ICalendarSettings::timeUnits() * timeUnitsInSeconds(ICalendarSettings::timeUnitInSeconds());
            icalcomponent_add_property(alarm, icalproperty_new_trigger(icaltriggertype_from_int(triggerInterval)));
            // alarm: duration
            int intervalBetweenReminders = ICalendarSettings::intervalBetweenRemindersTimeUnits() * timeUnitsInSeconds(ICalendarSettings::intervalBetweenRemindersTimeUnitInSeconds());
            icalcomponent_set_duration(alarm, icaldurationtype_from_int(intervalBetweenReminders));
            if (ICalendarSettings::repeatingReminders()) {
                // alarm: repeat
                icalcomponent_add_property(alarm, icalproperty_new_repeat(ICalendarSettings::numberOfReminders()));
            }
            // add the alarm to the schedule
            icalcomponent_add_component(schedule, alarm);
        }

        // add the schedule to the calendar
        if (newEntry)
            icalcomponent_add_component(vCalendar, schedule);
    }

    // now remove the ones that have been deleted by the user
    icalcomponent* itEntry = icalcomponent_get_first_component(vCalendar, newEntryKind);

    while ((itEntry = icalcomponent_get_current_component(vCalendar)) != nullptr) {
        const QString ical_uid = icalcomponent_get_uid(itEntry);
        if (!file->schedulesModel()->indexById(ical_uid).isValid()) {
            icalcomponent_remove_component(vCalendar, itEntry);
        } else {
            icalcomponent_get_next_component(vCalendar, newEntryKind);
        }
    }

    // write out the ics file

    icsFile.open(QIODevice::WriteOnly);
    d->m_icalendarAsString = QString::fromUtf8(icalcomponent_as_ical_string(vCalendar));
    // reclaim some memory :)
    icalcomponent_free(vCalendar);
    // write the calendar to the file
    stream << d->m_icalendarAsString << Qt::endl;
    icsFile.close();
}
