/***************************************************************************
                     ktemplateexportlg.cpp
                     ---------------------
    copyright        : (C) 2016 by Ralf Habacker <ralf.habacker@freenet.de>
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

#include "ktemplateexportdlg.h"
#include "ui_ktemplateexportdlg.h"

KTemplateExportDlg::KTemplateExportDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KTemplateExportDlg)
{
    ui->setupUi(this);
}

KTemplateExportDlg::~KTemplateExportDlg()
{
    delete ui;
}

QString KTemplateExportDlg::title() const
{
    return ui->m_title->text();
}

QString KTemplateExportDlg::shortDescription() const
{
    return ui->m_shortDescription->text();
}

QString KTemplateExportDlg::longDescription() const
{
    return ui->m_longDescription->document()->toPlainText();
}
