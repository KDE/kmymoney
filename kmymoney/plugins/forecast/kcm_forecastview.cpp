/***************************************************************************
 *   Copyright 2018  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com       *
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

#include "kcm_forecastview.h"
#include <config-kmymoney-version.h>

// KDE includes
#include <KPluginFactory>
#include <KAboutData>
#include "forecastviewsettings.h"

ForecastViewSettingsWidget::ForecastViewSettingsWidget(QWidget* parent) :
    QWidget(parent)
{
  setupUi(this);
}

KCMForecastView::KCMForecastView(QWidget *parent, const QVariantList& args)
  : KCModule(parent, args)
{
  ForecastViewSettingsWidget* w = new ForecastViewSettingsWidget(this);
  addConfig(ForecastViewSettings::self(), w);
  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);
  layout->addWidget(w);
  setButtons(NoAdditionalButton);
  load();
}

KCMForecastView::~KCMForecastView()
{
}

K_PLUGIN_FACTORY_WITH_JSON(KCMForecastViewFactory, "kcm_forecastview.json", registerPlugin<KCMForecastView>();)

#include "kcm_forecastview.moc"
