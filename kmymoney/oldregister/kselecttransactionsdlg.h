/*
 * Copyright 2007-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KSELECTTRANSACTIONSDLG_H
#define KSELECTTRANSACTIONSDLG_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyTransaction;
class MyMoneyAccount;

namespace KMyMoneyRegister { class SelectedTransactions; class Register;}

class KSelectTransactionsDlgPrivate;
class KMM_OLDREGISTER_EXPORT KSelectTransactionsDlg: public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KSelectTransactionsDlg)

public:
  explicit KSelectTransactionsDlg(const MyMoneyAccount& account, QWidget* parent = nullptr);
  ~KSelectTransactionsDlg();

  /**
   * Adds the transaction @a t to the dialog
   */
  void addTransaction(const MyMoneyTransaction& t);
  int exec() override;

  MyMoneyTransaction transaction() const;
  KMyMoneyRegister::Register *getRegister();

  bool eventFilter(QObject* o, QEvent* e) override;

public Q_SLOTS:
  virtual void slotHelp();

protected Q_SLOTS:
  void slotEnableOk(const KMyMoneyRegister::SelectedTransactions& list);

protected:
  void resizeEvent(QResizeEvent* ev) override;
  void showEvent(QShowEvent* event) override;
  KSelectTransactionsDlgPrivate * const d_ptr;

private:
  Q_DECLARE_PRIVATE(KSelectTransactionsDlg)
};

#endif // KMERGETRANSACTIONSDLG_H
