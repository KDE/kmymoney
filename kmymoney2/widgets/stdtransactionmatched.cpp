/***************************************************************************
                          stdtransactionmatched.cpp
                             -------------------
    begin                : Sat May 11 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <qregion.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <stdtransactionmatched.h>
#include <kmymoneyglobalsettings.h>
#include <register.h>

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransactionMatched::StdTransactionMatched(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  StdTransaction(parent, transaction, split, uniqueId),
  m_drawCounter(parent->drawCounter()-1)
{
  // setup initial size
  setNumRowsRegister(numRowsRegister(KMyMoneyGlobalSettings::showRegisterDetailed()));
}

bool StdTransactionMatched::paintRegisterCellSetup(QPainter* painter, int& row, int& col, QRect& cellRect, QRect& textRect, QColorGroup& cg, QBrush& brush)
{
  QRect r(cellRect);

  bool rc = Transaction::paintRegisterCellSetup(painter, row, col, cellRect, textRect, cg, brush);

  // if not selected paint in matched background color
  if(!isSelected()) {
    cg.setColor(QColorGroup::Base, KMyMoneyGlobalSettings::matchedTransactionColor());
    brush = QBrush(cg.base());
  }

  // the first line needs to be painted across all columns
  if(row + m_additionalRows - m_rowsRegister == 0) {
    // avoid painting the text over multiple columns twice for the same update round
    unsigned int drawCounter = m_parent->drawCounter();
    if(m_drawCounter == drawCounter) {
      return false;
    }


    // the fixed text always uses all cols
    col = m_parent->columnAt(1);
    painter->translate(-r.x() + m_parent->columnPos(col), 0);
#if 0
    r.setX(m_parent->columnPos(col));
    r.setWidth(m_parent->visibleWidth());
    painter->translate(r.x(), r.y());
#endif
    cellRect.setX(0);
    cellRect.setY(0);
    cellRect.setWidth(m_parent->visibleWidth());
    cellRect.setHeight(m_parent->rowHeight(row + m_startRow));

    textRect = cellRect;
    textRect.setX(2);
    textRect.setWidth(textRect.width()-4);
  }
  return rc;
}

void StdTransactionMatched::registerCellText(QString& txt, int& align, int row, int col, QPainter* painter)
{
  // run through the standard
  StdTransaction::registerCellText(txt, align, row, col, painter);

  // we only cover the additional rows
  if(row >= m_rowsRegister - m_additionalRows) {
    // make row relative to the last three rows
    row += m_additionalRows - m_rowsRegister;

    // remove anything that had been added by the standard method
    txt = "";

    // and we draw this information in italics
    if(painter) {
      QFont font = painter->font();
      font.setItalic(true);
      painter->setFont(font);
    }

    MyMoneyTransaction matchedTransaction = m_split.matchedTransaction();
    MyMoneySplit matchedSplit;
    try {
      matchedSplit = matchedTransaction.splitById(m_split.value("kmm-match-split"));
    } catch(MyMoneyException *e) {
      delete e;
    }

    Q3ValueList<MyMoneySplit>::const_iterator it_s;
    const Q3ValueList<MyMoneySplit>& list = matchedTransaction.splits();
    MyMoneyMoney importedValue;
    for(it_s = list.begin(); it_s != list.end(); ++it_s) {
      if((*it_s).accountId() == m_account.id()) {
        importedValue += (*it_s).shares();
      }
    }

    QDate postDate;
    QString memo;
    switch(row) {
      case 0:
        if(painter)
          txt = QString(" ")+i18n("KMyMoney has matched a downloaded transaction with a manually entered one (result above)");
        // return true for the first visible column only
        break;

      case 1:
        switch(col) {
          case DateColumn:
            align |= Qt::AlignLeft;
            txt = i18n("Bank entry:");
            break;

          case DetailColumn:
            align |= Qt::AlignLeft;
            txt = QString("%1 %2").arg(matchedTransaction.postDate().toString(Qt::ISODate)).arg(matchedTransaction.memo());
            break;

          case PaymentColumn:
            align |= Qt::AlignRight;
            if(importedValue.isNegative()) {
              txt = (-importedValue).formatMoney(m_account.fraction());
            }
            break;

          case DepositColumn:
            align |= Qt::AlignRight;
            if(!importedValue.isNegative()) {
              txt = importedValue.formatMoney(m_account.fraction());
            }
            break;
        }
        break;

      case 2:
        switch(col) {
          case DateColumn:
            align |= Qt::AlignLeft;
            txt = i18n("Your entry:");
            break;

          case DetailColumn:
            align |= Qt::AlignLeft;
            postDate = m_transaction.postDate();
            if(!m_split.value("kmm-orig-postdate").isEmpty()) {
              postDate = QDate::fromString(m_split.value("kmm-orig-postdate"), Qt::ISODate);
            }
            memo = m_split.memo();
            if(!matchedSplit.memo().isEmpty() && memo != matchedSplit.memo()) {
              int pos = memo.findRev(matchedSplit.memo());
              if(pos != -1) {
                memo = memo.left(pos);
                if(memo.endsWith("\n"))
                  memo = memo.left(pos-1);
              }
            }
            txt = QString("%1 %2").arg(postDate.toString(Qt::ISODate)).arg(memo);
            break;

          case PaymentColumn:
            align |= Qt::AlignRight;
            if(m_split.value().isNegative()) {
              txt = (-m_split.value(m_transaction.commodity(), m_splitCurrencyId)).formatMoney(m_account.fraction());
            }
            break;

          case DepositColumn:
            align |= Qt::AlignRight;
            if(!m_split.value().isNegative()) {
              txt = m_split.value(m_transaction.commodity(), m_splitCurrencyId).formatMoney(m_account.fraction());
            }
            break;

        }
        break;
    }
  }
}

void StdTransactionMatched::paintRegisterGrid(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& _cg) const
{
  // the last 3 rows should not show a grid
  if(row < m_rowsRegister - m_additionalRows) {
    Transaction::paintRegisterGrid(painter, row, col, r, _cg);

  } else if(row == m_rowsRegister-1) {
    painter->setPen(KMyMoneyGlobalSettings::listGridColor());
    painter->drawLine(r.x(), r.height()-1, r.width(), r.height()-1);
  }
}
