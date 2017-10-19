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

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccounttemplateselector.h"
#include "mymoneytemplate.h"

KLoadTemplateDlg::KLoadTemplateDlg(QWidget* parent) :
    KLoadTemplateDlgDecl(parent)
{
  connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(slotHelp()));
}

QList<MyMoneyTemplate> KLoadTemplateDlg::templates() const
{
  return m_templateSelector->selectedTemplates();
}

void KLoadTemplateDlg::slotHelp()
{
}
