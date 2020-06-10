/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#include "transaction.h"
#include "transaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QPainter>
#include <QWidget>
#include <QList>
#include <QPixmap>
#include <QHeaderView>
#include <QApplication>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "register.h"
#include "kmymoneycategory.h"
#include "kmymoneydateinput.h"
#include "transactionform.h"
#include "kmymoneyutils.h"
#include "registerfilter.h"
#include "tabbar.h"

#include "kmymoneysettings.h"
#include "widgetenums.h"
#include "mymoneyenums.h"

using namespace eWidgets;
using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

static unsigned char attentionSign[] = {
  0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
  0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
  0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x14,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x8D, 0x89, 0x1D,
  0x0D, 0x00, 0x00, 0x00, 0x04, 0x73, 0x42, 0x49,
  0x54, 0x08, 0x08, 0x08, 0x08, 0x7C, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58,
  0x74, 0x53, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72,
  0x65, 0x00, 0x77, 0x77, 0x77, 0x2E, 0x69, 0x6E,
  0x6B, 0x73, 0x63, 0x61, 0x70, 0x65, 0x2E, 0x6F,
  0x72, 0x67, 0x9B, 0xEE, 0x3C, 0x1A, 0x00, 0x00,
  0x02, 0x05, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8D,
  0xAD, 0x95, 0xBF, 0x4B, 0x5B, 0x51, 0x14, 0xC7,
  0x3F, 0x2F, 0xBC, 0x97, 0x97, 0x97, 0x97, 0x77,
  0xF3, 0xF2, 0x1C, 0xA4, 0x54, 0x6B, 0x70, 0x10,
  0x44, 0x70, 0x2A, 0x91, 0x2E, 0x52, 0x02, 0x55,
  0x8A, 0xB5, 0xA3, 0xAB, 0x38, 0x08, 0x66, 0xCC,
  0xEE, 0xE0, 0xE2, 0x20, 0xB8, 0x38, 0xB8, 0xB8,
  0xF8, 0x1F, 0x38, 0x29, 0xA5, 0x29, 0x74, 0x90,
  0x0E, 0x0D, 0x0E, 0x22, 0x1D, 0x44, 0xA8, 0xD0,
  0xD4, 0xB4, 0x58, 0x4B, 0x09, 0xF9, 0xF1, 0x4A,
  0x3B, 0xD4, 0xD3, 0xE1, 0x55, 0xD3, 0x34, 0xAF,
  0x49, 0x6C, 0x3D, 0xF0, 0x85, 0x7B, 0xCF, 0xFD,
  0x9E, 0xEF, 0x3D, 0xE7, 0xFE, 0xD4, 0x44, 0x84,
  0xDB, 0xB4, 0x48, 0x2F, 0xA4, 0x94, 0xAB, 0xE5,
  0x52, 0xAE, 0x96, 0xEB, 0x49, 0x51, 0x44, 0x3A,
  0x02, 0x18, 0x88, 0xC7, 0xF1, 0xE3, 0x71, 0x7C,
  0x60, 0xA0, 0x1B, 0xBF, 0x6B, 0x86, 0x49, 0xC5,
  0x46, 0x3E, 0x47, 0x34, 0x9F, 0x23, 0x9A, 0x54,
  0x6C, 0xFC, 0x57, 0x86, 0x40, 0xC6, 0x4B, 0xE1,
  0x37, 0xCA, 0x48, 0xA3, 0x8C, 0x78, 0x29, 0x7C,
  0x20, 0xD3, 0x31, 0xA6, 0xD3, 0xA0, 0x52, 0x1C,
  0x6D, 0x6F, 0x72, 0xD9, 0x28, 0x23, 0xFE, 0x07,
  0x64, 0x7B, 0x93, 0x4B, 0xA5, 0x38, 0xFA, 0x27,
  0x41, 0x60, 0x6E, 0x74, 0x84, 0x7A, 0xE5, 0x1D,
  0x92, 0x54, 0x88, 0xE7, 0x22, 0xD5, 0x12, 0x32,
  0x3A, 0x42, 0x1D, 0x98, 0xBB, 0x91, 0x20, 0x60,
  0xDA, 0x36, 0x17, 0xFB, 0x7B, 0xC8, 0xC1, 0x4B,
  0x04, 0x02, 0xBC, 0x7E, 0x81, 0xEC, 0xEF, 0x21,
  0xB6, 0xCD, 0x05, 0x60, 0xF6, 0x2C, 0x68, 0x9A,
  0x2C, 0xCF, 0x4C, 0xE1, 0x4B, 0x05, 0x39, 0x3F,
  0x69, 0x0A, 0xBE, 0x7F, 0x83, 0x48, 0x05, 0x99,
  0x99, 0xC2, 0x37, 0x4D, 0x96, 0x7B, 0x12, 0x04,
  0xFA, 0x2D, 0x8B, 0xC6, 0xE9, 0x61, 0x10, 0x2C,
  0x15, 0xC4, 0x8A, 0x21, 0x86, 0x8E, 0xFC, 0xF8,
  0x12, 0xF4, 0x4F, 0x0F, 0x11, 0xCB, 0xA2, 0x01,
  0xF4, 0x77, 0x3D, 0x36, 0x4E, 0x82, 0xF5, 0xA5,
  0x05, 0x8C, 0xE1, 0x74, 0xD3, 0x37, 0x34, 0x18,
  0x20, 0xF2, 0x8B, 0x3D, 0x9C, 0x86, 0xA5, 0x05,
  0x0C, 0x27, 0xC1, 0x7A, 0xC7, 0x63, 0x03, 0x8C,
  0x2B, 0x07, 0xBF, 0x5A, 0x6A, 0x66, 0x27, 0x15,
  0x64, 0x3A, 0x8B, 0x3C, 0x7A, 0xD8, 0xEA, 0xAB,
  0x96, 0x10, 0xE5, 0xE0, 0x03, 0xE3, 0x7F, 0xCD,
  0x50, 0x39, 0x6C, 0xAD, 0xAD, 0x10, 0x53, 0xAA,
  0x75, 0xD2, 0xF4, 0xBD, 0x00, 0x2D, 0x5C, 0x05,
  0x6B, 0x2B, 0xC4, 0x94, 0xC3, 0xD6, 0xEF, 0xFE,
  0x6B, 0x41, 0x4D, 0xD3, 0x66, 0xFB, 0x3C, 0xC6,
  0x16, 0xE7, 0xDB, 0x97, 0x61, 0xE2, 0x3E, 0x3C,
  0xC8, 0xB4, 0x15, 0xC7, 0xE2, 0x3C, 0x91, 0x3E,
  0x8F, 0x31, 0x4D, 0xD3, 0x66, 0x5B, 0x4A, 0x06,
  0x8C, 0x84, 0xCD, 0x59, 0x61, 0xA7, 0xB5, 0xAC,
  0x2B, 0x9C, 0x1C, 0x04, 0x08, 0x1B, 0x2B, 0xEC,
  0x20, 0x09, 0x9B, 0x33, 0xC0, 0xB8, 0xDE, 0x65,
  0x43, 0x27, 0x9F, 0x9D, 0xA4, 0x1E, 0x16, 0xF0,
  0xF9, 0x6D, 0xB0, 0xC3, 0x86, 0x1E, 0xB4, 0xC3,
  0x38, 0xD9, 0x49, 0xEA, 0x86, 0x4E, 0xFE, 0xEA,
  0x29, 0xF4, 0x2C, 0x8B, 0xDA, 0x71, 0x31, 0x9C,
  0xFC, 0xF5, 0x23, 0x32, 0x34, 0x88, 0xDC, 0xBD,
  0x13, 0x5C, 0xBF, 0x30, 0xCE, 0x71, 0x11, 0xB1,
  0x2C, 0x6A, 0x80, 0xA7, 0xDB, 0x36, 0xAB, 0x4F,
  0xA6, 0x89, 0xBA, 0x49, 0x38, 0xFF, 0xD4, 0xBE,
  0x4E, 0x00, 0xAF, 0x9E, 0x81, 0x08, 0xD4, 0xEA,
  0x01, 0xFE, 0x34, 0x37, 0x09, 0x4F, 0x1F, 0x13,
  0xDD, 0x7D, 0xCE, 0xAA, 0x96, 0x72, 0x29, 0x7C,
  0xFB, 0xCE, 0x44, 0xB8, 0xD4, 0xCD, 0x2C, 0x66,
  0x52, 0xD4, 0x6E, 0xFB, 0x0B, 0xF8, 0x09, 0x63,
  0x63, 0x31, 0xE4, 0x85, 0x76, 0x2E, 0x0E, 0x00,
  0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE,
  0x42, 0x60, 0x82
};

