/*
 * Copyright 2005-2010  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config-kmymoney.h>

#include "ksettingsschedules.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#ifdef KF5Holidays_FOUND
#include <KHolidays/Holiday>
#include <KHolidays/HolidayRegion>
using namespace KHolidays;
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsschedules.h"


class KSettingsSchedulesPrivate
{
  Q_DISABLE_COPY(KSettingsSchedulesPrivate)

public:
  KSettingsSchedulesPrivate() :
    ui(new Ui::KSettingsSchedules)
  {
  }

  ~KSettingsSchedulesPrivate()
  {
    delete ui;
  }

  Ui::KSettingsSchedules *ui;
  QMap<QString, QString> m_regionMap;
};

KSettingsSchedules::KSettingsSchedules(QWidget* parent) :
  QWidget(parent),
  d_ptr(new KSettingsSchedulesPrivate)
{
  Q_D(KSettingsSchedules);
  d->ui->setupUi(this);
  // hide the internally used holidayRegion field
  d->ui->kcfg_HolidayRegion->hide();

  loadList();

  // setup connections so that region gets selected once field is filled
  connect(d->ui->kcfg_HolidayRegion, &QLineEdit::textChanged, this, &KSettingsSchedules::slotLoadRegion);

  // setup connections so that changes are forwarded to the field
  connect(d->ui->m_holidayRegion, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &KSettingsSchedules::slotSetRegion);
}

KSettingsSchedules::~KSettingsSchedules()
{
  Q_D(KSettingsSchedules);
  delete d;
}

void KSettingsSchedules::loadList()
{
  Q_D(KSettingsSchedules);
  QStringList regions;
#ifdef KF5Holidays_FOUND
  QStringList regionCodes = HolidayRegion::regionCodes();

  foreach (const QString &regionCode, regionCodes) {
    const auto regionName( HolidayRegion::name(regionCode));
    const QLocale langLocale(HolidayRegion::languageCode(regionCode));
    const auto languageName = QLocale().languageToString(langLocale.language());
    const auto description = HolidayRegion::description(regionCode);
    const QString region = !description.isEmpty() ? description : i18nc("Holiday region (region language)", "Holidays for %1 (%2)", regionName, languageName);
    d->m_regionMap[region] = regionCode;
    regions << region;
  }
  regions.sort();
#endif

  d->m_regionMap[d->ui->m_holidayRegion->itemText(0)] = QString();
  d->ui->m_holidayRegion->insertItems(1, regions);
}

void KSettingsSchedules::slotSetRegion(const QString &region)
{
  Q_D(KSettingsSchedules);
  d->ui->kcfg_HolidayRegion->setText(d->m_regionMap[region]);
}

void KSettingsSchedules::slotLoadRegion(const QString &region)
{
  Q_D(KSettingsSchedules);
  // only need this once
  disconnect(d->ui->kcfg_HolidayRegion, &KLineEdit::textChanged, this, &KSettingsSchedules::slotLoadRegion);
  auto i = 0;
  if (!region.isEmpty())
    i = d->ui->m_holidayRegion->findText(d->m_regionMap.key(region));
  if ((i > -1) && (i != d->ui->m_holidayRegion->currentIndex())) {
    d->ui->m_holidayRegion->blockSignals(true);
    d->ui->m_holidayRegion->setCurrentIndex(i);
    d->ui->m_holidayRegion->blockSignals(false);
  }
}

void KSettingsSchedules::slotResetRegion()
{
  Q_D(KSettingsSchedules);
  slotLoadRegion(d->ui->kcfg_HolidayRegion->text());
}
