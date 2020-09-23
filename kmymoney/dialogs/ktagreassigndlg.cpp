/*
 * Copyright 2011-2012  Alessandro Russo <axela74@yahoo.it>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
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

#include "ktagreassigndlg.h"
#include "ui_ktagreassigndlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "tagsmodel.h"
#include "idfilter.h"

KTagReassignDlg::KTagReassignDlg(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::KTagReassignDlg)
{
  ui->setupUi(this);
}

KTagReassignDlg::~KTagReassignDlg()
{
  delete ui;
}

QString KTagReassignDlg::show(const QList<QString>& tagslist)
{
  auto filter = new IdFilter(this);
  filter->setFilterList(tagslist);
  filter->setSourceModel(MyMoneyFile::instance()->tagsModel());
  filter->setSortLocaleAware(true);
  filter->sort(0);
  ui->tagCombo->setModel(filter);

  // execute dialog and if aborted, return empty string
  if (this->exec() == QDialog::Rejected)
    return QString();

  // otherwise return id of selected tag
  const auto idx = filter->index(ui->tagCombo->currentIndex(), 0);
  return idx.data(eMyMoney::Model::IdRole).toString();
}
