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

#include "kmymoneysplittable.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QGlobalStatic>
#include <QPainter>
#include <QCursor>
#include <QApplication>
#include <QTimer>
#include <QLayout>
#include <QEventLoop>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QFrame>
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
#include <kshortcut.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "kmymoneyedit.h"
#include "kmymoneycategory.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneylineedit.h"
#include "mymoneysecurity.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoneymvccombo.h"

#include "kcurrencycalculator.h"

#include "mymoneyutils.h"

kMyMoneySplitTable::kMyMoneySplitTable(QWidget *parent) :
    QTableWidget(parent),
    m_currentRow(0),
    m_maxRows(0),
    m_precision(2),
    m_editCategory(0),
    m_editMemo(0),
    m_editTag(0),
    m_editAmount(0)
{
  // used for custom coloring with the help of the application's stylesheet
  setObjectName(QLatin1String("splittable"));

  // setup the transactions table
  setRowCount(1);
  setColumnCount(4);
  QStringList labels;
  labels << i18n("Category") << i18n("Memo") << i18n("Tag") << i18n("Amount");
  setHorizontalHeaderLabels(labels);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  int left, top, right, bottom;
  getContentsMargins(&left, &top, &right, &bottom);
  setContentsMargins(0, top, right, bottom);

  setFont(KMyMoneyGlobalSettings::listCellFont());

  setAlternatingRowColors(true);

  verticalHeader()->hide();
  horizontalHeader()->setMovable(false);
  horizontalHeader()->setFont(KMyMoneyGlobalSettings::listHeaderFont());

  KConfigGroup grp = KGlobal::config()->group("SplitTable");
  QByteArray columns;
  columns = grp.readEntry("HeaderState", columns);
  horizontalHeader()->restoreState(columns);
  horizontalHeader()->setStretchLastSection(true);

  setShowGrid(KMyMoneyGlobalSettings::showGrid());

  setEditTriggers(QAbstractItemView::NoEditTriggers);

  // setup the context menu
  m_contextMenu = new KMenu(this);
  m_contextMenu->setTitle(i18n("Split Options"));
  m_contextMenu->setIcon(KIcon("view-financial-transfer"));
  m_contextMenu->addAction(KIcon("document-edit"), i18n("Edit..."), this, SLOT(slotStartEdit()));
  m_contextMenuDuplicate = m_contextMenu->addAction(KIcon("edit-copy"), i18nc("To duplicate a split", "Duplicate"), this, SLOT(slotDuplicateSplit()));
  m_contextMenuDelete = m_contextMenu->addAction(KIcon("edit-delete"),
                        i18n("Delete..."),
                        this, SLOT(slotDeleteSplit()));

  connect(this, SIGNAL(clicked(QModelIndex)),
          this, SLOT(slotSetFocus(QModelIndex)));

  connect(this, SIGNAL(transactionChanged(MyMoneyTransaction)),
          this, SLOT(slotUpdateData(MyMoneyTransaction)));

  installEventFilter(this);
}

kMyMoneySplitTable::~kMyMoneySplitTable()
{
  KConfigGroup grp = KGlobal::config()->group("SplitTable");
  QByteArray columns = horizontalHeader()->saveState();
  grp.writeEntry("HeaderState", columns);
  grp.sync();
}

int kMyMoneySplitTable::currentRow() const
{
  return m_currentRow;
}

void kMyMoneySplitTable::setup(const QMap<QString, MyMoneyMoney>& priceInfo, int precision)
{
  m_priceInfo = priceInfo;
  m_precision = precision;
}