Transaction::Transaction(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  RegisterItem(*new TransactionPrivate, parent)
{
  Q_D(Transaction);
  d->m_transaction = transaction;
  d->m_split = split;
  d->m_form = nullptr;
  d->m_uniqueId = d->m_transaction.id();
  d->init(uniqueId);
}

Transaction::Transaction(TransactionPrivate &dd, Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  RegisterItem(dd, parent)
{
  Q_D(Transaction);
  d->m_form = nullptr;
  d->m_transaction = transaction;
  d->m_split = split;
  d->m_uniqueId = d->m_transaction.id();
  d->init(uniqueId);
}

Transaction::~Transaction()
{
}

const char* Transaction::className()
{
  return "Transaction";
}

bool Transaction::isSelectable() const
{
  return true;
}

bool Transaction::isSelected() const
{
  Q_D(const Transaction);
  return d->m_selected;
}

void Transaction::setSelected(bool selected)
{
  Q_D(Transaction);
  if (!selected || (selected && isVisible()))
    d->m_selected = selected;
}

bool Transaction::canHaveFocus() const
{
  return true;
}

bool Transaction::hasFocus() const
{
  Q_D(const Transaction);
  return d->m_focus;
}

bool Transaction::hasEditorOpen() const
{
  Q_D(const Transaction);
  return d->m_inEdit;
}

bool Transaction::isScheduled() const
{
  return false;
}

void Transaction::setFocus(bool focus, bool updateLens)
{
  Q_D(Transaction);
  if (focus != d->m_focus) {
    d->m_focus = focus;
  }
  if (updateLens) {
    if (KMyMoneySettings::ledgerLens()
        || !KMyMoneySettings::transactionForm()
        || KMyMoneySettings::showRegisterDetailed()
        || d->m_parent->ledgerLens()) {
      if (focus)
        setNumRowsRegister(numRowsRegister(true));
      else
        setNumRowsRegister(numRowsRegister(KMyMoneySettings::showRegisterDetailed()));
    }
  }
}

bool Transaction::isErroneous() const
{
  Q_D(const Transaction);
  return d->m_erroneous;
}

QDate Transaction::sortPostDate() const
{
  Q_D(const Transaction);
  return d->m_transaction.postDate();
}

int Transaction::sortSamePostDate() const
{
  return 2;
}

QDate Transaction::sortEntryDate() const
{
  Q_D(const Transaction);
  return d->m_transaction.entryDate();
}

const QString& Transaction::sortPayee() const
{
  Q_D(const Transaction);
  return d->m_payee;
}

const QList<QString>& Transaction::sortTagList() const
{
  Q_D(const Transaction);
  return d->m_tagList;
}

MyMoneyMoney Transaction::sortValue() const
{
  Q_D(const Transaction);
  return d->m_split.shares();
}

QString Transaction::sortNumber() const
{
  Q_D(const Transaction);
  return d->m_split.number();
}

const QString& Transaction::sortEntryOrder() const
{
  Q_D(const Transaction);
  return d->m_uniqueId;
}

eRegister::CashFlowDirection Transaction::sortType() const
{
  Q_D(const Transaction);
  return d->m_split.shares().isNegative() ? eRegister::CashFlowDirection::Payment : eRegister::CashFlowDirection::Deposit;
}

const QString& Transaction::sortCategory() const
{
  Q_D(const Transaction);
  return d->m_category;
}

eMyMoney::Split::State Transaction::sortReconcileState() const
{
  Q_D(const Transaction);
  return d->m_split.reconcileFlag();
}

QString Transaction::id() const
{
  Q_D(const Transaction);
  return d->m_uniqueId;
}

const MyMoneyTransaction& Transaction::transaction() const
{
  Q_D(const Transaction);
  return d->m_transaction;
}

const MyMoneySplit& Transaction::split() const
{
  Q_D(const Transaction);
  return d->m_split;
}

void Transaction::setBalance(const MyMoneyMoney& balance)
{
  Q_D(Transaction);
  d->m_balance = balance;
}

const MyMoneyMoney& Transaction::balance() const
{
  Q_D(const Transaction);
  return d->m_balance;
}

bool Transaction::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  Q_D(Transaction);
  Q_UNUSED(painter)

  if (d->m_reducedIntensity) {
    option.palette.setColor(QPalette::Text, option.palette.color(QPalette::Disabled, QPalette::Text));
  }

  if (d->m_selected) {
    option.state |= QStyle::State_Selected;
  } else {
    option.state &= ~QStyle::State_Selected;
  }

  if (d->m_focus) {
    option.state |= QStyle::State_HasFocus;
  } else {
    option.state &= ~QStyle::State_HasFocus;
  }

  if (option.widget && option.widget->hasFocus()) {
    option.palette.setCurrentColorGroup(QPalette::Active);
  } else {
    option.palette.setCurrentColorGroup(QPalette::Inactive);
  }

  if (index.column() == 0) {
    option.viewItemPosition = QStyleOptionViewItem::Beginning;
  } else if (index.column() == (int)eTransaction::Column::LastColumn - 1) {
    option.viewItemPosition = QStyleOptionViewItem::End;
  } else {
    option.viewItemPosition = QStyleOptionViewItem::Middle;
  }

  // do we need to switch to the error color?
  if (d->m_erroneous) {
    option.palette.setColor(QPalette::Text, KMyMoneySettings::schemeColor(SchemeColor::TransactionErroneous));
  }

  // do we need to switch to the negative balance color?
  if (index.column() == (int)eTransaction::Column::Balance) {
    bool showNegative = d->m_balance.isNegative();
    if (d->m_account.accountGroup() == eMyMoney::Account::Type::Liability && !d->m_balance.isZero())
      showNegative = !showNegative;
    if (showNegative)
      option.palette.setColor(QPalette::Text, KMyMoneySettings::schemeColor(SchemeColor::TransactionErroneous));
  }
  return true;
}

void Transaction::registerCellText(QString& txt, int row, int col)
{
  Qt::Alignment align;
  registerCellText(txt, align, row, col, 0);
}

void Transaction::paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  Q_D(Transaction);
  painter->save();
  if (paintRegisterCellSetup(painter, option, index)) {
    const QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    const QWidget* widget = option.widget;

    // clear the mouse over state before painting the background
    option.state &= ~QStyle::State_MouseOver;
    // the background
    if (option.state & QStyle::State_Selected || option.state & QStyle::State_HasFocus) {
      // if this is not the first row of the transaction paint the previous rows
      // since the selection background is painted from the first row of the transaction
      if (index.row() > startRow()) {
        QStyleOptionViewItem optionSibling = option;
        QModelIndex previousRowItem = index.sibling(index.row() - 1, index.column());
        optionSibling.rect = d->m_parent->visualRect(previousRowItem);
        paintRegisterCell(painter, optionSibling, previousRowItem);
      }
      // paint the selection background only from the first row on to the last row at once
      if (index.row() == startRow()) {
        QRect old = option.rect;
        int extraHeight = 0;
        if (d->m_inRegisterEdit) {
          // since, when editing a transaction inside the register (without the transaction form),
          // row heights can have various sizes (the memo row is larger than the rest) we have
          // to iterate over all the items of the transaction to compute the size of the selection rectangle
          // of course we start with the item after this one because it's size is already in the rectangle
          for (int i = startRow() + 1; i < startRow() + numRowsRegister(); ++i) {
            extraHeight += d->m_parent->visualRect(index.sibling(i, index.column())).height();
          }
        } else {
          // we are not editing in the register so all rows have the same sizes just compute the extra height
          extraHeight = (numRowsRegister() - 1) * option.rect.height();
        }
        option.rect.setBottom(option.rect.bottom() + extraHeight);
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, widget);
        if (d->m_focus && index.column() == (int)eTransaction::Column::Detail) {
          option.state |= QStyle::State_HasFocus;
          style->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, widget);
        }
        option.rect = old;
      }
    } else {
      if (d->m_alternate) {
        painter->fillRect(option.rect, option.palette.alternateBase());
      } else {
        painter->fillRect(option.rect, option.palette.base());
      }
    }

    // the text
    // construct the text for the cell
    QString txt;
    option.displayAlignment = Qt::AlignVCenter;
    if (!d->m_transaction.id().isEmpty() && !d->m_inRegisterEdit) {
      registerCellText(txt, option.displayAlignment, index.row() - startRow(), index.column(), painter);
    }

    if (Qt::mightBeRichText(txt)) {
      QTextDocument document;
      // this should set the alignment of the html but it does not work so htmls will be left aligned
      document.setDefaultTextOption(QTextOption(option.displayAlignment));
      document.setDocumentMargin(2);
      document.setHtml(txt);
      painter->translate(option.rect.topLeft());
      QAbstractTextDocumentLayout::PaintContext ctx;
      ctx.palette = option.palette;
      // Highlighting text if item is selected
      if (d->m_selected)
        ctx.palette.setColor(QPalette::Text, option.palette.color(QPalette::HighlightedText));
      document.documentLayout()->draw(painter, ctx);
      painter->translate(-option.rect.topLeft());
    } else {
      // draw plain text properly aligned
      style->drawItemText(painter, option.rect.adjusted(2, 0, -2, 0), option.displayAlignment, option.palette, true, txt, d->m_selected ? QPalette::HighlightedText : QPalette::Text);
    }

    // draw the grid if it's needed
    if (KMyMoneySettings::showGrid()) {
      const int gridHint = style->styleHint(QStyle::SH_Table_GridLineColor, &option, widget);
      const QPen gridPen = QPen(QColor(static_cast<QRgb>(gridHint)), 0);
      QPen old = painter->pen();
      painter->setPen(gridPen);
      if (index.row() == startRow())
        painter->drawLine(option.rect.topLeft(), option.rect.topRight());
      painter->drawLine(option.rect.topLeft(), option.rect.bottomLeft());
      painter->setPen(old);
    }

    // possible icons
    if (index.row() == startRow() && index.column() == (int)eTransaction::Column::Detail) {
      if (d->m_erroneous) {
        QPixmap attention;
        attention.loadFromData(attentionSign, sizeof(attentionSign), 0, 0);
        style->drawItemPixmap(painter, option.rect, Qt::AlignRight | Qt::AlignVCenter, attention);
      }
    }
  }
  painter->restore();
}

