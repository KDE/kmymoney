/***************************************************************************
                          kmymoneysplittable.cpp  -  description
                             -------------------
    begin                : Thu Jan 10 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include <qglobal.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qeventloop.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QKeyEvent>
#include <Q3ValueList>
#include <Q3Frame>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kcompletionbox.h>
#include <kpushbutton.h>
#include <kmenu.h>
#include <kstdaccel.h>
#include <kshortcut.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysplittable.h"
#include <mymoneyfile.h>
#include <kmymoneyedit.h>
#include <kmymoneycategory.h>
#include <kmymoneyaccountselector.h>
#include <kmymoneylineedit.h>
#include <mymoneysecurity.h>
#include <kmymoneyglobalsettings.h>

#include "kcurrencycalculator.h"

#include "mymoneyutils.h"

kMyMoneySplitTable::kMyMoneySplitTable(QWidget *parent, const char *name ) :
  Q3Table(parent,name),
  m_currentRow(0),
  m_maxRows(0),
  m_editMode(false),
  m_amountWidth(80),
  m_editCategory(0),
  m_editMemo(0),
  m_editAmount(0)
{
  // setup the transactions table
  setNumRows(1);
  setNumCols(3);
  horizontalHeader()->setLabel(0, i18n("Category"));
  horizontalHeader()->setLabel(1, i18n("Memo"));
  horizontalHeader()->setLabel(2, i18n("Amount"));
  setSelectionMode(Q3Table::NoSelection);
  setLeftMargin(0);
  verticalHeader()->hide();
  setColumnStretchable(0, false);
  setColumnStretchable(1, false);
  setColumnStretchable(2, false);
  horizontalHeader()->setResizeEnabled(false);
  horizontalHeader()->setMovingEnabled(false);
  horizontalHeader()->setFont(KMyMoneyGlobalSettings::listHeaderFont());

  setVScrollBarMode(Q3ScrollView::AlwaysOn);
  // never show a horizontal scroll bar
  setHScrollBarMode(Q3ScrollView::AlwaysOff);

  // setup the context menu
  m_contextMenu = new KMenu(this);
  KIconLoader *il = KIconLoader::global();
  m_contextMenu->setTitle(i18n("Split Options"));
  m_contextMenu->setIcon(il->loadIcon("transaction", KIconLoader::MainToolbar));
  m_contextMenu->insertItem(il->loadIcon("edit", KIconLoader::Small), i18n("Edit..."), this, SLOT(slotStartEdit()));
  m_contextMenuDuplicate = m_contextMenu->insertItem(il->loadIcon("editcopy", KIconLoader::Small), i18n("Duplicate"), this, SLOT(slotDuplicateSplit()));
  m_contextMenuDelete = m_contextMenu->insertItem(il->loadIcon("delete", KIconLoader::Small),
                        i18n("Delete ..."),
                        this, SLOT(slotDeleteSplit()));

  connect(this, SIGNAL(clicked(int, int, int, const QPoint&)),
    this, SLOT(slotSetFocus(int, int, int, const QPoint&)));

  connect(this, SIGNAL(transactionChanged(const MyMoneyTransaction&)),
    this, SLOT(slotUpdateData(const MyMoneyTransaction&)));
}

kMyMoneySplitTable::~kMyMoneySplitTable()
{
}

void kMyMoneySplitTable::setup(const QMap<QString, MyMoneyMoney>& priceInfo)
{
  m_priceInfo = priceInfo;
}

const QColor kMyMoneySplitTable::rowBackgroundColor(const int row) const
{
  return (row % 2) ? KMyMoneyGlobalSettings::listColor() : KMyMoneyGlobalSettings::listBGColor();
}

void kMyMoneySplitTable::paintCell(QPainter *p, int row, int col, const QRect& r, bool /*selected*/)
{
  QColorGroup g = colorGroup();
  QColor textColor;

  g.setColor(QColorGroup::Base, rowBackgroundColor(row));

  p->setFont(KMyMoneyGlobalSettings::listCellFont());

  QString firsttext = text(row, col);
  QString qstringCategory;
  QString qstringMemo;

  int intPos = firsttext.find("|");
  if(intPos > -1)
  {
    qstringCategory = firsttext.left(intPos);
    qstringMemo = firsttext.mid(intPos + 1);
  }

  QRect rr = r;
  QRect rr2 = r;
  rr.setX(0);
  rr.setY(0);
  rr.setWidth(columnWidth(col));
  rr.setHeight(rowHeight(row));

  rr2.setX(2);
  rr2.setY(0);
  rr2.setWidth(columnWidth(col)-4);
  rr2.setHeight(rowHeight(row));


  if(row == m_currentRow) {
    QBrush backgroundBrush(g.highlight());
    textColor = g.highlightedText();
    p->fillRect(rr,backgroundBrush);

  } else {
    QBrush backgroundBrush(g.base());
    textColor = g.text();
    p->fillRect(rr,backgroundBrush);
  }

  if (KMyMoneyGlobalSettings::showGrid()) {
    p->setPen(KMyMoneyGlobalSettings::listGridColor());
    if(col != 0)
      p->drawLine(rr.x(), 0, rr.x(), rr.height()-1);    // left frame
    p->drawLine(rr.x(), rr.y(), rr.width(), 0);         // bottom frame
    p->setPen(textColor);
  }

  switch (col) {
    case 0:     // category
    case 1:     // memo
      p->drawText(rr2, Qt::AlignLeft | Qt::AlignVCenter, text(row, col));
      break;

    case 2:     // amount
      p->drawText(rr2, Qt::AlignRight | Qt::AlignVCenter,firsttext);
      break;
  }
}

