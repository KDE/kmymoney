/***************************************************************************
                         kmymoneyaccountcombo  -  description
                            -------------------
   begin                : Mon May 31 2004
   copyright            : (C) 2000-2004 by Michael Edwardes
   email                : mte@users.sourceforge.net
                          Javier Campos Morales <javi_c@users.sourceforge.net>
                          Felix Rodriguez <frodriguez@users.sourceforge.net>
                          John C <thetacoturtle@users.sourceforge.net>
                          Thomas Baumgart <ipwizard@users.sourceforge.net>
                          Kevin Tambascio <ktambascio@users.sourceforge.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYACCOUNTCOMBO_H
#define KMYMONEYACCOUNTCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMouseEvent>
#include <QKeyEvent>
#include <QList>
#include <QItemSelection>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "accountsmodel.h"

class kMyMoneyAccountCompletion;

/**
  * A proxy model used to filter all the data from the core accounts model leaving
  * only the name of the accounts so this model can be used in the account
  * completion combo.
  *
  * @see AccountsModel
  * @see AccountsFilterProxyModel
  *
  * @author Cristian Onet 2010
  *
  */
class AccountNamesFilterProxyModel : public AccountsFilterProxyModel
{
  Q_OBJECT

public:
  AccountNamesFilterProxyModel(QObject *parent = 0);

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

protected:
  bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
};

/**
  *
  *
  * @author Cristian Onet
  */
class KMyMoneyAccountCombo : public KComboBox
{
  Q_OBJECT
public:
  explicit KMyMoneyAccountCombo(AccountNamesFilterProxyModel *model, QWidget *parent = 0);
  explicit KMyMoneyAccountCombo(QWidget *parent = 0);
  ~KMyMoneyAccountCombo();

  void setSelected(const QString& id);
  const QString& getSelected() const;

  void setModel(AccountNamesFilterProxyModel *model);

public slots:
  void expandAll();

protected:
  virtual void wheelEvent(QWheelEvent *ev);

protected slots:
  void activated();

signals:
  void accountSelected(const QString&);

private:
  class Private;
  Private* const d;
};

#endif