bool kMyMoneySplitTable::eventFilter(QObject *o, QEvent *e)
{
  // MYMONEYTRACER(tracer);
  QKeyEvent *k = static_cast<QKeyEvent *>(e);
  bool rc = false;
  int row = currentRow();
  int lines = viewport()->height() / rowHeight(0);

  if (e->type() == QEvent::KeyPress && !isEditMode()) {
    rc = true;
    switch (k->key()) {
      case Qt::Key_Up:
        if (row)
          slotSetFocus(model()->index(row - 1, 0));
        break;

      case Qt::Key_Down:
        if (row < m_transaction.splits().count() - 1)
          slotSetFocus(model()->index(row + 1, 0));
        break;

      case Qt::Key_Home:
        slotSetFocus(model()->index(0, 0));
        break;

      case Qt::Key_End:
        slotSetFocus(model()->index(m_transaction.splits().count() - 1, 0));
        break;

      case Qt::Key_PageUp:
        if (lines) {
          while (lines-- > 0 && row)
            --row;
          slotSetFocus(model()->index(row, 0));
        }
        break;

      case Qt::Key_PageDown:
        if (row < m_transaction.splits().count() - 1) {
          while (lines-- > 0 && row < m_transaction.splits().count() - 1)
            ++row;
          slotSetFocus(model()->index(row, 0));
        }
        break;

      case Qt::Key_Delete:
        slotDeleteSplit();
        break;

      case Qt::Key_Return:
      case Qt::Key_Enter:
        if (row < m_transaction.splits().count() - 1
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

        // duplicate split
        if (Qt::Key_C == k->key() && Qt::ControlModifier == k->modifiers()) {
          slotDuplicateSplit();

          // new split
        } else if (Qt::Key_Insert == k->key() && Qt::ControlModifier == k->modifiers()) {
          slotSetFocus(model()->index(m_transaction.splits().count() - 1, 0));
          slotStartEdit();

        } else if (k->text()[ 0 ].isPrint()) {
          KMyMoneyCategory* cat = createEditWidgets(false);
          if (cat) {
            kMyMoneyLineEdit *le = qobject_cast<kMyMoneyLineEdit*>(cat->lineEdit());
            if (le) {
              // make sure, the widget receives the key again
              // and does not select the text this time
              le->setText(k->text());
              le->end(false);
              le->deselect();
              le->skipSelectAll(true);
              le->setFocus();
            }
          }
        }
        break;
    }

  } else if (e->type() == QEvent::KeyPress && isEditMode()) {
    bool terminate = true;
    rc = true;
    switch (k->key()) {
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
        if (o->inherits("KLineEdit")) {
          KLineEdit* le = dynamic_cast<KLineEdit*>(o);
          KCompletionBox* box = le->completionBox(false);
          if (box && box->isVisible()) {
            terminate = false;
            le->completionBox(false)->hide();
          }
        }

        // in case we have the 'enter moves focus between fields', we need to simulate
        // a TAB key when the object 'o' points to the category or memo field.
        if (KMyMoneyGlobalSettings::enterMovesBetweenFields()) {
          if (o == m_editCategory->lineEdit() || o == m_editMemo || o == m_editTag) {
            terminate = false;
            QKeyEvent evt(e->type(),
                          Qt::Key_Tab, k->modifiers(), QString(),
                          k->isAutoRepeat(), k->count());

            QApplication::sendEvent(o, &evt);
          }
        }

        if (terminate) {
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
  } else if (e->type() == QEvent::KeyRelease && !isEditMode()) {
    // for some reason, we only see a KeyRelease event of the Menu key
    // here. In other locations (e.g. Register::eventFilter()) we see
    // a KeyPress event. Strange. (ipwizard - 2008-05-10)
    switch (k->key()) {
      case Qt::Key_Menu:
        // if the very last entry is selected, the delete
        // operation is not available otherwise it is
        m_contextMenuDelete->setEnabled(
          row < m_transaction.splits().count() - 1);
        m_contextMenuDuplicate->setEnabled(
          row < m_transaction.splits().count() - 1);

        m_contextMenu->exec(QCursor::pos());
        rc = true;
        break;
      default:
        break;
    }
  }

  // if the event has not been processed here, forward it to
  // the base class implementation if it's not a key event
  if (rc == false) {
    if (e->type() != QEvent::KeyPress
        && e->type() != QEvent::KeyRelease) {
      rc = QTableWidget::eventFilter(o, e);
    }
  }

  return rc;
}

void kMyMoneySplitTable::slotSetFocus(const QModelIndex& index, int button)
{
  MYMONEYTRACER(tracer);
  int   row = index.row();

  // adjust row to used area
  if (row > m_transaction.splits().count() - 1)
    row = m_transaction.splits().count() - 1;
  if (row < 0)
    row = 0;

  // make sure the row will be on the screen
  scrollTo(model()->index(row, 0));

  if (isEditMode()) {                   // in edit mode?
    if (isEditSplitValid() && KMyMoneyGlobalSettings::focusChangeIsEnter())
      endEdit(false/*keyboard driven*/, false/*set focus to next row*/);
    else
      slotCancelEdit();
  }

  if (button == Qt::LeftButton) {         // left mouse button
    if (row != currentRow()) {
      // setup new current row and update visible selection
      selectRow(row);
      slotUpdateData(m_transaction);
    }
  } else if (button == Qt::RightButton) {
    // context menu is only available when cursor is on
    // an existing transaction or the first line after this area
    if (row == index.row()) {
      // setup new current row and update visible selection
      selectRow(row);
      slotUpdateData(m_transaction);

      // if the very last entry is selected, the delete
      // operation is not available otherwise it is
      m_contextMenuDelete->setEnabled(
        row < m_transaction.splits().count() - 1);
      m_contextMenuDuplicate->setEnabled(
        row < m_transaction.splits().count() - 1);

      m_contextMenu->exec(QCursor::pos());
    }
  }
}

void kMyMoneySplitTable::mousePressEvent(QMouseEvent* e)
{
  slotSetFocus(indexAt(e->pos()), e->button());
}

/* turn off QTable behaviour */
void kMyMoneySplitTable::mouseReleaseEvent(QMouseEvent* /* e */)
{
}

void kMyMoneySplitTable::mouseDoubleClickEvent(QMouseEvent *e)
{
  MYMONEYTRACER(tracer);

  int col = columnAt(e->pos().x());
  slotSetFocus(model()->index(rowAt(e->pos().y()), col), e->button());
  createEditWidgets(false);

  QLineEdit* editWidget = 0;    //krazy:exclude=qmethods
  switch (col) {
    case 0:
      editWidget = m_editCategory->lineEdit();
      break;

    case 1:
      editWidget = m_editMemo;
      break;

    case 2:
      m_editTag->tagCombo()->setFocus();
      break;

    case 3:
      editWidget = m_editAmount->lineedit();
      break;

    default:
      break;
  }
  if (editWidget) {
    editWidget->setFocus();
    editWidget->selectAll();
  }
}

void kMyMoneySplitTable::selectRow(int row)
{
  MYMONEYTRACER(tracer);

  if (row > m_maxRows)
    row = m_maxRows;
  m_currentRow = row;
  QTableWidget::selectRow(row);
  QList<MyMoneySplit> list = getSplits(m_transaction);
  if (row < list.count())
    m_split = list[row];
  else
    m_split = MyMoneySplit();
}

void kMyMoneySplitTable::setRowCount(int irows)
{
  QTableWidget::setRowCount(irows);

  // determine row height according to the edit widgets
  // we use the category widget as the base
  QFontMetrics fm(KMyMoneyGlobalSettings::listCellFont());
  int height = fm.lineSpacing() + 6;
#if 0
  // recalculate row height hint
  KMyMoneyCategory cat;
  height = qMax(cat.sizeHint().height(), height);
#endif

  verticalHeader()->setUpdatesEnabled(false);

  for (int i = 0; i < irows; ++i)
    verticalHeader()->resizeSection(i, height);

  verticalHeader()->setUpdatesEnabled(true);
}

void kMyMoneySplitTable::setTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s, const MyMoneyAccount& acc)
{
  MYMONEYTRACER(tracer);
  m_transaction = t;
  m_account = acc;
  m_hiddenSplit = s;
  selectRow(0);
  slotUpdateData(m_transaction);
}

const QList<MyMoneySplit> kMyMoneySplitTable::getSplits(const MyMoneyTransaction& t) const
{
  // get list of splits
  QList<MyMoneySplit> list = t.splits();

  // and ignore the one that should be hidden
  QList<MyMoneySplit>::Iterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    if ((*it).id() == m_hiddenSplit.id()) {
      list.erase(it);
      break;
    }
  }
  return list;
}

void kMyMoneySplitTable::slotUpdateData(const MyMoneyTransaction& t)
{
  MYMONEYTRACER(tracer);
  unsigned long numRows = 0;
  QTableWidgetItem* textItem;

  QList<MyMoneySplit> list = getSplits(t);
  updateTransactionTableSize();

  // fill the part that is used by transactions
  QList<MyMoneySplit>::Iterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    QString colText;
    MyMoneyMoney value = (*it).value();
    if (!(*it).accountId().isEmpty()) {
      try {
        colText = MyMoneyFile::instance()->accountToCategory((*it).accountId());
      } catch (const MyMoneyException &) {
        qDebug("Unexpected exception in kMyMoneySplitTable::slotUpdateData()");
      }
    }
    QString amountTxt = value.formatMoney(m_account.fraction());
    if (value == MyMoneyMoney::autoCalc) {
      amountTxt = i18n("will be calculated");
    }

    if (colText.isEmpty() && (*it).memo().isEmpty() && value.isZero())
      amountTxt.clear();

    unsigned width = fontMetrics().width(amountTxt);
    kMyMoneyEdit* valfield = new kMyMoneyEdit();
    valfield->setMinimumWidth(width);
    width = valfield->minimumSizeHint().width();
    delete valfield;

    textItem = item(numRows, 0);
    if (textItem)
      textItem->setText(colText);
    else
      setItem(numRows, 0, new QTableWidgetItem(colText));

    textItem = item(numRows, 1);
    if (textItem)
      textItem->setText((*it).memo());
    else
      setItem(numRows, 1, new QTableWidgetItem((*it).memo()));

    QList<QString> t = (*it).tagIdList();
    QStringList tagNames;
    if (!t.isEmpty()) {
      for (int i = 0; i < t.size(); i++)
        tagNames.append(MyMoneyFile::instance()->tag(t[i]).name());
    }
    setItem(numRows, 2, new QTableWidgetItem(tagNames.join(", ")));

    textItem = item(numRows, 3);
    if (textItem)
      textItem->setText(amountTxt);
    else
      setItem(numRows, 3, new QTableWidgetItem(amountTxt));

    item(numRows, 3)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    ++numRows;
  }

  // now clean out the remainder of the table
  while (numRows < static_cast<unsigned long>(rowCount())) {
    for (int i = 0 ; i < 4; ++i) {
      textItem = item(numRows, i);
      if (textItem)
        textItem->setText("");
      else
        setItem(numRows, i, new QTableWidgetItem(""));
    }
    item(numRows, 3)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ++numRows;
  }
}