/** Override the QTable member function to avoid display of focus */
void kMyMoneySplitTable::paintFocus(QPainter * /* p */, const QRect & /*cr*/)
{
}

void kMyMoneySplitTable::columnWidthChanged(int col)
{
  for (int i=0; i<numRows(); i++)
    updateCell(i, col);
}

/** Override the QTable member function to avoid confusion with our own functionality */
void kMyMoneySplitTable::endEdit(int /*row*/, int /*col*/, bool /*accept*/, bool /*replace*/ )
{
}

bool kMyMoneySplitTable::eventFilter(QObject *o, QEvent *e)
{
  // MYMONEYTRACER(tracer);
  QKeyEvent *k = static_cast<QKeyEvent *> (e);
  bool rc = false;
  int row = currentRow();
  int lines = visibleHeight()/rowHeight(0);
  QWidget* w;

  if(e->type() == QEvent::KeyPress && !isEditMode()) {
    rc = true;
    switch(k->key()) {
      case Qt::Key_Up:
        if(row)
          slotSetFocus(row-1);
        break;

      case Qt::Key_Down:
        if(row < static_cast<int> (m_transaction.splits().count()-1))
          slotSetFocus(row+1);
        break;

      case Qt::Key_Home:
        slotSetFocus(0);
        break;

      case Qt::Key_End:
        slotSetFocus(m_transaction.splits().count()-1);
        break;

      case Qt::Key_PageUp:
        if(lines) {
          while(lines-- > 0 && row)
            row--;
          slotSetFocus(row);
        }
        break;

      case Qt::Key_PageDown:
        if(row < static_cast<int> (m_transaction.splits().count()-1)) {
          while(lines-- > 0 && row < static_cast<int> (m_transaction.splits().count()-1))
            row++;
          slotSetFocus(row);
        }
        break;

      case Qt::Key_Delete:
        slotDeleteSplit();
        break;

      case Qt::Key_Return:
      case Qt::Key_Enter:
        if(row < static_cast<int> (m_transaction.splits().count()-1)
        && KMyMoneyGlobalSettings::enterMovesBetweenFields()) {
          slotStartEdit();
        } else
          emit returnPressed();
        break;

      case Qt::Key_Escape:
        emit escapePressed();
        break;

      case Qt::Key_F2:
        slotStartEdit();
        break;

      default:
        rc = true;
        KShortcut copySplit(i18nc("Duplicate split", "Qt::CTRL+c"));
        KShortcut newSplit(QKeySequence(Qt::CTRL | Qt::Key_Insert));
#warning "port to kde4"
#if 0
        if(copySplit.contains(KKey(k))) {
          slotDuplicateSplit();

        } else if(newSplit.contains(KKey(k))) {
          slotSetFocus(m_transaction.splits().count()-1);
          slotStartEdit();

        } else if ( k->text()[ 0 ].isPrint() ) {
          w = slotStartEdit();
          // make sure, the widget receives the key again
          QApplication::sendEvent(w, e);
        }
#endif
        break;
    }

  } else if(e->type() == QEvent::KeyPress && isEditMode()) {
    bool terminate = true;
    rc = true;
    switch(k->key()) {
      // suppress the F2 functionality to start editing in inline edit mode
      case Qt::Key_F2:
      // suppress the cursor movement in inline edit mode
      case Qt::Key_Up:
      case Qt::Key_Down:
      case Qt::Key_PageUp:
      case Qt::Key_PageDown:
        break;

      case Qt::Key_Return:
      case Qt::Key_Enter:
        // we cannot call the slot directly, as it destroys the caller of
        // this method :-(  So we let the event handler take care of calling
        // the respective slot using a timeout. For a KLineEdit derived object
        // it could be, that at this point the user selected a value from
        // a completion list. In this case, we close the completion list and
        // do not end editing of the transaction.
        if(o->inherits("KLineEdit")) {
          KLineEdit* le = dynamic_cast<KLineEdit*> (o);
          KCompletionBox* box = le->completionBox(false);
          if(box && box->isVisible()) {
            terminate = false;
            le->completionBox(false)->hide();
          }
        }

        // in case we have the 'enter moves focus between fields', we need to simulate
        // a TAB key when the object 'o' points to the category or memo field.
        if(KMyMoneyGlobalSettings::enterMovesBetweenFields()) {
          if(o == m_editCategory->lineEdit() || o == m_editMemo) {
            terminate = false;
            QKeyEvent evt(e->type(),
                          Qt::Key_Tab, 0, k->state(), QString::null,
                          k->isAutoRepeat(), k->count());

            QApplication::sendEvent( o, &evt );
          }
        }

        if(terminate) {
          QTimer::singleShot(0, this, SLOT(slotEndEditKeyboard()));
        }
        break;

      case Qt::Key_Escape:
        // we cannot call the slot directly, as it destroys the caller of
        // this method :-(  So we let the event handler take care of calling
        // the respective slot using a timeout.
        QTimer::singleShot(0, this, SLOT(slotCancelEdit()));
        break;

      default:
        rc = false;
        break;
    }
  } else if(e->type() == QEvent::KeyRelease && !isEditMode()) {
    // for some reason, we only see a KeyRelease event of the Menu key
    // here. In other locations (e.g. Register::eventFilter()) we see
    // a KeyPress event. Strange. (ipwizard - 2008-05-10)
    switch(k->key()) {
      case Qt::Key_Menu:
        // if the very last entry is selected, the delete
        // operation is not available otherwise it is
        m_contextMenu->setItemEnabled(m_contextMenuDelete,
              row < static_cast<int> (m_transaction.splits().count()-1));
        m_contextMenu->setItemEnabled(m_contextMenuDuplicate,
              row < static_cast<int> (m_transaction.splits().count()-1));

        m_contextMenu->exec(QCursor::pos());
        rc = true;
        break;
      default:
        break;
    }
  }

  // if the event has not been processed here, forward it to
  // the base class implementation if it's not a key event
  if(rc == false) {
    if(e->type() != QEvent::KeyPress
    && e->type() != QEvent::KeyRelease) {
      rc = Q3Table::eventFilter(o, e);
    }
  }

  return rc;
}

