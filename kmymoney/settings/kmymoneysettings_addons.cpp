/*
    SPDX-FileCopyrightText: 2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
#include <QFontDatabase>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

QFont KMyMoneySettings::listCellFontEx()
{
  if (useSystemFont()) {
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  } else {
    return listCellFont();
  }
}

QFont KMyMoneySettings::listHeaderFontEx()
{
  if (useSystemFont()) {
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setBold(true);
    return font;
  } else {
    return listHeaderFont();
  }
}

QColor KMyMoneySettings::schemeColor(const SchemeColor color)
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
        return transactionImportedColor();
      else
        return KColorScheme (QPalette::Active, KColorScheme::View).background(KColorScheme::LinkBackground).color();
    case SchemeColor::TransactionMatched:
      if (useCustomColors())
        return transactionMatchedColor();
      else
        return KColorScheme (QPalette::Active, KColorScheme::View).background(KColorScheme::LinkBackground).color();
    case SchemeColor::TransactionErroneous:
      if (useCustomColors())
        return transactionErroneousColor();
      else
      return KColorScheme (QPalette::Active, KColorScheme::View).foreground(KColorScheme::NegativeText).color();
    case SchemeColor::FieldRequired:
      if (useCustomColors())
        return fieldRequiredColor();
      else
        return KColorScheme (QPalette::Active, KColorScheme::View).background(KColorScheme::NeutralBackground).color();
    case SchemeColor::GroupMarker:
      if (useCustomColors())
        return groupMarkerColor();
      else
      return KColorScheme (QPalette::Active, KColorScheme::Selection).background(KColorScheme::LinkBackground).color();
    case SchemeColor::MissingConversionRate:
      if (useCustomColors())
        return missingConversionRateColor();
      else
      return KColorScheme (QPalette::Active, KColorScheme::Complementary).foreground(KColorScheme::LinkText).color();
    default:
      return QColor();

  }
}

QStringList KMyMoneySettings::listOfItems()
{
  bool prevValue = self()->useDefaults(true);
  QStringList all = itemList().split(',', QString::SkipEmptyParts);
  self()->useDefaults(prevValue);
  QStringList list = itemList().split(',', QString::SkipEmptyParts);

  // now add all from 'all' that are missing in 'list'
  QRegExp exp("-?(\\d+)");
  QStringList::iterator it_s;
  for (it_s = all.begin(); it_s != all.end(); ++it_s) {
    if ((exp.indexIn(*it_s) != -1) && !list.contains(exp.cap(1)) && !list.contains(QString("-%1").arg(exp.cap(1)))) {
      list << *it_s;
    }
  }
  return list;
}

int KMyMoneySettings::firstFiscalMonth()
{
  return fiscalYearBegin() + 1;
}

int KMyMoneySettings::firstFiscalDay()
{
  return fiscalYearBeginDay();
}

QDate KMyMoneySettings::firstFiscalDate()
{
  QDate date = QDate(QDate::currentDate().year(), firstFiscalMonth(), firstFiscalDay());
  if (date > QDate::currentDate())
    date = date.addYears(-1);
  return date;
}