bool Transaction::formCellText(QString& /* txt */, Qt::Alignment& /* align */, int /* row */, int /* col */, QPainter* /* painter */)
{
  return false;
}

void Transaction::registerCellText(QString& /* txt */, Qt::Alignment& /* align */, int /* row */, int /* col */, QPainter* /* painter */)
{
}

int Transaction::registerColWidth(int /* col */, const QFontMetrics& /* cellFontMetrics */)
{
  return 0;
}

int Transaction::formRowHeight(int /*row*/)
{
  Q_D(Transaction);
  if (d->m_formRowHeight < 0) {
    d->m_formRowHeight = formRowHeight();
  }
  return d->m_formRowHeight;
}

int Transaction::formRowHeight() const
{
  Q_D(const Transaction);
  if (d->m_formRowHeight < 0) {
    // determine the height of the objects in the table
    KMyMoneyDateInput dateInput;
    KMyMoneyCategory category(true, nullptr);

    return qMax(dateInput.sizeHint().height(), category.sizeHint().height());
  }
  return d->m_formRowHeight;
}

void Transaction::setupForm(TransactionForm* form)
{
  Q_D(Transaction);
  d->m_form = form;
  form->verticalHeader()->setUpdatesEnabled(false);
  form->horizontalHeader()->setUpdatesEnabled(false);

  form->setRowCount(numRowsForm());
  form->setColumnCount(numColsForm());

  // Force all cells to have some text (so that paintCell is called for each cell)
  for (int r = 0; r < numRowsForm(); ++r) {
    for (int c = 0; c < numColsForm(); ++c) {
      if (r == 0 && form->columnWidth(c) == 0) {
        form->setColumnWidth(c, 10);
      }
    }
  }
  form->horizontalHeader()->setUpdatesEnabled(true);
  form->verticalHeader()->setUpdatesEnabled(true);

  loadTab(form);
}

