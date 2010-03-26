/***************************************************************************
                          kpayeesmodel.h
                          -------------
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

#ifndef PAYEESMODEL_H
#define PAYEESMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneypayee.h"

/**
  * A model for the payees
  * This model loads all the payees from the @ref MyMoneyFile.
  * This object should be kept sychronized
  * with the data in the @ref MyMoneyFile (this is accomplished
  * by the @ref Models object).
  *
  * @see MyMoneyPayee
  * @see MyMoneyFile
  *
  * @author Cristian Onet 2010
  *
  */
class PayeesModel : public QStandardItemModel
{
  Q_OBJECT

public:

  /**
    * User roles used by this model.
    */
  enum PayeesItemDataRole {
    PayeeIdRole = Qt::UserRole,                     /**< The account id is stored in this role in column 0 as a string.*/
    PayeeRole = Qt::UserRole + 1,                   /**< The MyMoneyAccount is stored in this role in column 0.*/
    PayeeAdressRole = Qt::UserRole + 2,
    PayeePostCodeRole = Qt::UserRole + 3,
    PayeeTelephoneRole = Qt::UserRole + 4,
    PayeeEmailRole = Qt::UserRole + 5,
    PayeeNotesRole = Qt::UserRole + 6,
    PayeeMatchKeyListRole = Qt::UserRole + 7,
    PayeeMatchRole = Qt::UserRole + 8,
    PayeeMatchIgnoreCaseRole = Qt::UserRole + 9,
    PayeeDefaultAccountRole = Qt::UserRole + 10,
    DisplayOrderRole = Qt::UserRole + 11,              /**< This role is used by the filtering proxies to order the accounts for displaying.*/
    CleanupRole = Qt::UserRole + 12                   /**< This role is used internally by the model to clean up removed accounts. */
  };

  enum PayeeMatch {
    NoMatch = 0,
    NameMatch = 1,
    KeyMatch = 2
  };

  /**
    * The columns of this model.
    */
  enum Columns {
    Payee = 0,
    LastColumnMarker
  };

  ~PayeesModel();

  void load();

public slots:

signals:

private:
  PayeesModel(QObject *parent = 0);

  /**
    * The copy-constructor is private so that only the @ref Models object can create such an object.
    */
  PayeesModel(const PayeesModel&);
  PayeesModel& operator=(PayeesModel&);

  /**
    * Allow only the @ref Models object to create such an object.
    */
  friend class Models;

private:
  class Private;
  Private* const d;
};

/**
  * A proxy model to provide various sorting and filtering operations for @ref PayeesModel.
  *
  * Here is an example of how to use this class in combination with the @ref PayeesModel.
  * (in the example @a widget is a pointer to a model/view widget):
  *
  * @code
  *   PayeesFilterProxyModel *filterModel = new PayeesFilterProxyModel(widget);
  *   filterModel->setSourceModel(Models::instance()->payeesModel());
  *   filterModel->sort(0);
  *
  *   widget->setModel(filterModel);
  * @endcode
  *
  * @endcode
  *
  * @see PayeesModel
  *
  * @author Alvaro Soliverez 2010
  *
  */
class PayeesComboProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  PayeesComboProxyModel(QObject *parent = 0);

  void setFilterList(const bool &allowEmpty, const QList<MyMoneyPayee> &payeeList);

  void clear(void);

protected:
  virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
  virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
  virtual bool acceptSourceItem(const QModelIndex &source) const;

signals:

private:
  class Private;
  Private* const d;

};


#endif // PAYEESMODEL_H
