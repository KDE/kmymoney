/***************************************************************************
                             ksettingsschedules.cpp
                             --------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#include <config-kmymoney.h>

#include "ksettingsschedules.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdeversion.h>
#include <klocale.h>
#include <klocalizedstring.h>
#ifdef KF5Holidays_FOUND
#include <KHolidays/Holiday>
using namespace KHolidays;
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney/kmymoneyglobalsettings.h"

KSettingsSchedules::KSettingsSchedules(QWidget* parent) :
    KSettingsSchedulesDecl(parent)
{
  // hide the internally used holidayRegion field
  kcfg_HolidayRegion->hide();

  loadList();

  // setup connections so that region gets selected once field is filled
  connect(kcfg_HolidayRegion, SIGNAL(textChanged(QString)), this, SLOT(slotLoadRegion(QString)));

  // setup connections so that changes are forwarded to the field
  connect(m_holidayRegion, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSetRegion(QString)));
}

void KSettingsSchedules::loadList()
{
  QStringList regions;
#ifdef KF5Holidays_FOUND
  QStringList regionCodes = HolidayRegion::regionCodes();

  foreach (const QString &regionCode, regionCodes) {
    QString regionName = HolidayRegion::name(regionCode);
    QString languageName = KLocale::global()->languageCodeToName(HolidayRegion::languageCode(regionCode));
    QString region;
    if (languageName.isEmpty())
      region = regionName;
    else
      region = i18nc("Holiday region (region language)", "%1 (%2)", regionName, languageName);

    m_regionMap[region] = regionCode;
    regions << region;
  }
  regions.sort();
#endif

  m_regionMap[m_holidayRegion->itemText(0)] = "";
  m_holidayRegion->insertItems(1, regions);
}

void KSettingsSchedules::slotSetRegion(const QString &region)
{
  kcfg_HolidayRegion->setText(m_regionMap[region]);
}

void KSettingsSchedules::slotLoadRegion(const QString &region)
{
  // only need this once
  disconnect(kcfg_HolidayRegion, SIGNAL(textChanged(QString)), this, SLOT(slotLoadRegion(QString)));
  int i = 0;
  if (!region.isEmpty())
    i = m_holidayRegion->findText(m_regionMap.key(region));
  if ((i > -1) && (i != m_holidayRegion->currentIndex())) {
    m_holidayRegion->blockSignals(true);
    m_holidayRegion->setCurrentIndex(i);
    m_holidayRegion->blockSignals(false);
  }
}

void KSettingsSchedules::slotResetRegion()
{
  slotLoadRegion(kcfg_HolidayRegion->text());
}

KSettingsSchedules::~KSettingsSchedules()
{
}