void kMyMoneySplitTable::slotSetFocus(int realrow, int /* col */, int button, const QPoint& /* point */)
{
  MYMONEYTRACER(tracer);
  int   row = realrow;

  // adjust row to used area
  if(row > static_cast<int> (m_transaction.splits().count()-1))
    row = m_transaction.splits().count()-1;
  if(row < 0)
    row = 0;

  // make sure the row will be on the screen
  ensureCellVisible(row, 0);

  if(button == Qt::LeftButton) {          // left mouse button
    if(isEditMode()) {                    // in edit mode?
      if(KMyMoneyGlobalSettings::focusChangeIsEnter())
        slotEndEdit();
      else
        slotCancelEdit();
    }
    if(row != static_cast<int> (currentRow())) {
      // setup new current row and update visible selection
      setCurrentCell(row, 0);
      slotUpdateData(m_transaction);
    }
  } else if(button == Qt::RightButton) {
    // context menu is only available when cursor is on
    // an existing transaction or the first line after this area
    if(row == realrow) {
      // setup new current row and update visible selection
      setCurrentCell(row, 0);
      slotUpdateData(m_transaction);

      // if the very last entry is selected, the delete
      // operation is not available otherwise it is
      m_contextMenu->setItemEnabled(m_contextMenuDelete,
            row < static_cast<int> (m_transaction.splits().count()-1));
      m_contextMenu->setItemEnabled(m_contextMenuDuplicate,
            row < static_cast<int> (m_transaction.splits().count()-1));

      m_contextMenu->exec(QCursor::pos());
    }
  }
}