void kMyMoneySplitTable::updateTransactionTableSize()
{
  // get current size of transactions table
  int tableHeight = height();
  int splitCount = m_transaction.splits().count() - 1;

  if (splitCount < 0)
    splitCount = 0;

  // see if we need some extra lines to fill the current size with the grid
  int numExtraLines = (tableHeight / rowHeight(0)) - splitCount;
  if (numExtraLines < 2)
    numExtraLines = 2;

  setRowCount(splitCount + numExtraLines);
  m_maxRows = splitCount;
}

void kMyMoneySplitTable::resizeEvent(QResizeEvent* ev)
{
  QTableWidget::resizeEvent(ev);
  if (!isEditMode()) {
    // update the size of the transaction table only if a split is not being edited
    // otherwise the height of the editors would be altered in an undesired way
    updateTransactionTableSize();
  }
}

void kMyMoneySplitTable::slotDuplicateSplit()
{
  MYMONEYTRACER(tracer);
  QList<MyMoneySplit> list = getSplits(m_transaction);
  if (m_currentRow < list.count()) {
    MyMoneySplit split = list[m_currentRow];
    split.clearId();
    try {
      m_transaction.addSplit(split);
      emit transactionChanged(m_transaction);
    } catch (const MyMoneyException &e) {
      qDebug("Cannot duplicate split: %s", qPrintable(e.what()));
    }
  }
}

