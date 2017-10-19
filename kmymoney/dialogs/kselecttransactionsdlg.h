/***************************************************************************
                          kselecttransactionsdlg.h
                             -------------------
    begin                : Wed May 16 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "selectedtransaction.h"

#include "ui_kselecttransactionsdlgdecl.h"


class KSelectTransactionsDlgDecl : public QDialog, public Ui::KSelectTransactionsDlgDecl
{
public:
  KSelectTransactionsDlgDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};
class KSelectTransactionsDlg: public KSelectTransactionsDlgDecl
{
  Q_OBJECT
public:
  explicit KSelectTransactionsDlg(const MyMoneyAccount& account, QWidget* parent = 0);

  /**
   * Adds the transaction @a t to the dialog
   */
  void addTransaction(const MyMoneyTransaction& t);
  int exec();

  MyMoneyTransaction transaction() const;

  bool eventFilter(QObject* o, QEvent* e);

public slots:
  virtual void slotHelp();

protected slots:
  void slotEnableOk(const KMyMoneyRegister::SelectedTransactions& list);

protected:
  void resizeEvent(QResizeEvent* ev);
  void showEvent(QShowEvent* event);

private:
  /**
    * The account in which the transactions are displayed
    */
  MyMoneyAccount m_account;
};

#endif // KMERGETRANSACTIONSDLG_H
