/***************************************************************************
                             accounts.h
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

#ifndef ACCOUNTS_H
#define ACCOUNTS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class Accounts; }

/**
  * @author Thomas Baumgart
  */

class Accounts : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(Accounts)

public:
  explicit Accounts(QWidget *parent = nullptr);
  virtual ~Accounts();

  Ui::Accounts *ui;
};


#endif
