/*
 * SPDX-FileCopyrightText: 2017 Ralf Habacker <ralf.habacker@freenet.de>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
