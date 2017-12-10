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

#include <config-kmymoney.h>
#include "models.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "onlinejobmodel.h"
#include "ledgermodel.h"
#include "costcentermodel.h"
#include "payeesmodel.h"
#include "equitiesmodel.h"
#include "securitiesmodel.h"

#ifdef KMM_MODELTEST
  #include "modeltest.h"
#endif

Q_GLOBAL_STATIC(Models, models);

struct Models::Private {
  Private()
  : m_accountsModel(0)
  , m_institutionsModel(0)
  , m_onlineJobModel(0)
  , m_ledgerModel(0)
  , m_costCenterModel(0)
  , m_payeesModel(0)
  , m_equitiesModel(0)
  , m_securitiesModel(0)
  {}

  AccountsModel *m_accountsModel;
  InstitutionsModel *m_institutionsModel;
  onlineJobModel *m_onlineJobModel;
  LedgerModel *m_ledgerModel;
  CostCenterModel *m_costCenterModel;
  PayeesModel *m_payeesModel;
  EquitiesModel *m_equitiesModel;
  SecuritiesModel *m_securitiesModel;
};


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

Models* Models::instance()
{
  return models;
}

/**
  * This is the function to get a reference to the core @ref AccountsModel object.
  * The returned object is owned by this object so don't delete it. It creates the
  * model on the first access to it.
  */
AccountsModel* Models::accountsModel()
{
  if (!d->m_accountsModel) {
    d->m_accountsModel = new AccountsModel(this);
#ifdef KMM_MODELTEST
    new ModelTest(d->m_accountsModel, Models::instance());
#endif
  }
  return d->m_accountsModel;
}

/**
 * This is the function to get a reference to the core @ref InstitutionsModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
InstitutionsModel* Models::institutionsModel()
{
  if (!d->m_institutionsModel) {
    d->m_institutionsModel = new InstitutionsModel(this);
#ifdef KMM_MODELTEST
    new ModelTest(d->m_institutionsModel, Models::instance());
#endif
  }
  return d->m_institutionsModel;
}

onlineJobModel* Models::onlineJobsModel()
{
  if (!d->m_onlineJobModel) {
    d->m_onlineJobModel = new onlineJobModel(this);
#ifdef KMM_MODELTEST
    /// @todo using the ModelTest feature on the onlineJobModel crashes. Need to fix.
    // new ModelTest(d->m_onlineJobModel, Models::instance());
#endif
  }
  return d->m_onlineJobModel;
}

#ifdef ENABLE_UNFINISHEDFEATURES
/**
 * This is the function to get a reference to the core @ref LedgerModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
LedgerModel* Models::ledgerModel()
{
  if (!d->m_ledgerModel) {
    d->m_ledgerModel = new LedgerModel(this);
#ifdef KMM_MODELTEST
    new ModelTest(d->m_ledgerModel, Models::instance());
#endif
  }
  return d->m_ledgerModel;
}
#endif

/**
 * This is the function to get a reference to the core @ref CostCenterModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
CostCenterModel* Models::costCenterModel()
{
  if (!d->m_costCenterModel) {
    d->m_costCenterModel = new CostCenterModel(this);
#ifdef KMM_MODELTEST
    new ModelTest(d->m_costCenterModel, Models::instance());
#endif
  }
  return d->m_costCenterModel;
}

/**
 * This is the function to get a reference to the core @ref PayeesModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
PayeesModel* Models::payeesModel()
{
  if (!d->m_payeesModel) {
    d->m_payeesModel = new PayeesModel(this);
    #ifdef KMM_MODELTEST
    new ModelTest(d->m_payeesModel, Models::instance());
    #endif
  }
  return d->m_payeesModel;
}

/**
 * This is the function to get a reference to the core @ref EquitiesModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
EquitiesModel* Models::equitiesModel()
{
  if (!d->m_equitiesModel) {
    d->m_equitiesModel = new EquitiesModel(this);
    #ifdef KMM_MODELTEST
    new ModelTest(d->m_equitiesModel, Models::instance());
    #endif
  }
  return d->m_equitiesModel;
}

/**
 * This is the function to get a reference to the core @ref SecuritiesModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
SecuritiesModel* Models::securitiesModel()
{
  if (!d->m_securitiesModel) {
    d->m_securitiesModel = new SecuritiesModel(this);
    #ifdef KMM_MODELTEST
    new ModelTest(d->m_securitiesModel, Models::instance());
    #endif
  }
  return d->m_securitiesModel;
}

QModelIndex Models::indexById(QAbstractItemModel* model, int role, const QString& id)
{
  QModelIndexList indexList = model->match(model->index(0, 0),
                                    role,
                                    id,
                                    1,
                                    Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive));

  if(indexList.count() == 1) {
    return indexList.first();
  }
  return QModelIndex();

}

void Models::fileOpened()
{
  accountsModel()->load();
  institutionsModel()->load();
  onlineJobsModel()->load();
  costCenterModel()->load();
  #ifdef ENABLE_UNFINISHEDFEATURES
  ledgerModel()->load();
  #endif
  payeesModel()->load();
  equitiesModel()->load();
  securitiesModel()->load();

  emit modelsLoaded();
}

void Models::fileClosed()
{
  // TODO: make this cleaner in the future, for now just clear the accounts model before the file is closed
  // to avoid any uncaught KMyMoneyExceptions while using the account objects from this model after the file has been closed
  accountsModel()->removeRows(0, accountsModel()->rowCount());
  institutionsModel()->removeRows(0, institutionsModel()->rowCount());
  onlineJobsModel()->unload();
  #ifdef ENABLE_UNFINISHEDFEATURES
  ledgerModel()->unload();
  #endif
  costCenterModel()->unload();
  payeesModel()->unload();
  equitiesModel()->removeRows(0, equitiesModel()->rowCount());
  securitiesModel()->removeRows(0, securitiesModel()->rowCount());
}
