/***************************************************************************
                          kloadtemplatedlg.cpp
                             -------------------
    copyright            : (C) 2008 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include <QPushButton>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>
#include <KStandardGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccounttemplateselector.h"

KLoadTemplateDlg::KLoadTemplateDlg(QWidget* parent) :
    KLoadTemplateDlgDecl(parent)
{
  KGuiItem::assign(buttonOk, KStandardGuiItem::ok());
  KGuiItem::assign(buttonCancel, KStandardGuiItem::cancel());
  KGuiItem::assign(buttonHelp, KStandardGuiItem::help());

  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
}

QList<MyMoneyTemplate> KLoadTemplateDlg::templates() const
{
  return m_templateSelector->selectedTemplates();
}

void KLoadTemplateDlg::slotHelp()
{
}
