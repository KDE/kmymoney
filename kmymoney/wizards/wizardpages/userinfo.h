/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef USERINFO_H
#define USERINFO_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class UserInfo; }

class MyMoneyPayee;

/**
  * @author Thomas Baumgart
  */

class UserInfo : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(UserInfo)

public:
  explicit UserInfo(QWidget *parent = nullptr);
  virtual ~UserInfo();

  MyMoneyPayee user() const;
  Ui::UserInfo *ui;
};


#endif
