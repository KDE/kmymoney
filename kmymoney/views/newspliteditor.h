/***************************************************************************
                          newspliteditor.h
                             -------------------
    begin                : Sat Apr 9 2016
    copyright            : (C) 2016 by Thomas Baumgart
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

#ifndef NEWSPLITEDITOR_H
#define NEWSPLITEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
#include <QScopedPointer>
class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"
#include "ledgermodel.h"

class NewSplitEditor : public QFrame
{
  Q_OBJECT

public:
  explicit NewSplitEditor(QWidget* parent, const QString& accountId = QString());
  virtual ~NewSplitEditor();

  /**
   * This method returns true if the user pressed the enter button.
   * It remains false, in case the user pressed the cancel button.
   */
  virtual bool accepted() const;

  void setInversedViewOfAmounts(bool inverse);
  bool isInversedViewOfAmounts();

protected:
  virtual void keyPressEvent(QKeyEvent* e);

public Q_SLOTS:
  /**
   * This method returns the transaction split id passed
   * to setSplitId().
   */
  QString splitId() const;

  /**
   * Returns the id of the selected account in the category widget
   */
  QString accountId() const;
  void setAccountId(const QString& id);

  /**
   * Returns the contents of the memo widget
   */
  QString memo() const;
  void setMemo(const QString& memo);

  MyMoneyMoney amount() const;
  void setAmount(MyMoneyMoney value);

  QString costCenterId() const;
  void setCostCenterId(const QString& id);

protected Q_SLOTS:
  virtual void reject();
  virtual void acceptEdit();


  virtual void numberChanged(const QString& newNumber);
  virtual void categoryChanged(const QString& accountId);
  virtual void costCenterChanged(int costCenterIndex);
  virtual void amountChanged(const QString& newAmount);

Q_SIGNALS:
  void done();
  void transactionChanged(const QString&);

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // NEWSPLITEDITOR_H

