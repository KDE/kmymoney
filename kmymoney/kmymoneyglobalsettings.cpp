/***************************************************************************
                          kmymoneyglobalsettings.cpp
                             -------------------
    copyright            : (C) 2006 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneyglobalsettings.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
#include <QFontDatabase>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyforecast.h"

QFont KMyMoneyGlobalSettings::listCellFont()
{
  if (useSystemFont()) {
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  } else {
    return KMyMoneySettings::listCellFont();
  }
}

QFont KMyMoneyGlobalSettings::listHeaderFont()
{
  if (useSystemFont()) {
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setBold(true);
    return font;
  } else {
    return KMyMoneySettings::listHeaderFont();
  }
}

QColor KMyMoneyGlobalSettings::schemeColor(const SchemeColor color)
{
  switch(color) {
    case SchemeColor::ListBackground1:
      return KColorScheme (QPalette::Active, KColorScheme::View).background(KColorScheme::NormalBackground).color();
    case SchemeColor::ListBackground2:
      return KColorScheme (QPalette::Active, KColorScheme::View).background(KColorScheme::AlternateBackground).color();
    case SchemeColor::ListGrid:
      return KColorScheme (QPalette::Active, KColorScheme::View).foreground(KColorScheme::InactiveText).color();
    case SchemeColor::ListHighlightText:
      return KColorScheme (QPalette::Active, KColorScheme::Selection).foreground(KColorScheme::NormalText).color();
    case SchemeColor::ListHighlight:
      return KColorScheme (QPalette::Active, KColorScheme::Selection).background(KColorScheme::NormalBackground).color();
    case SchemeColor::WindowText:
      return KColorScheme (QPalette::Active, KColorScheme::Window).foreground(KColorScheme::NormalText).color();
    case SchemeColor::WindowBackground:
      return KColorScheme (QPalette::Active, KColorScheme::Window).background(KColorScheme::NormalBackground).color();
    case SchemeColor::Positive:
      return KColorScheme (QPalette::Active, KColorScheme::View).foreground(KColorScheme::PositiveText).color();
    case SchemeColor::Negative:
      return KColorScheme (QPalette::Active, KColorScheme::View).foreground(KColorScheme::NegativeText).color();
    case SchemeColor::TransactionImported:
      if (useCustomColors())
        return KMyMoneySettings::transactionImportedColor();
      else
        return KColorScheme (QPalette::Active, KColorScheme::Complementary).background(KColorScheme::NeutralBackground).color();
    case SchemeColor::TransactionMatched:
      if (useCustomColors())
        return KMyMoneySettings::transactionMatchedColor();
      else
      return KColorScheme (QPalette::Active, KColorScheme::Complementary).background(KColorScheme::PositiveBackground).color();
    case SchemeColor::TransactionErroneous:
      if (useCustomColors())
        return KMyMoneySettings::transactionErroneousColor();
      else
      return KColorScheme (QPalette::Active, KColorScheme::View).foreground(KColorScheme::NegativeText).color();
    case SchemeColor::FieldRequired:
      if (useCustomColors())
        return KMyMoneySettings::fieldRequiredColor();
      else
        return KColorScheme (QPalette::Active, KColorScheme::View).background(KColorScheme::NeutralBackground).color();
    case SchemeColor::GroupMarker:
      if (useCustomColors())
        return KMyMoneySettings::groupMarkerColor();
      else
      return KColorScheme (QPalette::Active, KColorScheme::Selection).background(KColorScheme::LinkBackground).color();
    case SchemeColor::MissingConversionRate:
      if (useCustomColors())
        return KMyMoneySettings::missingConversionRateColor();
      else
      return KColorScheme (QPalette::Active, KColorScheme::Complementary).foreground(KColorScheme::LinkText).color();
    default:
      return QColor();

  }
}

QStringList KMyMoneyGlobalSettings::itemList()
{
  bool prevValue = self()->useDefaults(true);
  QStringList all = KMyMoneySettings::itemList().split(',', QString::SkipEmptyParts);
  self()->useDefaults(prevValue);
  QStringList list = KMyMoneySettings::itemList().split(',', QString::SkipEmptyParts);

  // now add all from 'all' that are missing in 'list'
  QRegExp exp("-?(\\d+)");
  QStringList::iterator it_s;
  for (it_s = all.begin(); it_s != all.end(); ++it_s) {
    exp.indexIn(*it_s);
    if (!list.contains(exp.cap(1)) && !list.contains(QString("-%1").arg(exp.cap(1)))) {
      list << *it_s;
    }
  }
  return list;
}

int KMyMoneyGlobalSettings::firstFiscalMonth()
{
  return KMyMoneySettings::fiscalYearBegin() + 1;
}

int KMyMoneyGlobalSettings::firstFiscalDay()
{
  return KMyMoneySettings::fiscalYearBeginDay();
}

QDate KMyMoneyGlobalSettings::firstFiscalDate()
{
  QDate date = QDate(QDate::currentDate().year(), firstFiscalMonth(), firstFiscalDay());
  if (date > QDate::currentDate())
    date = date.addYears(-1);
  return date;
}

MyMoneyForecast KMyMoneyGlobalSettings::forecast()
{
  MyMoneyForecast forecast;

  // override object defaults with those of the application
  forecast.setForecastCycles(KMyMoneyGlobalSettings::forecastCycles());
  forecast.setAccountsCycle(KMyMoneyGlobalSettings::forecastAccountCycle());
  forecast.setHistoryStartDate(QDate::currentDate().addDays(-forecast.forecastCycles()*forecast.accountsCycle()));
  forecast.setHistoryEndDate(QDate::currentDate().addDays(-1));
  forecast.setForecastDays(KMyMoneyGlobalSettings::forecastDays());
  forecast.setBeginForecastDay(KMyMoneyGlobalSettings::beginForecastDay());
  forecast.setForecastMethod(KMyMoneyGlobalSettings::forecastMethod());
  forecast.setHistoryMethod(KMyMoneyGlobalSettings::historyMethod());
  forecast.setIncludeFutureTransactions(KMyMoneyGlobalSettings::includeFutureTransactions());
  forecast.setIncludeScheduledTransactions(KMyMoneyGlobalSettings::includeScheduledTransactions());

  return forecast;
}
