/***************************************************************************
                          stdtransactionmatched.cpp
                             -------------------
    begin                : Sat May 11 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "stdtransactionmatched.h"
#include "stdtransaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QPainter>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "widgetenums.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransactionMatched::StdTransactionMatched(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  StdTransaction(parent, transaction, split, uniqueId)
{
  // setup initial size
  setNumRowsRegister(numRowsRegister(KMyMoneyGlobalSettings::showRegisterDetailed()));
}

StdTransactionMatched::~StdTransactionMatched()
{
}

const char* StdTransactionMatched::className()
{
  return "StdTransactionMatched";
}

bool StdTransactionMatched::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  auto rc = Transaction::paintRegisterCellSetup(painter, option, index);

  // if not selected paint in matched background color
  if (!isSelected()) {
    option.palette.setColor(QPalette::Base, KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionMatched));
    option.palette.setColor(QPalette::AlternateBase, KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionMatched));
  }
  //TODO: the first line needs to be painted across all columns
  return rc;
}

void StdTransactionMatched::registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter)
{
  Q_D(StdTransaction);
  // run through the standard
  StdTransaction::registerCellText(txt, align, row, col, painter);

  // we only cover the additional rows
  if (row >= RegisterItem::numRowsRegister() - m_additionalRows) {
    // make row relative to the last three rows
    row += m_additionalRows - RegisterItem::numRowsRegister();

    // remove anything that had been added by the standard method
    txt = QString();

    // and we draw this information in italics
    if (painter) {
      QFont font = painter->font();
      font.setItalic(true);
      painter->setFont(font);
    }

    MyMoneyTransaction matchedTransaction = d->m_split.matchedTransaction();
    MyMoneySplit matchedSplit;
    try {
      matchedSplit = matchedTransaction.splitById(d->m_split.value("kmm-match-split"));
    } catch (const MyMoneyException &) {
    }

    MyMoneyMoney importedValue;
    foreach (const auto split, matchedTransaction.splits()) {
      if (split.accountId() == d->m_account.id()) {
        importedValue += split.shares();
      }
    }

    QDate postDate;
    QString memo;
    switch (row) {
      case 0:
        if (painter && col == (int)eWidgets::eTransaction::Column::Detail)
          txt = QString(" ") + i18n("KMyMoney has matched the two selected transactions (result above)");
        // return true for the first visible column only
        break;

      case 1:
        switch (col) {
          case (int)eWidgets::eTransaction::Column::Date:
            align |= Qt::AlignLeft;
            txt = i18n("Bank entry:");
            break;

          case (int)eWidgets::eTransaction::Column::Detail:
            align |= Qt::AlignLeft;
            memo = matchedTransaction.memo();
            memo.replace("\n\n", "\n");
            memo.replace('\n', ", ");
            txt = QString("%1 %2").arg(matchedTransaction.postDate().toString(Qt::ISODate)).arg(memo);
            break;

          case (int)eWidgets::eTransaction::Column::Payment:
            align |= Qt::AlignRight;
            if (importedValue.isNegative()) {
              txt = (-importedValue).formatMoney(d->m_account.fraction());
            }
            break;

          case (int)eWidgets::eTransaction::Column::Deposit:
            align |= Qt::AlignRight;
            if (!importedValue.isNegative()) {
              txt = importedValue.formatMoney(d->m_account.fraction());
            }
            break;
        }
        break;

      case 2:
        switch (col) {
          case (int)eWidgets::eTransaction::Column::Date:
            align |= Qt::AlignLeft;
            txt = i18n("Your entry:");
            break;

          case (int)eWidgets::eTransaction::Column::Detail:
            align |= Qt::AlignLeft;
            postDate = d->m_transaction.postDate();
            if (!d->m_split.value("kmm-orig-postdate").isEmpty()) {
              postDate = QDate::fromString(d->m_split.value("kmm-orig-postdate"), Qt::ISODate);
            }
            memo = d->m_split.memo();
            if (!matchedSplit.memo().isEmpty() && memo != matchedSplit.memo()) {
              int pos = memo.lastIndexOf(matchedSplit.memo());
              if (pos != -1) {
                memo = memo.left(pos);
                // replace all new line characters because we only have one line available for the displayed data
              }
            }
            memo.replace("\n\n", "\n");
            memo.replace('\n', ", ");
            txt = QString("%1 %2").arg(postDate.toString(Qt::ISODate)).arg(memo);
            break;

          case (int)eWidgets::eTransaction::Column::Payment:
            align |= Qt::AlignRight;
            if (d->m_split.value().isNegative()) {
              txt = (-d->m_split.value(d->m_transaction.commodity(), d->m_splitCurrencyId)).formatMoney(d->m_account.fraction());
            }
            break;

          case (int)eWidgets::eTransaction::Column::Deposit:
            align |= Qt::AlignRight;
            if (!d->m_split.value().isNegative()) {
              txt = d->m_split.value(d->m_transaction.commodity(), d->m_splitCurrencyId).formatMoney(d->m_account.fraction());
            }
            break;

        }
        break;
    }
  }
}

int StdTransactionMatched::numRowsRegister(bool expanded) const
{
  return StdTransaction::numRowsRegister(expanded) + m_additionalRows;
}

int StdTransactionMatched::numRowsRegister() const
{
  return StdTransaction::numRowsRegister();
}
