/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Alvaro Soliverez <asoliverez@gmail.com>
    (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reportcontrolimpl.h"
#include "ui_reportcontrol.h"

ReportControl::ReportControl(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ReportControl)
{
  ui->setupUi(this);
}

ReportControl::~ReportControl()
{
  delete ui;
}


