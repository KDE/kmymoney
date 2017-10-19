/***************************************************************************
                          ktagreassigndlg.cpp
                             -------------------
    copyright            : (C) 2011 by Alessandro Russo <axela74@yahoo.it>

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

#include <kmymoneymvccombo.h>

KTagReassignDlg::KTagReassignDlg(QWidget* parent) :
    KTagReassignDlgDecl(parent)
{
  kMandatoryFieldGroup* mandatory = new kMandatoryFieldGroup(this);
  mandatory->add(tagCombo);
  mandatory->setOkButton(buttonBox->button(QDialogButtonBox::Ok));
}

KTagReassignDlg::~KTagReassignDlg()
{
}

QString KTagReassignDlg::show(const QList<MyMoneyTag>& tagslist)
{
  if (tagslist.isEmpty())
    return QString(); // no tag available? nothing can be selected...

  tagCombo->loadTags(tagslist);

  // execute dialog and if aborted, return empty string
  if (this->exec() == QDialog::Rejected)
    return QString();

  // otherwise return index of selected tag
  return tagCombo->selectedItem();
}


void KTagReassignDlg::accept()
{
  // force update of tagCombo
  buttonBox->button(QDialogButtonBox::Ok)->setFocus();

  if (tagCombo->selectedItem().isEmpty()) {
    KMessageBox::information(this, i18n("This dialog does not allow new tags to be created. Please pick a tag from the list."), i18n("Tag creation"));
  } else {
    KTagReassignDlgDecl::accept();
  }
}
