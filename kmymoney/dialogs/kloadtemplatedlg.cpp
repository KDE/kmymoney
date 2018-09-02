/*
 * Copyright 2008       Thomas Baumgart <tbaumgart@kde.org>
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

#include "kloadtemplatedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kloadtemplatedlg.h"

#include "mymoneytemplate.h"
#include "kaccounttemplateselector.h"

KLoadTemplateDlg::KLoadTemplateDlg(QWidget* parent) :
  QDialog(parent),
  ui(new Ui::KLoadTemplateDlg)
{
  ui->setupUi(this);
  connect(ui->buttonBox, &QDialogButtonBox::helpRequested, this, &KLoadTemplateDlg::slotHelp);
}

KLoadTemplateDlg::~KLoadTemplateDlg()
{
  delete ui;
}

QList<MyMoneyTemplate> KLoadTemplateDlg::templates() const
{
  return ui->m_templateSelector->selectedTemplates();
}

void KLoadTemplateDlg::slotHelp()
{
}
