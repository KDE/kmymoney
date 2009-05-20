/***************************************************************************
                          kmymoneyonlinequoteconfig.h  -  description
                             -------------------
    begin                : Thu Dec 30 2004
    copyright            : (C) 2004 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYONLINEQUOTECONFIG_H
#define KMYMONEYONLINEQUOTECONFIG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kmymoneyonlinequoteconfigdecl.h"
#include "../converter/webpricequote.h"
//Added by qt3to4:
#include <Q3ValueList>


class kMyMoneyOnlineQuoteConfigDecl : public QWidget, public Ui::kMyMoneyOnlineQuoteConfigDecl
{
public:
  kMyMoneyOnlineQuoteConfigDecl( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};
class kMyMoneyOnlineQuoteConfig : public kMyMoneyOnlineQuoteConfigDecl
{
  Q_OBJECT
public:
  kMyMoneyOnlineQuoteConfig(QWidget* parent);
  virtual ~kMyMoneyOnlineQuoteConfig() {}

  void writeConfig(void) {}
  void readConfig(void) {}
  void resetConfig(void);

protected slots:
  void slotUpdateEntry(void);
  void slotLoadWidgets(Q3ListViewItem* item);
  void slotEntryChanged(void);
  void slotNewEntry(void);
  void slotEntryRenamed(Q3ListViewItem* item, const QString& text, int col);

protected:
  void loadList(const bool updateResetList = false);

private:
  Q3ValueList<WebPriceQuoteSource>  m_resetList;
  WebPriceQuoteSource              m_currentItem;
};

#endif
