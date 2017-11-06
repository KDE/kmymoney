/***************************************************************************
                          kloadtemplatedlg.cpp
                             -------------------
    copyright            : (C) 2008 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
