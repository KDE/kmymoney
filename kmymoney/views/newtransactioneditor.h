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

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"

class NewTransactionEditor : public QFrame
{
  Q_OBJECT

public:
  explicit NewTransactionEditor(QWidget* parent = 0, const QString& accountId = QString());
  virtual ~NewTransactionEditor();

  /**
   * This method returns true if the user pressed the enter button.
   * It remains false, in case the user pressed the cancel button.
   */
  virtual bool accepted() const;

  /**
   * Returns the currently entered amount
   */
  MyMoneyMoney transactionAmount() const;

  /**
   */
  void loadTransaction(const QModelIndex& index);
  void saveTransaction();

  /**
   * Reimplemented to suppress some events in certain conditions
   */
  bool eventFilter(QObject* o, QEvent* e) override;

protected:
  virtual void keyPressEvent(QKeyEvent* e) override;

protected Q_SLOTS:
  virtual void reject();
  virtual void acceptEdit();

  virtual void editSplits();

  virtual void numberChanged(const QString& newNumber);
  virtual void categoryChanged(const QString& accountId);
  virtual void costCenterChanged(int costCenterIndex);
  virtual void postdateChanged(const QDate& date);
  virtual void payeeChanged(int payeeIndex);

  void valueChanged();

Q_SIGNALS:
  void done();
  void transactionChanged(const QString&);

private:
  class Private;
  QScopedPointer<Private> const d;
  static QDate  m_lastPostDateUsed;
};

#endif // NEWTRANSACTIONEDITOR_H

