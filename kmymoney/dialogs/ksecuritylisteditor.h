/***************************************************************************
                          ksecuritylisteditor.h  -  description
                             -------------------
    begin                : Wed Dec 16 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

#ifndef KSECURITYLISTEDITOR_H
#define KSECURITYLISTEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes



// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksecuritylisteditordecl.h"

#include "mymoneysecurity.h"

/**
  * @author Thomas Baumgart
  */

class KSecurityListEditorDecl : public KDialog, public Ui::KSecurityListEditorDecl
{
public:
  KSecurityListEditorDecl(QWidget *parent) : KDialog(parent) {
    setupUi(this);
  }
};

enum eSecurityColum { eIdColumn, eTypeColumn, eNameColumn, eSymbolColumn, eMarketColumn, eCurrencyColumn, eAcctFractionColumn, eCashFractionColumn };

class KSecurityListEditor : public KSecurityListEditorDecl
{
  Q_OBJECT
public:
  KSecurityListEditor(QWidget *parent);
  ~KSecurityListEditor();

protected slots:
  void slotLoadList(void);
  void slotUpdateButtons(void);
  void slotEditSecurity(void);
  void slotDeleteSecurity(void);

protected:
  void fillItem(QTreeWidgetItem* item, const MyMoneySecurity& security);

private:
  const QString m_currencyMarket;

};

#endif