void kMyMoneySplitTable::contentsMousePressEvent( QMouseEvent* e )
{
  slotSetFocus( rowAt(e->pos().y()), columnAt(e->pos().x()), e->button(), e->pos() );
}

/* turn off QTable behaviour */
void kMyMoneySplitTable::contentsMouseReleaseEvent( QMouseEvent* /* e */ )
{
}

void kMyMoneySplitTable::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
  MYMONEYTRACER(tracer);

  int col = columnAt(e->pos().x());
  slotSetFocus( rowAt(e->pos().y()), col, e->button(), e->pos() );
  slotStartEdit();

  KLineEdit* editWidget = 0;
  switch(col) {
    case 1:
      editWidget = m_editMemo;
      break;

    case 2:
      editWidget = dynamic_cast<KLineEdit*> (m_editAmount->focusWidget());
      break;

    default:
      break;
  }
  if(editWidget) {
    editWidget->setFocus();
    editWidget->selectAll();
    // we need to call setFocus on the edit widget from the
    // main loop again to get the keyboard focus to the widget also
    QTimer::singleShot(0, editWidget, SLOT(setFocus()));
  }
}

void kMyMoneySplitTable::setCurrentCell(int row, int /* col */)
{
  MYMONEYTRACER(tracer);

  if(row > m_maxRows)
    row = m_maxRows;
  m_currentRow = row;
  Q3Table::setCurrentCell(row, 0);
  Q3ValueList<MyMoneySplit> list = getSplits(m_transaction);
  if(row < static_cast<int>(list.count()))
    m_split = list[row];
  else
    m_split = MyMoneySplit();
}

void kMyMoneySplitTable::setNumRows(int irows)
{
  Q3Table::setNumRows(irows);

  // determine row height according to the edit widgets
  // we use the category widget as the base
  QFontMetrics fm( KMyMoneyGlobalSettings::listCellFont() );
  int height = fm.lineSpacing()+6;
#if 0
  // recalculate row height hint
  KMyMoneyCategory cat;
  height = qMax(cat.sizeHint().height(), height);
#endif

  verticalHeader()->setUpdatesEnabled(false);

  for(int i = 0; i < irows; ++i)
    verticalHeader()->resizeSection(i, height);

  verticalHeader()->setUpdatesEnabled(true);

  // add or remove scrollbars as required
  updateScrollBars();
}

