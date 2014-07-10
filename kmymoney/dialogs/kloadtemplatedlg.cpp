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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kstandardguiitem.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccounttemplateselector.h"
#include <QList>

KLoadTemplateDlg::KLoadTemplateDlg(QWidget* parent) :
    KLoadTemplateDlgDecl(parent)
{
  buttonOk->setGuiItem(KStandardGuiItem::ok());
  buttonCancel->setGuiItem(KStandardGuiItem::cancel());
  buttonHelp->setGuiItem(KStandardGuiItem::help());

  connect(buttonHelp, SIGNAL(clicked()), this, SLOT(slotHelp()));
}

QList<MyMoneyTemplate> KLoadTemplateDlg::templates(void) const
{
  return m_templateSelector->selectedTemplates();
}

void KLoadTemplateDlg::slotHelp(void)
{
}