void kMyMoneySplitTable::slotDeleteSplit()
{
  MYMONEYTRACER(tracer);
  QList<MyMoneySplit> list = getSplits(m_transaction);
  if (m_currentRow < list.count()) {
    if (KMessageBox::warningContinueCancel(this,
                                           i18n("You are about to delete the selected split. "
                                                "Do you really want to continue?"),
                                           i18n("KMyMoney"),
                                           KGuiItem(i18n("Continue"))
                                          ) == KMessageBox::Continue) {
      try {
        m_transaction.removeSplit(list[m_currentRow]);
        // if we removed the last split, select the previous
        if (m_currentRow && m_currentRow == list.count() - 1)
          selectRow(m_currentRow - 1);
        else
          selectRow(m_currentRow);
        emit transactionChanged(m_transaction);
      } catch (const MyMoneyException &e) {
        qDebug("Cannot remove split: %s", qPrintable(e.what()));
      }
    }
  }
}

KMyMoneyCategory* kMyMoneySplitTable::slotStartEdit()
{
  MYMONEYTRACER(tracer);
  return createEditWidgets(true);
}

void kMyMoneySplitTable::slotEndEdit()
{
  endEdit(false);
}

void kMyMoneySplitTable::slotEndEditKeyboard()
{
  endEdit(true);
}

