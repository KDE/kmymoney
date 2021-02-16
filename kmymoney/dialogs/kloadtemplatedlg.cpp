/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
