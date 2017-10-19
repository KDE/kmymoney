/***************************************************************************
                             currency.h
                             -------------------
    begin                : Fri Jun  1 2007
    copyright            : (C) 2007 Thomas Baumgart
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

#ifndef CURRENCY_H
#define CURRENCY_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_currencydecl.h"
#include "mymoneysecurity.h"

/**
  * @author Thomas Baumgart
  */

class CurrencyDecl : public QWidget, public Ui::CurrencyDecl
{
public:
  CurrencyDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};
class Currency : public CurrencyDecl
{
  Q_OBJECT
public:
  Currency(QWidget* parent = 0);
  QTreeWidgetItem* insertCurrency(const MyMoneySecurity& sec);
  void selectCurrency(const MyMoneySecurity& sec);
  QString selectedCurrency() const;
};

#endif
