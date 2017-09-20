/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif

#include <webaccount.h>
#include "mymoneykeyvaluecontainer.h"
#include <ui_webaccount.h>


struct WebAccountSettings::Private {
  Ui::WebAccountSettings ui;
};

WebAccountSettings::WebAccountSettings(const MyMoneyAccount& /*acc*/,
                                       QWidget* parent) :
    QWidget(parent),
    d(new Private)
{
  d->ui.setupUi(this);
}

WebAccountSettings::~WebAccountSettings()
{
  delete d;
}

void WebAccountSettings::loadUi(const MyMoneyKeyValueContainer& kvp)
{
  d->ui.id->setText(kvp.value("wb-id"));
  d->ui.backend->setText(kvp.value("wb-backend"));
  d->ui.max_history->setText(kvp.value("wb-max"));
}

void WebAccountSettings::loadKvp(MyMoneyKeyValueContainer& kvp)
{
  kvp.setValue("wb-id", d->ui.id->text());
  kvp.setValue("wb-backend", d->ui.backend->text());
  kvp.setValue("wb-max", d->ui.max_history->text());
}
