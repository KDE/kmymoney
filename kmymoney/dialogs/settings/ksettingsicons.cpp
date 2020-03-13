/*
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

#include "ksettingsicons.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>
#include <QDir>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsicons.h"

class KSettingsIconsPrivate
{
  Q_DISABLE_COPY(KSettingsIconsPrivate)

public:
  KSettingsIconsPrivate() :
    ui(new Ui::KSettingsIcons)
  {
  }

  ~KSettingsIconsPrivate()
  {
    delete ui;
  }

  Ui::KSettingsIcons *ui;
  QMap<int, QString>  m_themesMap;
};

KSettingsIcons::KSettingsIcons(QWidget* parent) :
  QWidget(parent),
  d_ptr(new KSettingsIconsPrivate)
{
  Q_D(KSettingsIcons);
  d->ui->setupUi(this);
  // hide the internally used holidayRegion field
  d->ui->kcfg_IconsTheme->hide();

  loadList();

  // setup connections so that region gets selected once field is filled
  connect(d->ui->kcfg_IconsTheme, &QLineEdit::textChanged, this, &KSettingsIcons::slotLoadTheme);

  // setup connections so that changes are forwarded to the field
  connect(d->ui->m_IconsTheme,
          static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &KSettingsIcons::slotSetTheme);
}

KSettingsIcons::~KSettingsIcons()
{
  Q_D(KSettingsIcons);
  delete d;
}

void KSettingsIcons::loadList()
{
  Q_D(KSettingsIcons);
  QStringList themes {QStringLiteral("oxygen"), QStringLiteral("Tango"), QStringLiteral("breeze"), QStringLiteral("breeze-dark")};
  QStringList searchPaths = QIcon::themeSearchPaths();
  d->ui->m_IconsTheme->addItem(QStringLiteral("system"));
  d->m_themesMap.insert(0, QStringLiteral("system"));
  for (auto i = 0; i < searchPaths.count(); ++i) {
    for (int j = 0; j < themes.count(); ++j) {
      QDir themeDir = QDir(searchPaths.at(i)).filePath(themes.at(j));
      if (themeDir.exists(QStringLiteral("index.theme"))) {
        d->ui->m_IconsTheme->addItem(themes.at(j));
        d->m_themesMap.insert(d->m_themesMap.count(), themes.at(j));
      }
    }
  }
}

void KSettingsIcons::slotSetTheme(const int &theme)
{
  Q_D(KSettingsIcons);
  d->ui->kcfg_IconsTheme->setText(d->m_themesMap.value(theme));
}

void KSettingsIcons::slotLoadTheme(const QString &theme)
{
  Q_D(KSettingsIcons);
  // only need this once
  disconnect(d->ui->kcfg_IconsTheme, &QLineEdit::textChanged, this, &KSettingsIcons::slotLoadTheme);
  auto i = 0;
  if (!theme.isEmpty())
    i = d->ui->m_IconsTheme->findText(theme);
  if ((i > -1) && (i != d->ui->m_IconsTheme->currentIndex())) {
    QSignalBlocker blocked(d->ui->m_IconsTheme);
    d->ui->m_IconsTheme->setCurrentIndex(i);
  }
}

void KSettingsIcons::slotResetTheme()
{
  Q_D(KSettingsIcons);
  slotLoadTheme(d->ui->kcfg_IconsTheme->text());
}
