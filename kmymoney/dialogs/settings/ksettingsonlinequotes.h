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

#ifndef KSETTINGSONLINEQUOTES_H
#define KSETTINGSONLINEQUOTES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsonlinequotesdecl.h"
#include "kmymoney/converter/webpricequote.h"


class KSettingsOnlineQuotesDecl : public QWidget, public Ui::KSettingsOnlineQuotesDecl
{
public:
  KSettingsOnlineQuotesDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class KSettingsOnlineQuotes : public KSettingsOnlineQuotesDecl
{
  Q_OBJECT
public:
  KSettingsOnlineQuotes(QWidget* parent = 0);
  virtual ~KSettingsOnlineQuotes() {}

  void writeConfig(void) {}
  void readConfig(void) {}
  void resetConfig(void);

protected slots:
  void slotUpdateEntry(void);
  void slotLoadWidgets();
  void slotEntryChanged(void);
  void slotNewEntry(void);
  void slotEntryRenamed(QListWidgetItem* item);
  void slotStartRename(QListWidgetItem* item);

protected:
  void loadList(const bool updateResetList = false);

private:
  QList<WebPriceQuoteSource>  m_resetList;
  WebPriceQuoteSource         m_currentItem;
  bool                        m_quoteInEditing;
};

#endif
