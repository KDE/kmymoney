/*
 * Copyright 2016-2020  Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneymoney.h"
class MyMoneySecurity;

class NewSplitEditor : public QFrame
{
  Q_OBJECT

public:
  /**
   * @a accountId is the current account displayed for the transaction
   */
  explicit NewSplitEditor(QWidget* parent, const MyMoneySecurity& commodity, const QString& accountId = QString());
  virtual ~NewSplitEditor();

  /**
   * This method returns true if the user pressed the enter button.
   * It remains false, in case the user pressed the cancel button.
   */
  virtual bool accepted() const;

  void setShowValuesInverted(bool inverse);
  bool showValuesInverted();

  void setPostDate(const QDate& date);

  void startLoadingSplit();
  void finishLoadingSplit();

protected:
  void keyPressEvent(QKeyEvent* e) final override;

public Q_SLOTS:
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

  MyMoneyMoney shares() const;
  void setShares(const MyMoneyMoney& shares);

  MyMoneyMoney value() const;
  void setValue(const MyMoneyMoney& value);

  QString costCenterId() const;
  void setCostCenterId(const QString& id);

  QString number() const;
  void setNumber(const QString& id);

  QString payeeId() const;
  void setPayeeId(const QString& id);

protected Q_SLOTS:
  virtual void reject();
  virtual void acceptEdit();


  virtual void numberChanged(const QString& newNumber);
  virtual void categoryChanged(const QString& accountId);
  virtual void costCenterChanged(int costCenterIndex);
  virtual void amountChanged();

Q_SIGNALS:
  void done();
  void transactionChanged(const QString&);

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // NEWSPLITEDITOR_H

