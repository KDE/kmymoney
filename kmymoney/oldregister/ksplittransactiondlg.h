/*
 * Copyright 2002       Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2011  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KSPLITTRANSACTIONDLG_H
#define KSPLITTRANSACTIONDLG_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;
class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneyAccount;
class MyMoneyTag;

namespace Ui { class KSplitCorrectionDlg; }

class KMM_OLDREGISTER_EXPORT KSplitCorrectionDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KSplitCorrectionDlg)

public:
  explicit KSplitCorrectionDlg(QWidget *parent = nullptr);
  ~KSplitCorrectionDlg();

  Ui::KSplitCorrectionDlg *ui;
};

/**
  * @author Thomas Baumgart
  */

class KSplitTransactionDlgPrivate;
class KSplitTransactionDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KSplitTransactionDlg)

public:
  explicit KSplitTransactionDlg(const MyMoneyTransaction& t,
                                const MyMoneySplit& s,
                                const MyMoneyAccount& acc,
                                const bool amountValid,
                                const bool deposit,
                                const MyMoneyMoney& calculatedValue,
                                const QMap<QString, MyMoneyMoney>& priceInfo,
                                QWidget* parent = nullptr);

  ~KSplitTransactionDlg();

  /**
    * Using this method, an external object can retrieve the result
    * of the dialog.
    *
    * @return MyMoneyTransaction based on the transaction passes during
    *         the construction of this object and modified using the
    *         dialog.
    */
  MyMoneyTransaction transaction() const;

  /**
    * This method calculates the difference between the split that references
    * the account passed as argument to the constructor of this object and
    * all the other splits shown in the register of this dialog.
    *
    * @return difference as MyMoneyMoney object
    */
  MyMoneyMoney diffAmount();

  /**
    * This method calculates the sum of the splits shown in the register
    * of this dialog.
    *
    * @return sum of splits as MyMoneyMoney object
    */
  MyMoneyMoney splitsValue();

public Q_SLOTS:
  int exec() override;

protected Q_SLOTS:
  void accept() override;
  void reject() override;
  void slotClearAllSplits();
  void slotClearUnusedSplits();
  void slotSetTransaction(const MyMoneyTransaction& t);
  void slotCreateCategory(const QString& txt, QString& id);
  void slotCreateTag(const QString &txt, QString &id);
  void slotUpdateButtons();
  void slotMergeSplits();
  void slotEditStarted();

  /// used internally to setup the initial size of all widgets
  void initSize();

Q_SIGNALS:
  /**
    * This signal is sent out, when a new category needs to be created
    * Depending on the setting of either a payment or deposit, the parent
    * account will be preset to Expense or Income.
    *
    * @param account reference to account info. Will be filled by called slot
    * @param parent reference to parent account
    */
  void createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
    * This signal is sent out, when a new tag needs to be created
    * @param txt The name of the tag to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createTag(const QString& txt, QString& id);

  /**
    * Signal is emitted, if any of the widgets enters (@a state equals @a true)
    *  or leaves (@a state equals @a false) object creation mode.
    *
    * @param state Enter (@a true) or leave (@a false) object creation
    */
  void objectCreation(bool state);

private:
  KSplitTransactionDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSplitTransactionDlg)
};

#endif