void kMyMoneySplitTable::setTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s, const MyMoneyAccount& acc)
{
  MYMONEYTRACER(tracer);
  m_transaction = t;
  m_account = acc;
  m_hiddenSplit = s;
  setCurrentCell(0, 0);
  slotUpdateData(m_transaction);
}

const Q3ValueList<MyMoneySplit> kMyMoneySplitTable::getSplits(const MyMoneyTransaction& t) const
{
  Q3ValueList<MyMoneySplit> list;
  Q3ValueList<MyMoneySplit>::Iterator it;

  // get list of splits
  list = t.splits();

  // and ignore the one that should be hidden

  for(it = list.begin(); it != list.end(); ++it) {
    if((*it).id() == m_hiddenSplit.id()) {
      list.remove(it);
      break;
    }
  }
  return list;
}

void kMyMoneySplitTable::slotUpdateData(const MyMoneyTransaction& t)
{
  MYMONEYTRACER(tracer);
  unsigned long rowCount=0;

  Q3ValueList<MyMoneySplit> list = getSplits(t);
  updateTransactionTableSize();

  // fill the part that is used by transactions
  Q3ValueList<MyMoneySplit>::Iterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    QString colText;
    MyMoneyMoney value = (*it).value();
    if(!(*it).accountId().isEmpty()) {
      try {
        colText = MyMoneyFile::instance()->accountToCategory((*it).accountId());
      } catch(MyMoneyException *e) {
        qDebug("Unexpected exception in kMyMoneySplitTable::slotUpdateData()");
        delete e;
      }
    }
    QString amountTxt = value.formatMoney(m_account.fraction());
    if(value == MyMoneyMoney::autoCalc) {
      amountTxt = i18n("will be calculated");
    }

    if(colText.isEmpty() && (*it).memo().isEmpty() && value.isZero())
      amountTxt = QString();

    unsigned width = fontMetrics().width(amountTxt);
    kMyMoneyEdit* valfield = new kMyMoneyEdit();
    valfield->setMinimumWidth(width);
    width = valfield->minimumSizeHint().width();
    delete valfield;

    if(width > m_amountWidth)
      m_amountWidth = width;

    setText(rowCount, 0, colText);
    setText(rowCount, 1, (*it).memo());
    setText(rowCount, 2, amountTxt);

    rowCount++;
  }

  // now clean out the remainder of the table
  while(rowCount < static_cast<unsigned long> (numRows())) {
    setText(rowCount, 0, "");
    setText(rowCount, 1, "");
    setText(rowCount, 2, "");
    ++rowCount;
  }
}

void kMyMoneySplitTable::updateTransactionTableSize(void)
{
  // get current size of transactions table
  int rowHeight = cellGeometry(0, 0).height();
  int tableHeight = height();
  int splitCount = m_transaction.splits().count()-1;

  if(splitCount < 0)
    splitCount = 0;

  // see if we need some extra lines to fill the current size with the grid
  int numExtraLines = (tableHeight / rowHeight) - splitCount;
  if(numExtraLines < 2)
    numExtraLines = 2;

  setNumRows(splitCount + numExtraLines);
  // setMaxRows(splitCount);
  m_maxRows = splitCount;
}

void kMyMoneySplitTable::resizeEvent(QResizeEvent* /* ev */)
{
  int w = visibleWidth() - m_amountWidth;

  // resize the columns
  setColumnWidth(0, w/2);
  setColumnWidth(1, w/2);
  setColumnWidth(2, m_amountWidth);

  updateTransactionTableSize();
}

void kMyMoneySplitTable::slotDuplicateSplit(void)
{
  MYMONEYTRACER(tracer);
  Q3ValueList<MyMoneySplit> list = getSplits(m_transaction);
  if(m_currentRow < static_cast<int> (list.count())) {
    MyMoneySplit split = list[m_currentRow];
    split.clearId();
    try {
      m_transaction.addSplit(split);
      emit transactionChanged(m_transaction);
    } catch(MyMoneyException *e) {
      qDebug("Cannot duplicate split: %s", e->what().toLatin1());
      delete e;
    }
  }
}