void Transaction::paintFormCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)
{
  Q_D(Transaction);
  if (!d->m_form)
    return;

  QRect cellRect = option.rect;

  QRect textRect(cellRect);
  textRect.setWidth(textRect.width() - 2);
  textRect.setHeight(textRect.height() - 2);

  painter->setPen(option.palette.text().color());

  QString txt;
  Qt::Alignment align = Qt::AlignVCenter;
  bool editField = formCellText(txt, align, index.row(), index.column(), painter);

  // if we have an editable field and don't currently edit the transaction
  // show the background in a different color
  if (editField && !d->m_inEdit) {
    painter->fillRect(textRect, option.palette.alternateBase());
  }

  if (!d->m_inEdit)
    painter->drawText(textRect, align, txt);
}

void Transaction::setupPalette(const QPalette& palette, QMap<QString, QWidget*>& editWidgets)
{
  QMap<QString, QWidget*>::iterator it_w;
  for (it_w = editWidgets.begin(); it_w != editWidgets.end(); ++it_w) {
    if (*it_w) {
      (*it_w)->setPalette(palette);
    }
  }
}

void Transaction::setupFormPalette(QMap<QString, QWidget*>& editWidgets)
{
  Q_D(Transaction);
  QPalette palette = d->m_parent->palette();
  palette.setColor(QPalette::Active, QPalette::Base, palette.color(QPalette::Active, QPalette::Base));
  setupPalette(palette, editWidgets);
}

