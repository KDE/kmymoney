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

class QTreeWidgetItem;
class MyMoneySecurity;

namespace Ui { class Currency; }

/**
  * @author Thomas Baumgart
  */

class Currency : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(Currency)

public:
  explicit Currency(QWidget *parent = nullptr);
  virtual ~Currency();

  QTreeWidgetItem* insertCurrency(const MyMoneySecurity& sec);
  void selectCurrency(const MyMoneySecurity& sec);
  QString selectedCurrency() const;

  Ui::Currency *ui;
};

#endif