void kMyMoneySplitTable::slotDeleteSplit(void)
{
  MYMONEYTRACER(tracer);
  Q3ValueList<MyMoneySplit> list = getSplits(m_transaction);
  if(m_currentRow < static_cast<int> (list.count())) {
    if(KMessageBox::warningContinueCancel (this,
        i18n("You are about to delete the selected split. "
            "Do you really want to continue?"),
        i18n("KMyMoney"),
        KGuiItem( i18n("Continue") )
        ) == KMessageBox::Continue) {
      try {
        m_transaction.removeSplit(list[m_currentRow]);
        // if we removed the last split, select the previous
        if(m_currentRow && m_currentRow == static_cast<int>(list.count())-1)
          setCurrentCell(m_currentRow-1, 0);
        else
          setCurrentCell(m_currentRow, 0);
        emit transactionChanged(m_transaction);
      } catch(MyMoneyException *e) {
        qDebug("Cannot remove split: %s", e->what().toLatin1());
        delete e;
      }
    }
  }
}

QWidget* kMyMoneySplitTable::slotStartEdit(void)
{
  MYMONEYTRACER(tracer);
  return createEditWidgets();
}

void kMyMoneySplitTable::slotEndEdit(void)
{
  endEdit(false);
}

void kMyMoneySplitTable::slotEndEditKeyboard(void)
{
  endEdit(true);
}

void kMyMoneySplitTable::endEdit(bool keyBoardDriven)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MYMONEYTRACER(tracer);
  MyMoneySplit s1 = m_split;

  bool needUpdate = false;
  if(m_editCategory->selectedItem() != m_split.accountId()) {
    s1.setAccountId(m_editCategory->selectedItem());
    needUpdate = true;
  }
  if(m_editMemo->text() != m_split.memo()) {
    s1.setMemo(m_editMemo->text());
    needUpdate = true;
  }
  if(m_editAmount->value() != m_split.value()) {
    s1.setValue(m_editAmount->value());
    needUpdate = true;
  }

  if(needUpdate) {
    if(!s1.value().isZero()) {
      MyMoneyAccount cat = file->account(s1.accountId());
      if(cat.currencyId() != m_transaction.commodity()) {

        MyMoneySecurity fromCurrency, toCurrency;
        MyMoneyMoney fromValue, toValue;
        fromCurrency = file->security(m_transaction.commodity());
        toCurrency = file->security(cat.currencyId());

        // determine the fraction required for this category
        int fract = toCurrency.smallestAccountFraction();
        if(cat.accountType() == MyMoneyAccount::Cash)
          fract = toCurrency.smallestCashFraction();

        // display only positive values to the user
        fromValue = s1.value().abs();

        // if we had a price info in the beginning, we use it here
        if(m_priceInfo.find(cat.currencyId()) != m_priceInfo.end()) {
          toValue = (fromValue * m_priceInfo[cat.currencyId()]).convert(fract);
        }

        // if the shares are still 0, we need to change that
        if(toValue.isZero()) {
          MyMoneyPrice price = MyMoneyFile::instance()->price(fromCurrency.id(), toCurrency.id());
          // if the price is valid calculate the shares. If it is invalid
          // assume a conversion rate of 1.0
          if(price.isValid()) {
            toValue = (price.rate(toCurrency.id()) * fromValue).convert(fract);
          } else {
            toValue = fromValue;
          }
        }

        // now present all that to the user
        KCurrencyCalculator calc(fromCurrency,
                                toCurrency,
                                fromValue,
                                toValue,
                                m_transaction.postDate(),
                                fract,
                                this, "currencyCalculator");

        if(calc.exec() == QDialog::Rejected) {
          return;
        } else {
          s1.setShares((s1.value() * calc.price()).convert(fract));
        }

      } else {
        s1.setShares(s1.value());
      }
    } else
      s1.setShares(s1.value());

    m_split = s1;
    try {
      if(m_split.id().isEmpty()) {
        m_transaction.addSplit(m_split);
      } else {
        m_transaction.modifySplit(m_split);
      }
      emit transactionChanged(m_transaction);
    } catch(MyMoneyException *e) {
      qDebug("Cannot add/modify split: %s", e->what().toLatin1());
      delete e;
    }
  }
  this->setFocus();
  destroyEditWidgets();
  slotSetFocus(currentRow()+1);

  // if we still have more splits, we start editing right away
  // in case we have selected 'enter moves betweeen fields'
  if(keyBoardDriven
  && currentRow() < static_cast<int> (m_transaction.splits().count()-1)
  && KMyMoneyGlobalSettings::enterMovesBetweenFields()) {
    slotStartEdit();
  }

}