void Transaction::setupRegisterPalette(QMap<QString, QWidget*>& editWidgets)
{
  Q_D(Transaction);
  // make sure, we're using the right palette
  QPalette palette = d->m_parent->palette();

  // use the highlight color as background
  palette.setColor(QPalette::Active, QPalette::Background, palette.color(QPalette::Active, QPalette::Highlight));

  setupPalette(palette, editWidgets);
}

QWidget* Transaction::focusWidget(QWidget* w) const
{
  if (w) {
    while (w->focusProxy())
      w = w->focusProxy();
  }
  return w;
}

void Transaction::arrangeWidget(QTableWidget* tbl, int row, int col, QWidget* w) const
{
  if (w) {
    tbl->setCellWidget(row, col, w);
    // remove the widget from the QTable's eventFilter so that all
    // events will be directed to the edit widget
    w->removeEventFilter(tbl);
  }
}

bool Transaction::haveNumberField() const
{
  Q_D(const Transaction);
  auto rc = true;
  switch (d->m_account.accountType()) {
    case eMyMoney::Account::Type::Savings:
    case eMyMoney::Account::Type::Cash:
    case eMyMoney::Account::Type::Loan:
    case eMyMoney::Account::Type::AssetLoan:
    case eMyMoney::Account::Type::Asset:
    case eMyMoney::Account::Type::Liability:
    case eMyMoney::Account::Type::Equity:
      rc = KMyMoneySettings::alwaysShowNrField();
      break;

    case eMyMoney::Account::Type::Checkings:
    case eMyMoney::Account::Type::CreditCard:
      // the next case is used for the editor when the account
      // is unknown (eg. when creating new schedules)
    case eMyMoney::Account::Type::Unknown:
      break;

    default:
      rc = false;
      break;
  }
  return rc;
}

