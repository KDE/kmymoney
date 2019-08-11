/*
 * Copyright 2009-2015  Cristian Oneț <onet.cristian@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "models.h"

/// @todo port to new model code
// this file should not be needed anymore
// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#ifdef ENABLE_UNFINISHEDFEATURES
#include "ledgermodel.h"
#endif
/// @todo add new models here
#include "costcentermodel.h"
#include "payeesmodel.h"
#include "schedulesmodel.h"
#include "tagsmodel.h"
#include "securitiesmodel.h"
#include "budgetsmodel.h"
#include "accountsmodel.h"
#include "institutionsmodel.h"
#include "journalmodel.h"
#include "pricemodel.h"
#include "parametersmodel.h"
#include "onlinejobsmodel.h"
#include "reportsmodel.h"

#include "mymoneyfile.h"

#ifdef KMM_MODELTEST
  #include "modeltest.h"
#endif

Q_GLOBAL_STATIC(Models, models);

struct Models::Private {
  Private()
  : m_accountsModel(0)
  , m_institutionsModel(0)
#ifdef ENABLE_UNFINISHEDFEATURES
  , m_ledgerModel(0)
#endif
  // , m_equitiesModel(0)
  // , m_securitiesModel(0)
  {}

  AccountsModel *m_accountsModel;
  InstitutionsModel *m_institutionsModel;
#ifdef ENABLE_UNFINISHEDFEATURES
  LedgerModel *m_ledgerModel;
#endif
  // EquitiesModel *m_equitiesModel;
  // SecuritiesModel *m_securitiesModel;
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
  return MyMoneyFile::instance()->accountsModel();
}

/**
 * This is the function to get a reference to the core @ref InstitutionsModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
InstitutionsModel* Models::institutionsModel()
{
  return MyMoneyFile::instance()->institutionsModel();
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
CostCenterModel* Models::costCenterModel() const
{
  return MyMoneyFile::instance()->costCenterModel();
}

/**
 * This is the function to get a reference to the core @ref PayeesModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
PayeesModel* Models::payeesModel() const
{
  return MyMoneyFile::instance()->payeesModel();
}

/**
 * This is the function to get a reference to the core @ref SchedulesModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
SchedulesModel* Models::schedulesModel() const
{
  return MyMoneyFile::instance()->schedulesModel();
}

/**
 * This is the function to get a reference to the core @ref TagsModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
TagsModel* Models::tagsModel() const
{
  return MyMoneyFile::instance()->tagsModel();
}

/**
 * This is the function to get a reference to the core @ref SecuritiesModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
SecuritiesModel* Models::securitiesModel() const
{
  return MyMoneyFile::instance()->securitiesModel();
}

/**
 * This is the function to get a reference to the core @ref CurrenciesModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
SecuritiesModel* Models::currenciesModel() const
{
  return MyMoneyFile::instance()->currenciesModel();
}

/**
 * This is the function to get a reference to the core @ref BudgetsModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
BudgetsModel* Models::budgetsModel() const
{
  return MyMoneyFile::instance()->budgetsModel();
}

/**
 * This is the function to get a reference to the core @ref JournalModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
JournalModel* Models::journalModel() const
{
  return MyMoneyFile::instance()->journalModel();
}

/**
 * This is the function to get a reference to the core @ref PriceModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
PriceModel* Models::priceModel() const
{
  return MyMoneyFile::instance()->priceModel();
}

/**
 * This is the function to get a reference to the core @ref ParametersModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
ParametersModel* Models::parametersModel() const
{
  return MyMoneyFile::instance()->parametersModel();
}

/**
 * This is the function to get a reference to the core @ref OnlineJobsModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
OnlineJobsModel* Models::onlineJobsModel() const
{
  return MyMoneyFile::instance()->onlineJobsModel();
}

/**
 * This is the function to get a reference to the core @ref ReportsModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
ReportsModel* Models::reportsModel() const
{
  return MyMoneyFile::instance()->reportsModel();
}




/// @todo add new models here

#if 0
/**
 * This is the function to get a reference to the core @ref EquitiesModel.
 * The returned object is owned by this object so don't delete it. It creates the
 * model on the first access to it.
 */
EquitiesModel* Models::equitiesModel()
{
  return nullptr;
#if 0
  if (!d->m_equitiesModel) {
    d->m_equitiesModel = new EquitiesModel(this);
    #ifdef KMM_MODELTEST
    new ModelTest(d->m_equitiesModel, Models::instance());
    #endif
  }
  return d->m_equitiesModel;
#endif
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
#endif

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

void Models::fileSaved()
{
  /// @todo add new models here
  schedulesModel()->setDirty(false);
  costCenterModel()->setDirty(false);
  payeesModel()->setDirty(false);
  tagsModel()->setDirty(false);
  securitiesModel()->setDirty(false);
  currenciesModel()->setDirty(false);
  budgetsModel()->setDirty(false);
  accountsModel()->setDirty(false);
  institutionsModel()->setDirty(false);
  journalModel()->setDirty(false);
  priceModel()->setDirty(false);
  parametersModel()->setDirty(false);
  onlineJobsModel()->setDirty(false);
  reportsModel()->setDirty(false);
}

void Models::fileOpened()
{
  /// @todo cleanup
  // accountsModel()->AccountsModel::load();
  // institutionsModel()->InstitutionsModel::load();
  // securitiesModel()->load();
  #ifdef ENABLE_UNFINISHEDFEATURES
  // ledgerModel()->load();
  #endif
  // equitiesModel()->load();

  emit modelsLoaded();
}

void Models::unload()
{
  // TODO: make this cleaner in the future, for now just clear the accounts model before the file is closed
  // to avoid any uncaught KMyMoneyExceptions while using the account objects from this model after the file has been closed
  accountsModel()->removeRows(0, accountsModel()->rowCount());
  institutionsModel()->removeRows(0, institutionsModel()->rowCount());
  #ifdef ENABLE_UNFINISHEDFEATURES
  ledgerModel()->unload();
  #endif
  // equitiesModel()->removeRows(0, equitiesModel()->rowCount());
  // securitiesModel()->removeRows(0, securitiesModel()->rowCount());

  MyMoneyFile::instance()->unload();
}