void kMyMoneySplitTable::slotCancelEdit(void)
{
  MYMONEYTRACER(tracer);
  if(isEditMode()) {
    destroyEditWidgets();
    this->setFocus();
  }
}

bool kMyMoneySplitTable::isEditMode(void) const
{
  return m_editMode;
}

void kMyMoneySplitTable::destroyEditWidgets(void)
{
  MYMONEYTRACER(tracer);

  disconnect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadEditWidgets()));

  clearCellWidget(m_currentRow, 0);
  clearCellWidget(m_currentRow, 1);
  clearCellWidget(m_currentRow, 2);
  clearCellWidget(m_currentRow+1, 0);
  m_editMode = false;
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInput, 100);
}

QWidget* kMyMoneySplitTable::createEditWidgets(void)
{
  MYMONEYTRACER(tracer);

  QFont cellFont = KMyMoneyGlobalSettings::listCellFont();
  m_tabOrderWidgets.clear();

  // create the widgets
  m_editAmount = new kMyMoneyEdit(0);
  m_editAmount->setFont(cellFont);
  m_editAmount->setResetButtonVisible(false);

  m_editCategory = new KMyMoneyCategory();
  m_editCategory->setHint(i18n("Category"));
  m_editCategory->setFont(cellFont);
  connect(m_editCategory, SIGNAL(createItem(const QString&, QString&)), this, SIGNAL(createCategory(const QString&, QString&)));
  connect(m_editCategory, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  m_editMemo = new kMyMoneyLineEdit(0, 0, false, Qt::AlignLeft|Qt::AlignVCenter);
  m_editMemo->setHint(i18n("Memo"));
  m_editMemo->setFont(cellFont);

  // create buttons for the mouse users
  KIconLoader *il = KIconLoader::global();
  m_registerButtonFrame = new Q3Frame(this, "buttonFrame");
  QPalette palette = m_registerButtonFrame->palette();
  palette.setColor(QColorGroup::Background, rowBackgroundColor(m_currentRow+1) );
  m_registerButtonFrame->setPalette(palette);

  Q3HBoxLayout* l = new Q3HBoxLayout(m_registerButtonFrame);
  m_registerEnterButton = new KPushButton(KIcon( il->loadIcon("button_ok", KIconLoader::Small, KIconLoader::SizeSmall) ), QString(), m_registerButtonFrame);

  m_registerCancelButton = new KPushButton(KIcon( il->loadIcon("button_cancel", KIconLoader::Small, KIconLoader::SizeSmall) ), QString(), m_registerButtonFrame);

  l->addWidget(m_registerEnterButton);
  l->addWidget(m_registerCancelButton);
  l->addStretch(2);

  connect(m_registerEnterButton, SIGNAL(clicked()), this, SLOT(slotEndEdit()));
  connect(m_registerCancelButton, SIGNAL(clicked()), this, SLOT(slotCancelEdit()));

  // setup tab order
  addToTabOrder(m_editCategory);
  addToTabOrder(m_editMemo);
  addToTabOrder(m_editAmount);
  addToTabOrder(m_registerEnterButton);
  addToTabOrder(m_registerCancelButton);

  if(!m_split.accountId().isEmpty()) {
    m_editCategory->setSelectedItem(m_split.accountId());
  } else {
    // check if the transaction is balanced or not. If not,
    // assign the remainder to the amount.
    MyMoneyMoney diff;
    Q3ValueList<MyMoneySplit> list = m_transaction.splits();
    Q3ValueList<MyMoneySplit>::ConstIterator it_s;
    for(it_s = list.begin(); it_s != list.end(); ++it_s) {
      if(!(*it_s).accountId().isEmpty())
        diff += (*it_s).value();
    }
    m_split.setValue(-diff);
  }

  m_editMemo->loadText(m_split.memo());
  // don't allow automatically calculated values to be modified
  if(m_split.value() == MyMoneyMoney::autoCalc) {
    m_editAmount->setEnabled(false);
    m_editAmount->loadText("will be calculated");
  } else
    m_editAmount->setValue(m_split.value());

  setCellWidget(m_currentRow, 0, m_editCategory);
  setCellWidget(m_currentRow, 1, m_editMemo);
  setCellWidget(m_currentRow, 2, m_editAmount);
  setCellWidget(m_currentRow+1, 0, m_registerButtonFrame);

  // load e.g. the category widget with the account list
  slotLoadEditWidgets();
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadEditWidgets()));
#warning "port to kde4"
#if 0
  // setup the keyboard filter for all widgets
  for(QWidget* w = m_tabOrderWidgets.first(); w; w = m_tabOrderWidgets.next()) {
    w->installEventFilter(this);
  }