bool Transaction::maybeTip(const QPoint& cpos, int row, int col, QRect& r, QString& msg)
{
  Q_D(Transaction);
  if (col != (int)eTransaction::Column::Detail)
    return false;

  if (!d->m_erroneous && d->m_transaction.splitCount() < 3)
    return false;

  // check for detail column in row 0 of the transaction for a possible
  // exclamation mark. m_startRow is based 0, whereas the row to obtain
  // the modelindex is based 1, so we need to add one here
  r = d->m_parent->visualRect(d->m_parent->model()->index(d->m_startRow + 1, col));
  r.setBottom(r.bottom() + (numRowsRegister() - 1)*r.height());
  if (r.contains(cpos) && d->m_erroneous) {
    if (d->m_transaction.splits().count() < 2) {
      msg = QString("<qt>%1</qt>").arg(i18n("Transaction is missing a category assignment."));
    } else {
      const auto sec = MyMoneyFile::instance()->security(d->m_account.currencyId());
      msg = QString("<qt>%1</qt>").arg(i18n("The transaction has a missing assignment of <b>%1</b>.", MyMoneyUtils::formatMoney(d->m_transaction.splitSum().abs(), d->m_account, sec)));
    }
    return true;
  }

  // check if the mouse cursor is located on row 1 of the transaction
  // and display the details of a split transaction if it is one
  if (row == 1 && r.contains(cpos) && d->m_transaction.splitCount() > 2) {
    auto file = MyMoneyFile::instance();
    QString txt;
    const auto sec = file->security(d->m_transaction.commodity());
    MyMoneyMoney factor(1, 1);
    if (!d->m_split.value().isNegative())
      factor = -factor;

    foreach (const auto split, d->m_transaction.splits()) {
      if (split == d->m_split)
        continue;
      const MyMoneyAccount& acc = file->account(split.accountId());
      auto category = file->accountToCategory(acc.id());
      auto amount = MyMoneyUtils::formatMoney((split.value() * factor), acc, sec);

      txt += QString("<tr><td><nobr>%1</nobr></td><td align=right><nobr>%2</nobr></td></tr>").arg(category, amount);
    }
    msg = QString("<table>%1</table>").arg(txt);
    return true;
  }

  return false;
}

