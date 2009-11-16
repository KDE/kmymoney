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

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGlobalSettings>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

QFont KMyMoneyGlobalSettings::listCellFont(void)
{
  if(useSystemFont()) {
    return KGlobalSettings::generalFont();
  } else {
    return KMyMoneySettings::listCellFont();
  }
}

QFont KMyMoneyGlobalSettings::listHeaderFont(void)
{
  if(useSystemFont()) {
    QFont font = KGlobalSettings::generalFont();
    font.setBold(true);
    return font;
  } else {
    return KMyMoneySettings::listHeaderFont();
  }
}

QColor KMyMoneyGlobalSettings::listColor(void)
{
  if(useSystemColors()) {
    KColorScheme scheme(QPalette::Normal);
    return scheme.background(KColorScheme::NormalBackground).color();
  } else {
    return KMyMoneySettings::listColor();
  }
}

QColor KMyMoneyGlobalSettings::listBGColor(void)
{
  if(useSystemColors()) {
    KColorScheme scheme(QPalette::Normal);
    return scheme.background(KColorScheme::AlternateBackground).color();
  } else {
    return KMyMoneySettings::listBGColor();
  }
}

QStringList KMyMoneyGlobalSettings::itemList(void)
{
  bool prevValue = self()->useDefaults(true);
  QStringList all = KMyMoneySettings::itemList().split(',', QString::SkipEmptyParts);
  self()->useDefaults(prevValue);
  QStringList list = KMyMoneySettings::itemList().split(',', QString::SkipEmptyParts);

  // now add all from 'all' that are missing in 'list'
  QRegExp exp("-?(\\d+)");
  QStringList::iterator it_s;
  for(it_s = all.begin(); it_s != all.end(); ++it_s) {
    exp.indexIn(*it_s);
    if(!list.contains(exp.cap(1)) && !list.contains(QString("-%1").arg(exp.cap(1)))) {
      list << *it_s;
    }
  }
  return list;
}

int KMyMoneyGlobalSettings::firstFiscalMonth(void)
{
  return KMyMoneySettings::fiscalYearBegin()+1;
}

int KMyMoneyGlobalSettings::firstFiscalDay(void)
{
  return KMyMoneySettings::fiscalYearBeginDay();
}