void kMyMoneySplitTable::endEdit(bool keyboardDriven, bool setFocusToNextRow)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MYMONEYTRACER(tracer);
  MyMoneySplit s1 = m_split;

  if (!isEditSplitValid()) {
    KMessageBox::information(this, i18n("You need to assign a category to this split before it can be entered."), i18n("Enter split"), "EnterSplitWithEmptyCategory");
    m_editCategory->setFocus();
    return;
  }

  bool needUpdate = false;
  if (m_editCategory->selectedItem() != m_split.accountId()) {
    s1.setAccountId(m_editCategory->selectedItem());
    needUpdate = true;
  }
  if (m_editMemo->text() != m_split.memo()) {
    s1.setMemo(m_editMemo->text());
    needUpdate = true;
  }
  if (m_editTag->selectedTags() != m_split.tagIdList()) {
    s1.setTagIdList(m_editTag->selectedTags());
    needUpdate = true;
  }
  if (m_editAmount->value() != m_split.value()) {
    s1.setValue(m_editAmount->value());
    needUpdate = true;
  }

  if (needUpdate) {
    if (!s1.value().isZero()) {
      MyMoneyAccount cat = file->account(s1.accountId());
      if (cat.currencyId() != m_transaction.commodity()) {

        MyMoneySecurity fromCurrency, toCurrency;
        MyMoneyMoney fromValue, toValue;
        fromCurrency = file->security(m_transaction.commodity());
        toCurrency = file->security(cat.currencyId());

        // determine the fraction required for this category
        int fract = toCurrency.smallestAccountFraction();
        if (cat.accountType() == MyMoneyAccount::Cash)
          fract = toCurrency.smallestCashFraction();

        // display only positive values to the user
        fromValue = s1.value().abs();

        // if we had a price info in the beginning, we use it here
        if (m_priceInfo.find(cat.currencyId()) != m_priceInfo.end()) {
          toValue = (fromValue * m_priceInfo[cat.currencyId()]).convert(fract);
        }

        // if the shares are still 0, we need to change that
        if (toValue.isZero()) {
          const MyMoneyPrice &price = MyMoneyFile::instance()->price(fromCurrency.id(), toCurrency.id());
          // if the price is valid calculate the shares. If it is invalid
          // assume a conversion rate of 1.0
          if (price.isValid()) {
            toValue = (price.rate(toCurrency.id()) * fromValue).convert(fract);
          } else {
            toValue = fromValue;
          }
        }

        // now present all that to the user
        QPointer<KCurrencyCalculator> calc =
          new KCurrencyCalculator(fromCurrency,
                                  toCurrency,
                                  fromValue,
                                  toValue,
                                  m_transaction.postDate(),
                                  fract,
                                  this);

        if (calc->exec() == QDialog::Rejected) {
          delete calc;
          return;
        } else {
          s1.setShares((s1.value() * calc->price()).convert(fract));
          delete calc;
        }

      } else {
        s1.setShares(s1.value());
      }
    } else
      s1.setShares(s1.value());

    m_split = s1;
    try {
      if (m_split.id().isEmpty()) {
        m_transaction.addSplit(m_split);
      } else {
        m_transaction.modifySplit(m_split);
      }
      emit transactionChanged(m_transaction);
    } catch (const MyMoneyException &e) {
      qDebug("Cannot add/modify split: %s", qPrintable(e.what()));
    }
  }
  this->setFocus();
  destroyEditWidgets();
  if (setFocusToNextRow) {
    slotSetFocus(model()->index(currentRow() + 1, 0));
  }

  // if we still have more splits, we start editing right away
  // in case we have selected 'enter moves between fields'
  if (keyboardDriven
      && currentRow() < m_transaction.splits().count() - 1
      && KMyMoneyGlobalSettings::enterMovesBetweenFields()) {
    slotStartEdit();
  }

}

void kMyMoneySplitTable::slotCancelEdit()
{
  MYMONEYTRACER(tracer);
  if (isEditMode()) {
    /*
     * Prevent asking to add a new category which happens if the user entered any text
     * caused by emitting signals in KMyMoneyCombo::focusOutEvent() on focus out event.
     * (see bug 344409)
     */
    if (m_editCategory)
      m_editCategory->lineEdit()->setText(QString());
    destroyEditWidgets();
    this->setFocus();
  }
}