QString Transaction::reconcileState(bool text) const
{
  Q_D(const Transaction);
  auto txt = KMyMoneyUtils::reconcileStateToString(d->m_split.reconcileFlag(), text);

  if ((text == true)
      && (txt == i18nc("Unknown reconciliation state", "Unknown"))
      && (d->m_transaction.id().isEmpty()))
    txt.clear();
  return txt;
}

void Transaction::startEditMode()
{
  Q_D(Transaction);
  d->m_inEdit = true;

  // hide the original tabbar since the edit tabbar will be added
  if (auto form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*>(d->m_form))
    form->getTabBar()->setVisible(false);

  // only update the number of lines displayed if we edit inside the register
  if (d->m_inRegisterEdit)
    setNumRowsRegister(numRowsRegister(true));
}

int Transaction::numRowsRegister() const
{
  return RegisterItem::numRowsRegister();
}

void Transaction::leaveEditMode()
{
  Q_D(Transaction);
  // show the original tabbar since the edit tabbar was removed
  if (auto form = dynamic_cast<KMyMoneyTransactionForm::TransactionForm*>(d->m_form))
    form->getTabBar()->setVisible(true);

  // make sure we reset the row height of all the transaction's rows because it could have been changed during edit
  if (d->m_parent) {
    for (auto i = 0; i < numRowsRegister(); ++i)
      d->m_parent->setRowHeight(d->m_startRow + i, d->m_parent->rowHeightHint());
  }

  d->m_inEdit = false;
  d->m_inRegisterEdit = false;
  setFocus(hasFocus(), true);
}

void Transaction::singleLineMemo(QString& txt, const MyMoneySplit& split) const
{
  txt = split.memo();
  // remove empty lines
  txt.replace("\n\n", "\n");
  // replace '\n' with ", "
  txt.replace('\n', ", ");
}

int Transaction::rowHeightHint() const
{
  Q_D(const Transaction);
  return d->m_inEdit ? formRowHeight() : RegisterItem::rowHeightHint();
}

