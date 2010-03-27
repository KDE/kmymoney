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

#include "ksettingsschedules.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KStandardDirs>
#include <KHolidays/Holidays>
using namespace KHolidays;

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
  connect(kcfg_HolidayRegion, SIGNAL(textChanged(const QString&)), this, SLOT(slotLoadRegion(const QString&)));

  // setup connections so that changes are forwarded to the field
  connect(m_holidayRegion, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(slotSetRegion(const QString&)));
}

void KSettingsSchedules::loadList(void)
{
  QStringList countries = HolidayRegion::locations();
  QStringList regions;

  foreach (const QString &country, countries) {
    QString file = KStandardDirs::locate("locale",
                                         "l10n/" + country + "/entry.desktop");
    QString region;
    if (!file.isEmpty()) {
      KConfig entry(file, KConfig::SimpleConfig);
      KConfigGroup grp = entry.group("KCM Locale");
      region = grp.readEntry("Name");
    }
    if (region.isEmpty())
      region = country;

    m_regionMap[region] = country;
    regions << region;
  }
  regions.sort();

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
  disconnect(kcfg_HolidayRegion, SIGNAL(textChanged(const QString&)), this, SLOT(slotLoadRegion(const QString&)));
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

#include "ksettingsschedules.moc"