bool kMyMoneySplitTable::isEditMode() const
{
  // while the edit widgets exist we're in edit mode
  return m_editAmount || m_editMemo || m_editCategory || m_editTag;
}

bool kMyMoneySplitTable::isEditSplitValid() const
{
  return isEditMode() && !(m_editCategory && m_editCategory->selectedItem().isEmpty());
}

void kMyMoneySplitTable::destroyEditWidgets()
{
  MYMONEYTRACER(tracer);

  emit editFinished();

  disconnect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadEditWidgets()));

  destroyEditWidget(m_currentRow, 0);
  destroyEditWidget(m_currentRow, 1);
  destroyEditWidget(m_currentRow, 2);
  destroyEditWidget(m_currentRow, 3);
  destroyEditWidget(m_currentRow + 1, 0);
}

void kMyMoneySplitTable::destroyEditWidget(int r, int c)
{
  if (QWidget* cw = cellWidget(r, c))
    cw->hide();
  removeCellWidget(r, c);
}

KMyMoneyCategory* kMyMoneySplitTable::createEditWidgets(bool setFocus)
{
  MYMONEYTRACER(tracer);

  emit editStarted();

  QFont cellFont = KMyMoneyGlobalSettings::listCellFont();
  m_tabOrderWidgets.clear();

  // create the widgets
  m_editAmount = new kMyMoneyEdit(0);
  m_editAmount->setFont(cellFont);
  m_editAmount->setResetButtonVisible(false);
  m_editAmount->setPrecision(m_precision);

  m_editCategory = new KMyMoneyCategory();
  m_editCategory->setClickMessage(i18n("Category"));
  m_editCategory->setFont(cellFont);
  connect(m_editCategory, SIGNAL(createItem(QString,QString&)), this, SIGNAL(createCategory(QString,QString&)));
  connect(m_editCategory, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  m_editMemo = new kMyMoneyLineEdit(0, false, Qt::AlignLeft | Qt::AlignVCenter);
  m_editMemo->setClickMessage(i18n("Memo"));
  m_editMemo->setFont(cellFont);

  m_editTag = new KTagContainer;
  m_editTag->tagCombo()->setClickMessage(i18n("Tag"));
  m_editTag->tagCombo()->setFont(cellFont);
  m_editTag->loadTags(MyMoneyFile::instance()->tagList());
  connect(m_editTag->tagCombo(), SIGNAL(createItem(QString,QString&)), this, SLOT(slotCreateTag(QString,QString&)));
  connect(m_editTag->tagCombo(), SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  // create buttons for the mouse users
  m_registerButtonFrame = new QFrame(this);
  m_registerButtonFrame->setContentsMargins(0, 0, 0, 0);
  m_registerButtonFrame->setAutoFillBackground(true);

  QHBoxLayout* l = new QHBoxLayout(m_registerButtonFrame);
  l->setContentsMargins(0, 0, 0, 0);
  l->setSpacing(0);
  m_registerEnterButton = new KPushButton(KIcon("dialog-ok"), QString(), m_registerButtonFrame);
  m_registerCancelButton = new KPushButton(KIcon("dialog-cancel"), QString(), m_registerButtonFrame);

  l->addWidget(m_registerEnterButton);
  l->addWidget(m_registerCancelButton);
  l->addStretch(2);

  connect(m_registerEnterButton, SIGNAL(clicked()), this, SLOT(slotEndEdit()));
  connect(m_registerCancelButton, SIGNAL(clicked()), this, SLOT(slotCancelEdit()));

  // setup tab order
  addToTabOrder(m_editCategory);
  addToTabOrder(m_editMemo);
  addToTabOrder(m_editTag);
  addToTabOrder(m_editAmount);
  addToTabOrder(m_registerEnterButton);
  addToTabOrder(m_registerCancelButton);

  if (!m_split.accountId().isEmpty()) {
    m_editCategory->setSelectedItem(m_split.accountId());
  } else {
    // check if the transaction is balanced or not. If not,
    // assign the remainder to the amount.
    MyMoneyMoney diff;
    QList<MyMoneySplit> list = m_transaction.splits();
    QList<MyMoneySplit>::ConstIterator it_s;
    for (it_s = list.constBegin(); it_s != list.constEnd(); ++it_s) {
      if (!(*it_s).accountId().isEmpty())
        diff += (*it_s).value();
    }
    m_split.setValue(-diff);
  }

  QList<QString> t = m_split.tagIdList();
  if (!t.isEmpty()) {
    for (int i = 0; i < t.size(); i++)
      m_editTag->addTagWidget(t[i]);
  }

  m_editMemo->loadText(m_split.memo());
  // don't allow automatically calculated values to be modified
  if (m_split.value() == MyMoneyMoney::autoCalc) {
    m_editAmount->setEnabled(false);
    m_editAmount->loadText("will be calculated");
  } else
    m_editAmount->setValue(m_split.value());

  setCellWidget(m_currentRow, 0, m_editCategory);
  setCellWidget(m_currentRow, 1, m_editMemo);
  setCellWidget(m_currentRow, 2, m_editTag);
  setCellWidget(m_currentRow, 3, m_editAmount);
  setCellWidget(m_currentRow + 1, 0, m_registerButtonFrame);

  // load e.g. the category widget with the account list
  slotLoadEditWidgets();
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadEditWidgets()));

  foreach (QWidget* w, m_tabOrderWidgets) {
    if (w) {
      w->installEventFilter(this);
    }
  }

  if (setFocus) {
    m_editCategory->lineEdit()->setFocus();
    m_editCategory->lineEdit()->selectAll();
  }

  // resize the rows so the added edit widgets would fit appropriately
  resizeRowsToContents();

  return m_editCategory;
}

