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

#include "kcm_printcheck.h"

// Qt includes
#include <qcombobox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qframe.h>

// KDE includes
#include <khtml_part.h>
#include <khtmlview.h>
#include <kgenericfactory.h>
#include <kurlrequester.h>

#include "pluginsettings.h"
#include "ui_pluginsettingsdecl.h"

class PluginSettingsWidget : public QWidget, public Ui::PluginSettingsDecl
{
public:
  PluginSettingsWidget( QWidget* parent = 0 ) : QWidget(parent)
  {
    setupUi(this);
    m_checkTemplatePreviewHTMLPart = new KHTMLPart(m_previewFrame);
    QVBoxLayout *layout = new QVBoxLayout;
    m_previewFrame->setLayout(layout);
    layout->addWidget(m_checkTemplatePreviewHTMLPart->view());
  }

public slots:
  virtual void urlSelected(const KUrl& url)
  {
    m_checkTemplatePreviewHTMLPart->openUrl(url);
  }

  virtual void returnPressed(const QString& url)
  {
    m_checkTemplatePreviewHTMLPart->openUrl(url);
  }

private:
  KHTMLPart* m_checkTemplatePreviewHTMLPart;
};

K_PLUGIN_FACTORY(KCMPrintCheckFactory,
                 registerPlugin<KCMPrintCheck>();
                )
K_EXPORT_PLUGIN(KCMPrintCheckFactory( "kmm_printcheck", "kmymoney" ) )

KCMPrintCheck::KCMPrintCheck(QWidget *parent, const QVariantList& args) : KCModule(KCMPrintCheckFactory::componentData(), parent, args)
{
  PluginSettingsWidget* w = new PluginSettingsWidget(this);
  addConfig(PluginSettings::self(), w);
  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);
  layout->addWidget(w);
  load();
  w->urlSelected(PluginSettings::checkTemplateFile());
}

KCMPrintCheck::~KCMPrintCheck()
{
}