bool Transaction::matches(const RegisterFilter& filter) const
{
  Q_D(const Transaction);
  // check if the state matches
  if (!transaction().id().isEmpty()) {
    switch (filter.state) {
      default:
        break;
      case eRegister::ItemState::Imported:
        if (!transaction().isImported())
          return false;
        break;
      case eRegister::ItemState::Matched:
        if (!split().isMatched())
          return false;
        break;
      case eRegister::ItemState::Erroneous:
        if (transaction().splitSum().isZero())
          return false;
        break;
      case eRegister::ItemState::NotMarked:
        if (split().reconcileFlag() != eMyMoney::Split::State::NotReconciled)
          return false;
        break;
      case eRegister::ItemState::NotReconciled:
        if (split().reconcileFlag() != eMyMoney::Split::State::NotReconciled
            && split().reconcileFlag() != eMyMoney::Split::State::Cleared)
          return false;
        break;
      case eRegister::ItemState::Cleared:
        if (split().reconcileFlag() != eMyMoney::Split::State::Cleared)
          return false;
        break;
      case eRegister::ItemState::Scheduled:
        if (!isScheduled())
          return false;
        break;
    }
  }

  // check if the text matches
  if (filter.text.isEmpty() || d->m_transaction.splitCount() == 0)
    return true;

  auto file = MyMoneyFile::instance();

  foreach (const auto split, d->m_transaction.splits()) {
    // check if the text is contained in one of the fields
    // memo, number, payee, tag, account
    if (split.memo().contains(filter.text, Qt::CaseInsensitive)
        || split.number().contains(filter.text, Qt::CaseInsensitive))
      return true;

    if (!split.payeeId().isEmpty()) {
      const MyMoneyPayee& payee = file->payee(split.payeeId());
      if (payee.name().contains(filter.text, Qt::CaseInsensitive))
        return true;
    }
    if (!split.tagIdList().isEmpty()) {
      const QList<QString>& t = split.tagIdList();
      for (auto i = 0; i < t.count(); ++i) {
        if ((file->tag(t[i])).name().contains(filter.text, Qt::CaseInsensitive))
          return true;
      }
    }
    const MyMoneyAccount& acc = file->account(split.accountId());
    // search for account hierarchy
    if (filter.text.contains(MyMoneyFile::AccountSeparator)) {
      QStringList names;
      MyMoneyAccount current = acc;
      QString accountId;
      do {
        names.prepend(current.name());
        accountId = current.parentAccountId();
        current = file->account(accountId);
      } while (current.accountType() != eMyMoney::Account::Type::Unknown && !MyMoneyFile::instance()->isStandardAccount(accountId));
      if (names.size() > 1 && names.join(MyMoneyFile::AccountSeparator).contains(filter.text, Qt::CaseInsensitive))
        return true;
    }

    if (acc.name().contains(filter.text, Qt::CaseInsensitive))
      return true;

    QString s(filter.text);
    s.replace(MyMoneyMoney::thousandSeparator(), QChar());
    if (!s.isEmpty()) {
      // check if any of the value field matches if a value has been entered
      QString r = split.value().formatMoney(d->m_account.fraction(), false);
      if (r.contains(s, Qt::CaseInsensitive))
        return true;
      const MyMoneyAccount& splitAcc = file->account(split.accountId());
      r = split.shares().formatMoney(splitAcc.fraction(), false);
      if (r.contains(s, Qt::CaseInsensitive))
        return true;
    }
  }

  return false;
}

void Transaction::setShowBalance(bool showBalance)
{
  Q_D(Transaction);
  d->m_showBalance = showBalance;
}

bool Transaction::showRowInForm(int row) const
{
  Q_UNUSED(row) return true;
}

void Transaction::setShowRowInForm(int row, bool show)
{
  Q_UNUSED(row); Q_UNUSED(show)
}

void Transaction::setReducedIntensity(bool reduced)
{
  Q_D(Transaction);
  d->m_reducedIntensity = reduced;
}

void Transaction::setVisible(bool visible)
{
  Q_D(Transaction);
  if (visible != isVisible()) {
    RegisterItem::setVisible(visible);
    RegisterItem* p;
    Transaction* t;
    if (!visible) {
      // if we are hidden, we need to inform all previous transactions
      // about it so that they don't show the balance
      p = prevItem();
      while (p) {
        if ((t = dynamic_cast<Transaction*>(p))) {
          if (!t->d_func()->m_showBalance)
            break;
          t->d_func()->m_showBalance = false;
        }
        p = p->prevItem();
      }
    } else {
      // if we are shown, we need to check if the next transaction
      // is visible and change the display of the balance
      p = this;
      do {
        p = p->nextItem();
        t = dynamic_cast<Transaction*>(p);
      } while (!t && p);

      // if the next transaction is visible or I am the last one
      if ((t && t->d_func()->m_showBalance) || !t) {
        d->m_showBalance = true;
        p = prevItem();
        while (p && p->isVisible()) {
          if ((t = dynamic_cast<Transaction*>(p))) {
            if (t->d_func()->m_showBalance)
              break;
            t->d_func()->m_showBalance = true;
          }
          p = p->prevItem();
        }
      }
    }
  }
}

