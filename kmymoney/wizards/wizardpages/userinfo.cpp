/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "userinfo.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_userinfo.h"
#include "mymoneypayee.h"

UserInfo::UserInfo(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::UserInfo)
{
    ui->setupUi(this);
    ui->m_userNameEdit->setFocus();
}

UserInfo::~UserInfo()
{
    delete ui;
}

MyMoneyPayee UserInfo::user() const
{
    MyMoneyPayee user;
    user.setName(ui->m_userNameEdit->text());
    user.setAddress(ui->m_streetEdit->text());
    user.setCity(ui->m_townEdit->text());
    user.setState(ui->m_countyEdit->text());
    user.setPostcode(ui->m_postcodeEdit->text());
    user.setTelephone(ui->m_telephoneEdit->text());
    user.setEmail(ui->m_emailEdit->text());
    return user;
}
