/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACCOUNTS_H
#define ACCOUNTS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class Accounts;
}

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
