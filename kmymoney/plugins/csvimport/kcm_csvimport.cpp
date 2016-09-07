/***************************************************************************
 *   Copyright 2016  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com       *
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

#include "kcm_csvimport.h"

// KDE includes
#include <KPluginFactory>
#include "pluginsettings.h"

PluginSettingsWidget::PluginSettingsWidget(QWidget* parent) :
    QWidget(parent)
{
  setupUi(this);
}

K_PLUGIN_FACTORY_WITH_JSON(KCMcsvimportFactory, "kcm_kmm_csvimport.json", registerPlugin<KCMcsvimport>();)

// TODO: port to KF5
KCMcsvimport::KCMcsvimport(QWidget *parent, const QVariantList& args) : KCModule(0/*KCMcsvimportFactory::componentData()*/, parent, args)
{
  PluginSettingsWidget* w = new PluginSettingsWidget(this);
  addConfig(PluginSettings::self(), w);
  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);
  layout->addWidget(w);
  setButtons(NoAdditionalButton);
  load();
}

KCMcsvimport::~KCMcsvimport()
{
}

#include "kcm_csvimport.moc"
