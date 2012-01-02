/***************************************************************************
                          kpayeesview.cpp
                          ---------------
    begin                : Tue Mar 02 2010
    copyright            : (C) 2010 by Alvaro Soliverez <asoliverez@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "payeesmodel.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneyfile.h"

class PayeesModel::Private
{
public:
  /**
    * The pimpl.
    */
  Private() :
      m_file(MyMoneyFile::instance()) {
  }

  ~Private() {
  }

  /**
    * Note: this functions should only be called after the child account data has been set.
    */
  void setPayeeData(QStandardItemModel *model, const QModelIndex &index, const MyMoneyPayee &payee) {
    // Account
    model->setData(index, payee.name(), Qt::DisplayRole);
    model->setData(index, QVariant::fromValue(payee), PayeeRole);
    model->setData(index, QVariant(payee.id()), PayeeIdRole);

    //Set the data in the model

    model->setData(index, QVariant(payee.address()), PayeeAdressRole);
    model->setData(index, QVariant(payee.postcode()), PayeePostCodeRole);
    model->setData(index, QVariant(payee.telephone()), PayeeTelephoneRole);
    model->setData(index, QVariant(payee.email()), PayeeEmailRole);
    model->setData(index, QVariant(payee.notes()), PayeeNotesRole);

    QStringList keys;
    bool ignorecase = false;
    MyMoneyPayee::payeeMatchType type = payee.matchData(ignorecase, keys);
    model->setData(index, QVariant(keys), PayeeMatchKeyListRole);
    model->setData(index, QVariant(type), PayeeMatchRole);
    model->setData(index, QVariant(ignorecase), PayeeMatchIgnoreCaseRole);
    model->setData(index, QVariant(payee.defaultAccountId()), PayeeDefaultAccountRole);
    model->setData(index, QVariant(payee.name()), DisplayOrderRole);

    // this payee still exists so it should not be cleaned up
    model->setData(index, false, CleanupRole);

  }

  /**
    * Function to get the item from a payee id.
    *
    * @param parent The parent to localize the seach in the child items of this parameter.
    * @param objectId Search will be based on this parameter.
    *
    * @return The item corresponding to the given account id, NULL if the account was not found.
    */
  QStandardItem *itemFromObjectId(QStandardItem *parent, const QString &objectId) {
    QStandardItemModel *model = parent->model();
    QModelIndexList list = model->match(model->index(0, 0, parent->index()), PayeeIdRole, QVariant(objectId), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));
    if (list.count() > 0) {
      return model->itemFromIndex(list.front());
    }
    // TODO: if not found at this item search for in int the model and if found reparent it.
    return 0;
  }

  /**
    * Used to load the accounts data.
    */
  MyMoneyFile *m_file;

};



/**
  * The constructor is private so that only the @ref Models object can create such an object.
  */
PayeesModel::PayeesModel(QObject *parent /*= 0*/)
    : QStandardItemModel(parent), d(new Private)
{
  QStringList headerLabels;
  for (int i = 0; i < LastColumnMarker; ++i) {
    switch (i) {
      case Payee:
        headerLabels << i18n("Payee");
        break;
      default:
        headerLabels << QString();
    }
  }
  setHorizontalHeaderLabels(headerLabels);
}

PayeesModel::~PayeesModel()
{
  delete d;
}

/**
  * This function synchronizes the data of the model with the data
  * from the @ref MyMoneyFile.
  */
void PayeesModel::load()
{
  // mark all rows as candidates for cleaning up
  QModelIndexList list = match(index(0, 0), Qt::DisplayRole, "*", -1, Qt::MatchFlags(Qt::MatchWildcard | Qt::MatchRecursive));
  foreach (const QModelIndex &index, list) {
    setData(PayeesModel::index(index.row(), 0, index.parent()), true, CleanupRole);
  }

  QList<MyMoneyPayee> payeelist = d->m_file->payeeList();

  for (QList<MyMoneyPayee>::ConstIterator it_l = payeelist.constBegin(); it_l != payeelist.constEnd(); ++it_l) {
    QStandardItem* item = new QStandardItem((*it_l).name());
    appendRow(item);
    item->setColumnCount(columnCount());
    item->setEditable(false);
    d->setPayeeData(this, item->index(), *it_l);
  }
  QStandardItem* blankItem = new QStandardItem(QString());
  blankItem->setData(QVariant(QString()), PayeeIdRole);
  appendRow(blankItem);

}

/**
  * The pimpl.
  */
class PayeesComboProxyModel::Private
{
public:
  Private() :
      m_allowEmptyPayee(false) {
    m_payeeList = QStringList();
  }

  ~Private() {
  }

  QStringList m_payeeList;
  bool m_allowEmptyPayee;
};

PayeesComboProxyModel::PayeesComboProxyModel(QObject *parent /*= 0*/)
    : QSortFilterProxyModel(parent), d(new Private)
{
  setDynamicSortFilter(true);
  setFilterKeyColumn(-1);
  setSortLocaleAware(true);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

/**
  * This function was re-implemented so we could have a special display order (favorites first)
  */
bool PayeesComboProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  QVariant leftData = sourceModel()->data(left, PayeesModel::DisplayOrderRole);
  QVariant rightData = sourceModel()->data(right, PayeesModel::DisplayOrderRole);

  if (leftData.toInt() == rightData.toInt()) {
    // sort items of the same display order alphabetically
    return QSortFilterProxyModel::lessThan(left, right);
  }
  return leftData.toInt() < rightData.toInt();
}

/**
  * This function was re-implemented to consider all the filtering aspects that we need in the application.
  */
bool PayeesComboProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
  return acceptSourceItem(index);
}

/**
  * Use this to reset the filter.
  */
void PayeesComboProxyModel::clear(void)
{
  invalidateFilter();
}

/**
  * Implementation function that performs the actual filtering.
  */
bool PayeesComboProxyModel::acceptSourceItem(const QModelIndex &source) const
{
  if (source.isValid()) {
    QVariant data = sourceModel()->data(source, PayeesModel::PayeeIdRole);
    if (data.isValid()) {

      //if the payee list is empty, all items are allowed
      if (d->m_payeeList.isEmpty())
        return true;

      QString id = data.toString();

      //check whether empty payees should be allowed
      if (id.isEmpty()) {
        if (d->m_allowEmptyPayee) {
          return true;
        } else  {
          return false;
        }
      }

      if (d->m_payeeList.contains(id, Qt::CaseSensitive))
        return true;
    }

    int rowCount = sourceModel()->rowCount(source);
    for (int i = 0; i < rowCount; ++i) {
      QModelIndex index = sourceModel()->index(i, 0, source);
      if (acceptSourceItem(index))
        return true;
    }
  }
  return false;
}

void PayeesComboProxyModel::setFilterList(const bool &allowEmpty, const QList<MyMoneyPayee> &payeeList)
{
  QStringList idList;

  QList<MyMoneyPayee>::const_iterator payeeIt = payeeList.constBegin();
  while (payeeIt != payeeList.constEnd()) {
    idList.append((*payeeIt).id());
    ++payeeIt;
  }
  d->m_payeeList = idList;
  d->m_allowEmptyPayee = allowEmpty;
}


#include "payeesmodel.moc"
