/***************************************************************************
                          newtransactioneditor.h
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NEWTRANSACTIONEDITOR_H
#define NEWTRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QWidget;
#if 0
#include <QObject>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTableView>

// ----------------------------------------------------------------------------
// KDE Includes

#endif
// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"

class NewTransactionEditor : public QFrame
{
  Q_OBJECT
  Q_PROPERTY(QString transactionId READ transactionId WRITE setTransactionId NOTIFY transactionChanged USER true)

public:
  explicit NewTransactionEditor(QWidget* parent = 0, const QString& accountId = QString());
  virtual ~NewTransactionEditor();

  /**
   * This method returns true if the user pressed the enter button.
   * It remains false, in case the user pressed the cancel button.
   */
  virtual bool accepted() const;

  QString transactionId() const;

  /**
   * This method returns the transaction if the user left
   * the editor via the save button.
   */
  MyMoneyTransaction transaction() const;

  /**
   * Returns the currently entered amount
   */
  MyMoneyMoney transactionAmount() const;

  /**
   * Used to invert the sign of the value depending on the
   * account one is working in
   */
  void setInvertSign(bool invertSign);

protected:
  virtual void keyPressEvent(QKeyEvent* e);

public Q_SLOTS:
  void setTransactionId(const QString& id);
  void setAccountId(const QString& id);

protected Q_SLOTS:
  virtual void reject();
  virtual void acceptEdit();

  virtual void editSplits();

  virtual void numberChanged(const QString& newNumber);
  virtual void categoryChanged(const QString& accountId);
  virtual void costCenterChanged(int costCenterIndex);
  virtual void postdateChanged(const QDate& date);

Q_SIGNALS:
  void done();
  void transactionChanged(const QString&);

private:
  class Private;
  Private * const d;
};

#endif // NEWTRANSACTIONEDITOR_H

