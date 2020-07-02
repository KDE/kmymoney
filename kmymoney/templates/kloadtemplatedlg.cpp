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
#include "templateloader.h"

class KLoadTemplateDlgPrivate
{
public:
  KLoadTemplateDlgPrivate()
  : ui(new Ui::KLoadTemplateDlg)
  {}

  Ui::KLoadTemplateDlg* ui;
  TemplatesModel        model;
  TemplateLoader        loader;
};

KLoadTemplateDlg::KLoadTemplateDlg(QWidget* parent) :
  QDialog(parent),
  d_ptr(new KLoadTemplateDlgPrivate)
{
  Q_D(KLoadTemplateDlg);
  d->ui->setupUi(this);
  connect(d->ui->buttonBox, &QDialogButtonBox::helpRequested, this, &KLoadTemplateDlg::slotHelp);

  d->loader.load(&d->model);
  d->ui->m_templateSelector->setModel(&d->model);

  connect(&d->loader, &TemplateLoader::loadingFinished, d->ui->m_templateSelector, &KAccountTemplateSelector::setupInitialSelection);
}

KLoadTemplateDlg::~KLoadTemplateDlg()
{
  Q_D(KLoadTemplateDlg);
  delete d;
}

QList<MyMoneyTemplate> KLoadTemplateDlg::templates() const
{
  Q_D(const KLoadTemplateDlg);
  return d->ui->m_templateSelector->selectedTemplates();
}

void KLoadTemplateDlg::slotHelp()
{
}
