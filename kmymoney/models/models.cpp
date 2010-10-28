/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#include "models.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"

struct Models::Private {
  Private() :
    m_accountsModel(0),
    m_loadOnIdle(false)
  {}

  AccountsModel *m_accountsModel;
  bool m_loadOnIdle;
};

Models Models::models;

/**
  * This object is a singleton so it will be created very early in the application's life
  * so don't do anything on this constructor that relies on application data (even a i18n call).
  */
Models::Models() : QObject(), d(new Private)
{
}

Models::~Models()
{
  delete d;
}

/**
  * This is the function to get a reference to the core @ref AccountsModel object.
  * The returned object is owned by this object so don't delete it. It creates the
  * model on the first access to it.
  */
AccountsModel* Models::accountsModel()
{
  if (!d->m_accountsModel)
    d->m_accountsModel = new AccountsModel(this);
  return d->m_accountsModel;
}

void Models::dataChanged(void)
{
  if (d->m_loadOnIdle)
  {
    // load the accounts model on idle
    QTimer::singleShot(0, accountsModel(), SLOT(load()));
  } else
  {
    // make sure we load the accounts model when the event loop is idle only after we loaded some real accounts
    d->m_loadOnIdle = accountsModel()->load();
  }
}

void Models::fileClosed(void)
{
  // TODO: make this cleaner in the future, for now just clear the accounts model before the file is closed
  // to avoid any uncaught KMyMoneyExceptions while using the account objects from this model after the file has been closed
  accountsModel()->removeRows(0, accountsModel()->rowCount());
  // make sure that the accounts model will be loaded immediately on the next dataChanged() signal
  d->m_loadOnIdle = false;
}