#endif
  m_editCategory->setFocus();
  m_editCategory->lineEdit()->selectAll();
  m_editMode = true;

  return m_editCategory->lineEdit();
}

void kMyMoneySplitTable::slotLoadEditWidgets(void)
{
  // reload category widget
  QString categoryId = m_editCategory->selectedItem();

  AccountSet aSet;
  aSet.addAccountGroup(MyMoneyAccount::Asset);
  aSet.addAccountGroup(MyMoneyAccount::Liability);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  if(KMyMoneyGlobalSettings::expertMode())
    aSet.addAccountGroup(MyMoneyAccount::Equity);

  // remove the accounts with invalid types at this point
  aSet.removeAccountType(MyMoneyAccount::CertificateDep);
  aSet.removeAccountType(MyMoneyAccount::Investment);
  aSet.removeAccountType(MyMoneyAccount::Stock);
  aSet.removeAccountType(MyMoneyAccount::MoneyMarket);

  aSet.load(m_editCategory->selector());

  // if an account is specified then remove it from the widget so that the user
  // cannot create a transfer with from and to account being the same account
  if(!m_account.id().isEmpty())
    m_editCategory->selector()->removeItem(m_account.id());

  if(!categoryId.isEmpty())
    m_editCategory->setSelectedItem(categoryId);

}

void kMyMoneySplitTable::addToTabOrder(QWidget* w)
{
  if(w) {
    while(w->focusProxy())
      w = w->focusProxy();
    m_tabOrderWidgets.append(w);
  }
}

bool kMyMoneySplitTable::focusNextPrevChild(bool next)
{
  MYMONEYTRACER(tracer);
  bool  rc = false;
#warning "port to kde4"
#if 0
  if(m_editCategory) {
    QWidget *w = 0;
    QWidget *currentWidget;

    m_tabOrderWidgets.find(qApp->focusWidget());
    currentWidget = m_tabOrderWidgets.current();
    w = next ? m_tabOrderWidgets.next() : m_tabOrderWidgets.prev();

    do {
      if(!w) {
        w = next ? m_tabOrderWidgets.first() : m_tabOrderWidgets.last();
      }

      if(w != currentWidget
      && ((w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus)
      && w->isVisible() && w->isEnabled()) {
        w->setFocus();
        rc = true;
        break;
      }
      w = next ? m_tabOrderWidgets.next() : m_tabOrderWidgets.prev();
    } while(w != currentWidget);

  } else
    rc = Q3Table::focusNextPrevChild(next);
#endif
  return rc;
}



#include "kmymoneysplittable.moc"
