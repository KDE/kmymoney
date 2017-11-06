/***************************************************************************
                          kselecttransactionsdlg.h
                             -------------------
    begin                : Wed May 16 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSELECTTRANSACTIONSDLG_H
#define KSELECTTRANSACTIONSDLG_H

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
class KSelectTransactionsDlg: public QDialog
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

public slots:
  virtual void slotHelp();

protected slots:
  void slotEnableOk(const KMyMoneyRegister::SelectedTransactions& list);

protected:
  void resizeEvent(QResizeEvent* ev) override;
  void showEvent(QShowEvent* event) override;
  KSelectTransactionsDlgPrivate * const d_ptr;

private:
  Q_DECLARE_PRIVATE(KSelectTransactionsDlg)
};

#endif // KMERGETRANSACTIONSDLG_H