void kMyMoneySplitTable::slotLoadEditWidgets()
{
  // reload category widget
  QString categoryId = m_editCategory->selectedItem();

  AccountSet aSet;
  aSet.addAccountGroup(MyMoneyAccount::Asset);
  aSet.addAccountGroup(MyMoneyAccount::Liability);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  if (KMyMoneyGlobalSettings::expertMode())
    aSet.addAccountGroup(MyMoneyAccount::Equity);

  // remove the accounts with invalid types at this point
  aSet.removeAccountType(MyMoneyAccount::CertificateDep);
  aSet.removeAccountType(MyMoneyAccount::Investment);
  aSet.removeAccountType(MyMoneyAccount::Stock);
  aSet.removeAccountType(MyMoneyAccount::MoneyMarket);

  aSet.load(m_editCategory->selector());

  // if an account is specified then remove it from the widget so that the user
  // cannot create a transfer with from and to account being the same account
  if (!m_account.id().isEmpty())
    m_editCategory->selector()->removeItem(m_account.id());

  if (!categoryId.isEmpty())
    m_editCategory->setSelectedItem(categoryId);
}

void kMyMoneySplitTable::addToTabOrder(QWidget* w)
{
  if (w) {
    while (w->focusProxy())
      w = w->focusProxy();
    m_tabOrderWidgets.append(w);
  }
}

bool kMyMoneySplitTable::focusNextPrevChild(bool next)
{
  MYMONEYTRACER(tracer);
  bool  rc = false;
  if (isEditMode()) {
    QWidget *w = 0;

    w = qApp->focusWidget();
    int currentWidgetIndex = m_tabOrderWidgets.indexOf(w);
    while (w && currentWidgetIndex == -1) {
      // qDebug("'%s' not in list, use parent", w->className());
      w = w->parentWidget();
      currentWidgetIndex = m_tabOrderWidgets.indexOf(w);
    }

    if (currentWidgetIndex != -1) {
      // if(w) qDebug("tab order is at '%s'", w->className());
      currentWidgetIndex += next ? 1 : -1;
      if (currentWidgetIndex < 0)
        currentWidgetIndex = m_tabOrderWidgets.size() - 1;
      else if (currentWidgetIndex >= m_tabOrderWidgets.size())
        currentWidgetIndex = 0;

      w = m_tabOrderWidgets[currentWidgetIndex];

      if (((w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus) && w->isVisible() && w->isEnabled()) {
        // qDebug("Selecting '%s' as focus", w->className());
        w->setFocus();
        rc = true;
      }
    }
  } else
    rc = QTableWidget::focusNextPrevChild(next);
  return rc;
}

void kMyMoneySplitTable::slotCreateTag(const QString& name, QString& id)
{
    emit createTag(name, id);
    slotUpdateTag(id);
}

void kMyMoneySplitTable::slotUpdateTag(QString& id)
{
    m_editTag->loadTags(MyMoneyFile::instance()->tagList());
    m_editTag->tagCombo()->setSelectedItem(id);
}
