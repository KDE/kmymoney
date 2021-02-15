/*
    SPDX-FileCopyrightText: 2007-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRANSACTIONEDITOR_P_H
#define TRANSACTIONEDITOR_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QList>
#include <QMap>
#include <QString>
#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneylineedit.h"
#include "kmymoneyutils.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "register.h"
#include "registeritem.h"
#include "selectedtransactions.h"
#include "transactioneditor.h"
#include "qwidgetcontainer.h"
#include "widgetenums.h"

class MyMoneyMoney;
class TransactionEditorContainer;
namespace KMyMoneyRegister { class Transaction; }

class TransactionEditorPrivate
{
  Q_DISABLE_COPY(TransactionEditorPrivate)
  Q_DECLARE_PUBLIC(TransactionEditor)

public:
  explicit TransactionEditorPrivate(TransactionEditor *qq) :
    q_ptr(qq),
    m_paymentMethod(eMyMoney::Schedule::PaymentType::Any),
    m_regForm(nullptr),
    m_item(nullptr),
    m_initialAction(eWidgets::eRegister::Action::None),
    m_openEditSplits(false),
    m_memoChanged(false)
  {
  }

  ~TransactionEditorPrivate()
  {
  }

  void init()
  {
    m_paymentMethod = eMyMoney::Schedule::PaymentType::Any;
    m_regForm = 0;
    m_item = 0;
    m_initialAction = eWidgets::eRegister::Action::None;
    m_openEditSplits = false;
    m_memoChanged = false;
  }

  /**
  *  If a new or an edited transaction has a valid number, keep it with the account
  */
  void keepNewNumber(const MyMoneyTransaction& tr)
  {
    Q_Q(TransactionEditor);
    // verify that new number, possibly containing alpha, is valid
    auto txn = tr;
    auto file = MyMoneyFile::instance();
    if (!txn.splits().isEmpty()) {
      QString number = txn.splits().first().number();
      if (KMyMoneyUtils::numericPart(number) > 0) {
        // numeric is valid
        auto numberEdit = dynamic_cast<KMyMoneyLineEdit*>(q->haveWidget("number"));
        if (numberEdit) {
          numberEdit->loadText(number);
          MyMoneySplit split = txn.splits().first();
          split.setNumber(number);
          txn.modifySplit(split);
          m_account.setValue("lastNumberUsed", number);
          file->modifyAccount(m_account);
        }
      }
    }
  }

  TransactionEditor                      *q_ptr;
  QString                                 m_scheduleInfo;
  eMyMoney::Schedule::PaymentType         m_paymentMethod;
  QString                                 m_memoText;
  QList<MyMoneySplit>                     m_splits;
  KMyMoneyRegister::SelectedTransactions  m_transactions;
  QList<const QWidget*>                   m_finalEditWidgets;
  TransactionEditorContainer*             m_regForm;
  KMyMoneyRegister::Transaction*          m_item;
  KMyMoneyRegister::QWidgetContainer      m_editWidgets;
  MyMoneyAccount                          m_account;
  MyMoneyTransaction                      m_transaction;
  MyMoneySplit                            m_split;
  QDate                                   m_lastPostDate;
  QMap<QString, MyMoneyMoney>             m_priceInfo;
  eWidgets::eRegister::Action              m_initialAction;
  bool                                    m_openEditSplits;
  bool                                    m_memoChanged;
};

#endif // KMERGETRANSACTIONSDLG_H
