/***************************************************************************
                          ktagreassigndlg.cpp
                             -------------------
    copyright            : (C) 2011 by Alessandro Russo <axela74@yahoo.it>
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

#include "ktagreassigndlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <kguiutils.h>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ktagreassigndlg.h"
#include <kmymoneymvccombo.h>

KTagReassignDlg::KTagReassignDlg(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::KTagReassignDlg)
{
  ui->setupUi(this);
  auto mandatory = new KMandatoryFieldGroup(this);
  mandatory->add(ui->tagCombo);
  mandatory->setOkButton(ui->buttonBox->button(QDialogButtonBox::Ok));
}

KTagReassignDlg::~KTagReassignDlg()
{
  delete ui;
}

QString KTagReassignDlg::show(const QList<MyMoneyTag>& tagslist)
{
  if (tagslist.isEmpty())
    return QString(); // no tag available? nothing can be selected...

  ui->tagCombo->loadTags(tagslist);

  // execute dialog and if aborted, return empty string
  if (this->exec() == QDialog::Rejected)
    return QString();

  // otherwise return index of selected tag
  return ui->tagCombo->selectedItem();
}

void KTagReassignDlg::accept()
{
  // force update of ui->tagCombo
  ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();

  if (ui->tagCombo->selectedItem().isEmpty()) {
    KMessageBox::information(this, i18n("This dialog does not allow new tags to be created. Please pick a tag from the list."), i18n("Tag creation"));
  } else {
    QDialog::accept();
  }
}
