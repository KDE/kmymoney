/*
 * SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "onlinejobmessagesview.h"

#include "ui_onlinejobmessagesview.h"

onlineJobMessagesView::onlineJobMessagesView(QWidget* parent)
    : QWidget(parent),
    ui(new Ui::onlineJobMessageView)
{
  ui->setupUi(this);
  connect(ui->closeButton, &QAbstractButton::pressed, this, &QWidget::close);
}

void onlineJobMessagesView::setModel(QAbstractItemModel* model)
{
  ui->tableView->setModel(model);
  // Enlarge the description column
  ui->tableView->setColumnWidth(2, 4*ui->tableView->columnWidth(2));
}

onlineJobMessagesView::~onlineJobMessagesView()
{
  delete ui;
}
