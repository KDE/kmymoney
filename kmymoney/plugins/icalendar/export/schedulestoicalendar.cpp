/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

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
#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"
#include "mymoneyschedule.h"
#include "mymoneyaccount.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneypayee.h"
#include "mymoneyenums.h"

// plugin includes
#include "pluginsettings.h"

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
      qWarning() << "Once, any or unknown recurrence returned recurrence is invalid" << endl;
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
  foreach (const auto split, transaction.splits()) {
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

void KMMSchedulesToiCalendar::exportToFile(const QString& filePath, bool settingsChaged)
{
  if (!MyMoneyFile::instance()->storageAttached())
    return;

  QFile icsFile(filePath);

  icsFile.open(QIODevice::ReadOnly);
  QTextStream stream(&icsFile);
  d->m_icalendarAsString = stream.readAll();
  icsFile.close();

  // create the calendar
  bool newCalendar = false;
  icalcomponent* vCalendar = 0;
  if (d->m_icalendarAsString.isEmpty()) {
    newCalendar = true;
    vCalendar = icalcomponent_new_vcalendar();
  } else {
    vCalendar = icalcomponent_new_from_string(d->m_icalendarAsString.toUtf8());
    if (vCalendar == 0) {
      qDebug() << "Error parsing the following string into an icalendar:" << endl;
      qDebug() << d->m_icalendarAsString << endl;
      qDebug() << "so we will overwrite this with a new calendar" << endl;
      newCalendar = true;
      vCalendar = icalcomponent_new_vcalendar();
    }
  }

  if (vCalendar == 0) {
    // one way or the other we must have a calendar by now
    qDebug() << "Unable to create vcalendar component" << endl;
    return;
  }

  if (newCalendar) {
    // set proid and version
    icalcomponent_add_property(vCalendar, icalproperty_new_prodid("icalendarexport"));;
    icalcomponent_add_property(vCalendar, icalproperty_new_version("2.0"));
  }

  // export schedules as TODOs
  auto file = MyMoneyFile::instance();
  QList<MyMoneySchedule> schedules = file->scheduleList();
  for (QList<MyMoneySchedule>::const_iterator itSchedule = schedules.constBegin(); itSchedule != schedules.constEnd(); ++itSchedule) {
    const MyMoneySchedule& myMoneySchedule = *itSchedule;

    if (myMoneySchedule.isFinished())
      continue;  // skip this schedule if it is already finished

    icalcomponent* schedule = 0;
    bool newTodo = false;
    if (!newCalendar) {
      // try to find the schedule to update it if we do not use a new calendar
      icalcomponent* itVTODO = icalcomponent_get_first_component(vCalendar, ICAL_VTODO_COMPONENT);
      for (; itVTODO != 0; itVTODO = icalcomponent_get_next_component(vCalendar, ICAL_VTODO_COMPONENT)) {
        if (icalcomponent_get_uid(itVTODO) == myMoneySchedule.id()) {
          // we found our todo stop searching
          schedule = itVTODO;
          break;
        }
      }
      if (schedule == 0) {
        schedule = icalcomponent_new_vtodo();
        newTodo = true;
      }
    } else {
      schedule = icalcomponent_new_vtodo();
      newTodo = true;
    }

    // description
    icalcomponent_set_description(schedule, scheduleToDescription(myMoneySchedule).toUtf8());
    // summary
    icalcomponent_set_summary(schedule, myMoneySchedule.name().toUtf8());
    // uid
    icalcomponent_set_uid(schedule, myMoneySchedule.id().toUtf8());
    // dtstart
    icalcomponent_set_dtstart(schedule, qdateToIcalTimeType(myMoneySchedule.startDate()));
    // due
    icalcomponent_set_due(schedule, qdateToIcalTimeType(myMoneySchedule.nextDueDate()));
    if (newTodo) {
      // created
      icalcomponent_add_property(schedule, icalproperty_new_created(qdateTimeToIcalTimeType(QDateTime::currentDateTime())));
    } else {
      // last modified
      icalproperty* pLastMod = icalcomponent_get_first_property(schedule, ICAL_LASTMODIFIED_PROPERTY);
      if (pLastMod != 0) {
        // set the current property
        icalproperty_set_lastmodified(pLastMod, qdateTimeToIcalTimeType(QDateTime::currentDateTime()));
      } else {
        // create a new property
        icalcomponent_add_property(schedule, icalproperty_new_lastmodified(qdateTimeToIcalTimeType(QDateTime::currentDateTime())));
      }
    }
    // recurrence
    icalproperty* pRRule = icalcomponent_get_first_property(schedule, ICAL_RRULE_PROPERTY);
    if (pRRule != 0) {
      icalcomponent_remove_property(schedule, pRRule);
    }
    if (myMoneySchedule.occurrence() != Schedule::Occurrence::Once && myMoneySchedule.baseOccurrence() != Schedule::Occurrence::Any)
      icalcomponent_add_property(schedule, icalproperty_new_rrule(scheduleToRecurenceRule(myMoneySchedule)));

    icalcomponent* oldAlarm = icalcomponent_get_first_component(schedule, ICAL_VALARM_COMPONENT);
    if (oldAlarm && settingsChaged)
      icalcomponent_remove_component(schedule, oldAlarm);

    if (PluginSettings::createAlarm() && (!oldAlarm || settingsChaged)) {
      // alarm: beginning with one day before the todo is due every one hour
      icalcomponent* alarm = icalcomponent_new_valarm();
      // alarm: action
      icalcomponent_add_property(alarm, icalproperty_new_action(ICAL_ACTION_DISPLAY));
      // alarm: description
      icalcomponent_set_description(alarm, scheduleToDescription(myMoneySchedule).toUtf8());
      // alarm: trigger
      int triggerInterval = beforeAfterToInt(PluginSettings::beforeAfter()) * PluginSettings::timeUnits() * timeUnitsInSeconds(PluginSettings::timeUnitInSeconds());
      icalcomponent_add_property(alarm, icalproperty_new_trigger(icaltriggertype_from_int(triggerInterval)));
      // alarm: duration
      int intervalBetweenReminders = PluginSettings::intervalBetweenRemindersTimeUnits() * timeUnitsInSeconds(PluginSettings::intervalBetweenRemindersTimeUnitInSeconds());
      icalcomponent_set_duration(alarm, icaldurationtype_from_int(intervalBetweenReminders));
      if (PluginSettings::repeatingReminders()) {
        // alarm: repeat
        icalcomponent_add_property(alarm, icalproperty_new_repeat(PluginSettings::numberOfReminders()));
      }
      // add the alarm to the schedule
      icalcomponent_add_component(schedule, alarm);
    }

    // add the schedule to the calendar
    if (newTodo)
      icalcomponent_add_component(vCalendar, schedule);
  }

  icsFile.open(QIODevice::WriteOnly);

  d->m_icalendarAsString = QString::fromUtf8(icalcomponent_as_ical_string(vCalendar));
  // reclaim some memory :)
  icalcomponent_free(vCalendar);
  // write the calendar to the file
  stream << d->m_icalendarAsString << endl;
  icsFile.close();
}
