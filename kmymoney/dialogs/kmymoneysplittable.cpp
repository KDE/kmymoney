/*
 * Copyright 2008-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#include "kmymoneysplittable.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCursor>
#include <QApplication>
#include <QTimer>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QFrame>
#include <QMouseEvent>
#include <QEvent>
#include <QPushButton>
#include <QMenu>
#include <QIcon>
#include <QHeaderView>
#include <QPointer>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KCompletionBox>
#include <KSharedConfig>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneyprice.h"
#include "amountedit.h"
#include "kmymoneycategory.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneylineedit.h"
#include "mymoneysecurity.h"
#include "kmymoneysettings.h"
#include "kmymoneymvccombo.h"
#include "mymoneytag.h"
#include "kmymoneytagcombo.h"
#include "ktagcontainer.h"
#include "kcurrencycalculator.h"
#include "mymoneyutils.h"
#include "mymoneytracer.h"
#include "mymoneyexception.h"
#include "icons.h"
#include "mymoneyenums.h"

using namespace Icons;

class KMyMoneySplitTablePrivate
{
  Q_DISABLE_COPY(KMyMoneySplitTablePrivate)

public:
  KMyMoneySplitTablePrivate() :
    m_currentRow(0),
    m_maxRows(0),
    m_precision(2),
    m_contextMenu(nullptr),
    m_contextMenuDelete(nullptr),
    m_contextMenuDuplicate(nullptr),
    m_editCategory(0),
    m_editTag(0),
    m_editMemo(0),
    m_editAmount(0)
  {
  }

  ~KMyMoneySplitTablePrivate()
  {
  }

  /// the currently selected row (will be printed as selected)
  int                 m_currentRow;

  /// the number of rows filled with data
  int                 m_maxRows;

  MyMoneyTransaction  m_transaction;
  MyMoneyAccount      m_account;
  MyMoneySplit        m_split;
  MyMoneySplit        m_hiddenSplit;

  /**
    * This member keeps the precision for the values
    */
  int                 m_precision;

  /**
    * This member keeps a pointer to the context menu
    */
  QMenu*         m_contextMenu;

  /// keeps the QAction of the delete entry in the context menu
  QAction*       m_contextMenuDelete;

  /// keeps the QAction of the duplicate entry in the context menu
  QAction*       m_contextMenuDuplicate;

  /**
    * This member contains a pointer to the input widget for the category.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  QPointer<KMyMoneyCategory> m_editCategory;

  /**
    * This member contains a pointer to the tag widget for the memo.
    */
  QPointer<KTagContainer> m_editTag;

  /**
    * This member contains a pointer to the input widget for the memo.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  QPointer<KMyMoneyLineEdit> m_editMemo;

  /**
    * This member contains a pointer to the input widget for the amount.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  QPointer<AmountEdit>     m_editAmount;

  /**
    * This member keeps the tab order for the above widgets
    */
  QWidgetList         m_tabOrderWidgets;

  QPointer<QFrame>           m_registerButtonFrame;
  QPointer<QPushButton>      m_registerEnterButton;
  QPointer<QPushButton>      m_registerCancelButton;

  QMap<QString, MyMoneyMoney>  m_priceInfo;
};

KMyMoneySplitTable::KMyMoneySplitTable(QWidget *parent) :
    QTableWidget(parent),
    d_ptr(new KMyMoneySplitTablePrivate)
{
  Q_D(KMyMoneySplitTable);
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

  setFont(KMyMoneySettings::listCellFontEx());

  setAlternatingRowColors(true);

  verticalHeader()->hide();
  horizontalHeader()->setSectionsMovable(false);
  horizontalHeader()->setFont(KMyMoneySettings::listHeaderFontEx());

  KConfigGroup grp = KSharedConfig::openConfig()->group("SplitTable");
  QByteArray columns;
  columns = grp.readEntry("HeaderState", columns);
  horizontalHeader()->restoreState(columns);
  horizontalHeader()->setStretchLastSection(true);

  setShowGrid(KMyMoneySettings::showGrid());

  setEditTriggers(QAbstractItemView::NoEditTriggers);

  // setup the context menu
  d->m_contextMenu = new QMenu(this);
  d->m_contextMenu->setTitle(i18n("Split Options"));
  d->m_contextMenu->setIcon(Icons::get(Icon::ViewFinancialTransfer));
  d->m_contextMenu->addAction(Icons::get(Icon::DocumentEdit), i18n("Edit..."), this, SLOT(slotStartEdit()));
  d->m_contextMenuDuplicate = d->m_contextMenu->addAction(Icons::get(Icon::EditCopy), i18nc("To duplicate a split", "Duplicate"), this, SLOT(slotDuplicateSplit()));
  d->m_contextMenuDelete = d->m_contextMenu->addAction(Icons::get(Icon::EditDelete),
                        i18n("Delete..."),
                        this, SLOT(slotDeleteSplit()));

  connect(this, &QAbstractItemView::clicked,
          this, static_cast<void (KMyMoneySplitTable::*)(const QModelIndex&)>(&KMyMoneySplitTable::slotSetFocus));

  connect(this, &KMyMoneySplitTable::transactionChanged,
          this, &KMyMoneySplitTable::slotUpdateData);

  installEventFilter(this);
}

KMyMoneySplitTable::~KMyMoneySplitTable()
{
  Q_D(KMyMoneySplitTable);
  auto grp = KSharedConfig::openConfig()->group("SplitTable");
  QByteArray columns = horizontalHeader()->saveState();
  grp.writeEntry("HeaderState", columns);
  grp.sync();
  delete d;
}

int KMyMoneySplitTable::currentRow() const
{
  Q_D(const KMyMoneySplitTable);
  return d->m_currentRow;
}

void KMyMoneySplitTable::setup(const QMap<QString, MyMoneyMoney>& priceInfo, int precision)
{
  Q_D(KMyMoneySplitTable);
  d->m_priceInfo = priceInfo;
  d->m_precision = precision;
}

bool KMyMoneySplitTable::eventFilter(QObject *o, QEvent *e)
{
  Q_D(KMyMoneySplitTable);
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
        if (row < d->m_transaction.splits().count() - 1)
          slotSetFocus(model()->index(row + 1, 0));
        break;

      case Qt::Key_Home:
        slotSetFocus(model()->index(0, 0));
        break;

      case Qt::Key_End:
        slotSetFocus(model()->index(d->m_transaction.splits().count() - 1, 0));
        break;

      case Qt::Key_PageUp:
        if (lines) {
          while (lines-- > 0 && row)
            --row;
          slotSetFocus(model()->index(row, 0));
        }
        break;

      case Qt::Key_PageDown:
        if (row < d->m_transaction.splits().count() - 1) {
          while (lines-- > 0 && row < d->m_transaction.splits().count() - 1)
            ++row;
          slotSetFocus(model()->index(row, 0));
        }
        break;

      case Qt::Key_Delete:
        slotDeleteSplit();
        break;

      case Qt::Key_Return:
      case Qt::Key_Enter:
        if (row < d->m_transaction.splits().count() - 1
            && KMyMoneySettings::enterMovesBetweenFields()) {
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
          slotSetFocus(model()->index(d->m_transaction.splits().count() - 1, 0));
          slotStartEdit();

        } else if (k->text()[ 0 ].isPrint()) {
          KMyMoneyCategory* cat = createEditWidgets(false);
          if (cat) {
            KMyMoneyLineEdit *le = qobject_cast<KMyMoneyLineEdit*>(cat->lineEdit());
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
          if (auto le = dynamic_cast<KLineEdit*>(o)) {
            KCompletionBox* box = le->completionBox(false);
            if (box && box->isVisible()) {
              terminate = false;
              le->completionBox(false)->hide();
            }
          }
        }

        // in case we have the 'enter moves focus between fields', we need to simulate
        // a TAB key when the object 'o' points to the category or memo field.
        if (KMyMoneySettings::enterMovesBetweenFields()) {
          if (o == d->m_editCategory->lineEdit() || o == d->m_editMemo || o == d->m_editTag) {
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
        d->m_contextMenuDelete->setEnabled(
          row < d->m_transaction.splits().count() - 1);
        d->m_contextMenuDuplicate->setEnabled(
          row < d->m_transaction.splits().count() - 1);

        d->m_contextMenu->exec(QCursor::pos());
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

void KMyMoneySplitTable::slotSetFocus(const QModelIndex& index)
{
  slotSetFocus(index, Qt::LeftButton);
}

void KMyMoneySplitTable::slotSetFocus(const QModelIndex& index, int button)
{
  Q_D(KMyMoneySplitTable);
  MYMONEYTRACER(tracer);
  auto row = index.row();

  // adjust row to used area
  if (row > d->m_transaction.splits().count() - 1)
    row = d->m_transaction.splits().count() - 1;
  if (row < 0)
    row = 0;

  // make sure the row will be on the screen
  scrollTo(model()->index(row, 0));

  if (isEditMode()) {                   // in edit mode?
    if (isEditSplitValid() && KMyMoneySettings::focusChangeIsEnter())
      endEdit(false/*keyboard driven*/, false/*set focus to next row*/);
    else
      slotCancelEdit();
  }

  if (button == Qt::LeftButton) {         // left mouse button
    if (row != currentRow()) {
      // setup new current row and update visible selection
      selectRow(row);
      slotUpdateData(d->m_transaction);
    }
  } else if (button == Qt::RightButton) {
    // context menu is only available when cursor is on
    // an existing transaction or the first line after this area
    if (row == index.row()) {
      // setup new current row and update visible selection
      selectRow(row);
      slotUpdateData(d->m_transaction);

      // if the very last entry is selected, the delete
      // operation is not available otherwise it is
      d->m_contextMenuDelete->setEnabled(
        row < d->m_transaction.splits().count() - 1);
      d->m_contextMenuDuplicate->setEnabled(
        row < d->m_transaction.splits().count() - 1);

      d->m_contextMenu->exec(QCursor::pos());
    }
  }
}

void KMyMoneySplitTable::mousePressEvent(QMouseEvent* e)
{
  slotSetFocus(indexAt(e->pos()), e->button());
}

/* turn off QTable behaviour */
void KMyMoneySplitTable::mouseReleaseEvent(QMouseEvent* /* e */)
{
}

void KMyMoneySplitTable::mouseDoubleClickEvent(QMouseEvent *e)
{
  Q_D(KMyMoneySplitTable);
  MYMONEYTRACER(tracer);

  int col = columnAt(e->pos().x());
  slotSetFocus(model()->index(rowAt(e->pos().y()), col), e->button());
  createEditWidgets(false);

  QLineEdit* editWidget = 0;    //krazy:exclude=qmethods
  switch (col) {
    case 0:
      editWidget = d->m_editCategory->lineEdit();
      break;

    case 1:
      editWidget = d->m_editMemo;
      break;

    case 2:
     d->m_editTag->tagCombo()->setFocus();
      break;

    case 3:
      editWidget = d->m_editAmount;
      break;

    default:
      break;
  }
  if (editWidget) {
    editWidget->setFocus();
    editWidget->selectAll();
  }
}

void KMyMoneySplitTable::selectRow(int row)
{
  Q_D(KMyMoneySplitTable);
  MYMONEYTRACER(tracer);

  if (row > d->m_maxRows)
    row = d->m_maxRows;
  d->m_currentRow = row;
  QTableWidget::selectRow(row);
  QList<MyMoneySplit> list = getSplits(d->m_transaction);
  if (row < list.count())
    d->m_split = list[row];
  else
    d->m_split = MyMoneySplit();
}

void KMyMoneySplitTable::setRowCount(int irows)
{
  QTableWidget::setRowCount(irows);

  // determine row height according to the edit widgets
  // we use the category widget as the base
  QFontMetrics fm(KMyMoneySettings::listCellFontEx());
  int height = fm.lineSpacing() + 6;
#if 0
  // recalculate row height hint
  KMyMoneyCategory cat;
  height = qMax(cat.sizeHint().height(), height);
#endif

  verticalHeader()->setUpdatesEnabled(false);

  for (auto i = 0; i < irows; ++i)
    verticalHeader()->resizeSection(i, height);

  verticalHeader()->setUpdatesEnabled(true);
}

void KMyMoneySplitTable::setTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s, const MyMoneyAccount& acc)
{
  Q_D(KMyMoneySplitTable);
  MYMONEYTRACER(tracer);
  d->m_transaction = t;
  d->m_account = acc;
  d->m_hiddenSplit = s;
  selectRow(0);
  slotUpdateData(d->m_transaction);
}

MyMoneyTransaction KMyMoneySplitTable::transaction() const
{
  Q_D(const KMyMoneySplitTable);
  return d->m_transaction;
}

QList<MyMoneySplit> KMyMoneySplitTable::getSplits(const MyMoneyTransaction& t) const
{
  Q_D(const KMyMoneySplitTable);
  // get list of splits
  QList<MyMoneySplit> list = t.splits();

  // and ignore the one that should be hidden
  QList<MyMoneySplit>::Iterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    if ((*it).id() == d->m_hiddenSplit.id()) {
      list.erase(it);
      break;
    }
  }
  return list;
}

void KMyMoneySplitTable::slotUpdateData(const MyMoneyTransaction& t)
{
  Q_D(KMyMoneySplitTable);
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
        qDebug("Unexpected exception in KMyMoneySplitTable::slotUpdateData()");
      }
    }
    QString amountTxt = value.formatMoney(d->m_account.fraction());
    if (value == MyMoneyMoney::autoCalc) {
      amountTxt = i18n("will be calculated");
    }

    if (colText.isEmpty() && (*it).memo().isEmpty() && value.isZero())
      amountTxt.clear();

    unsigned width = fontMetrics().width(amountTxt);
    AmountEdit* valfield = new AmountEdit();
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

    QList<QString> tl = (*it).tagIdList();
    QStringList tagNames;
    if (!tl.isEmpty()) {
      for (int i = 0; i < tl.size(); i++)
        tagNames.append(MyMoneyFile::instance()->tag(tl[i]).name());
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
    for (auto i = 0 ; i < 4; ++i) {
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

void KMyMoneySplitTable::updateTransactionTableSize()
{
  Q_D(KMyMoneySplitTable);
  // get current size of transactions table
  int tableHeight = height();
  int splitCount = d->m_transaction.splits().count() - 1;

  if (splitCount < 0)
    splitCount = 0;

  // see if we need some extra lines to fill the current size with the grid
  int numExtraLines = (tableHeight / rowHeight(0)) - splitCount;
  if (numExtraLines < 2)
    numExtraLines = 2;

  setRowCount(splitCount + numExtraLines);
  d->m_maxRows = splitCount;
}

void KMyMoneySplitTable::resizeEvent(QResizeEvent* ev)
{
  QTableWidget::resizeEvent(ev);
  if (!isEditMode()) {
    // update the size of the transaction table only if a split is not being edited
    // otherwise the height of the editors would be altered in an undesired way
    updateTransactionTableSize();
  }
}

void KMyMoneySplitTable::slotDuplicateSplit()
{
  Q_D(KMyMoneySplitTable);
  MYMONEYTRACER(tracer);
  QList<MyMoneySplit> list = getSplits(d->m_transaction);
  if (d->m_currentRow < list.count()) {
    MyMoneySplit split = list[d->m_currentRow];
    split.clearId();
    try {
      d->m_transaction.addSplit(split);
      emit transactionChanged(d->m_transaction);
    } catch (const MyMoneyException &e) {
      qDebug("Cannot duplicate split: %s", e.what());
    }
  }
}

void KMyMoneySplitTable::slotDeleteSplit()
{
  Q_D(KMyMoneySplitTable);
  MYMONEYTRACER(tracer);
  QList<MyMoneySplit> list = getSplits(d->m_transaction);
  if (d->m_currentRow < list.count()) {
    if (KMessageBox::warningContinueCancel(this,
                                           i18n("You are about to delete the selected split. "
                                                "Do you really want to continue?"),
                                           i18n("KMyMoney")
                                          ) == KMessageBox::Continue) {
      try {
        d->m_transaction.removeSplit(list[d->m_currentRow]);
        // if we removed the last split, select the previous
        if (d->m_currentRow && d->m_currentRow == list.count() - 1)
          selectRow(d->m_currentRow - 1);
        else
          selectRow(d->m_currentRow);
        emit transactionChanged(d->m_transaction);
      } catch (const MyMoneyException &e) {
        qDebug("Cannot remove split: %s", e.what());
      }
    }
  }
}

KMyMoneyCategory* KMyMoneySplitTable::slotStartEdit()
{
  MYMONEYTRACER(tracer);
  return createEditWidgets(true);
}

void KMyMoneySplitTable::slotEndEdit()
{
  endEdit(false);
}

void KMyMoneySplitTable::slotEndEditKeyboard()
{
  endEdit(true);
}

void KMyMoneySplitTable::endEdit(bool keyboardDriven, bool setFocusToNextRow)
{
  Q_D(KMyMoneySplitTable);
  auto file = MyMoneyFile::instance();

  MYMONEYTRACER(tracer);
  MyMoneySplit s1 = d->m_split;

  if (!isEditSplitValid()) {
    KMessageBox::information(this, i18n("You need to assign a category to this split before it can be entered."), i18n("Enter split"), "EnterSplitWithEmptyCategory");
    d->m_editCategory->setFocus();
    return;
  }

  bool needUpdate = false;
  if (d->m_editCategory->selectedItem() != d->m_split.accountId()) {
    s1.setAccountId(d->m_editCategory->selectedItem());
    needUpdate = true;
  }
  if (d->m_editMemo->text() != d->m_split.memo()) {
    s1.setMemo(d->m_editMemo->text());
    needUpdate = true;
  }
  if (d->m_editTag->selectedTags() != d->m_split.tagIdList()) {
    s1.setTagIdList(d->m_editTag->selectedTags());
    needUpdate = true;
  }
  if (d->m_editAmount->value() != d->m_split.value()) {
    s1.setValue(d->m_editAmount->value());
    needUpdate = true;
  }

  if (needUpdate) {
    if (!s1.value().isZero()) {
      MyMoneyAccount cat = file->account(s1.accountId());
      if (cat.currencyId() != d->m_transaction.commodity()) {

        MyMoneySecurity fromCurrency, toCurrency;
        MyMoneyMoney fromValue, toValue;
        fromCurrency = file->security(d->m_transaction.commodity());
        toCurrency = file->security(cat.currencyId());

        // determine the fraction required for this category
        int fract = toCurrency.smallestAccountFraction();
        if (cat.accountType() == eMyMoney::Account::Type::Cash)
          fract = toCurrency.smallestCashFraction();

        // display only positive values to the user
        fromValue = s1.value().abs();

        // if we had a price info in the beginning, we use it here
        if (d->m_priceInfo.find(cat.currencyId()) != d->m_priceInfo.end()) {
          toValue = (fromValue * d->m_priceInfo[cat.currencyId()]).convert(fract);
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
                                  d->m_transaction.postDate(),
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

    d->m_split = s1;
    try {
      if (d->m_split.id().isEmpty()) {
        d->m_transaction.addSplit(d->m_split);
      } else {
        d->m_transaction.modifySplit(d->m_split);
      }
      emit transactionChanged(d->m_transaction);
    } catch (const MyMoneyException &e) {
      qDebug("Cannot add/modify split: %s", e.what());
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
      && currentRow() < d->m_transaction.splits().count() - 1
      && KMyMoneySettings::enterMovesBetweenFields()) {
    slotStartEdit();
  }

}

void KMyMoneySplitTable::slotCancelEdit()
{
  Q_D(const KMyMoneySplitTable);
  MYMONEYTRACER(tracer);
  if (isEditMode()) {
    /*
     * Prevent asking to add a new category which happens if the user entered any text
     * caused by emitting signals in KMyMoneyCombo::focusOutEvent() on focus out event.
     * (see bug 344409)
     */
    if (d->m_editCategory)
      d->m_editCategory->lineEdit()->setText(QString());
    destroyEditWidgets();
    this->setFocus();
  }
}

bool KMyMoneySplitTable::isEditMode() const
{
  Q_D(const KMyMoneySplitTable);
  // while the edit widgets exist we're in edit mode
  return d->m_editAmount || d->m_editMemo || d->m_editCategory || d->m_editTag;
}

bool KMyMoneySplitTable::isEditSplitValid() const
{
  Q_D(const KMyMoneySplitTable);
  return isEditMode() && !(d->m_editCategory && d->m_editCategory->selectedItem().isEmpty());
}

void KMyMoneySplitTable::destroyEditWidgets()
{
  MYMONEYTRACER(tracer);

  Q_D(KMyMoneySplitTable);
  emit editFinished();

  disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KMyMoneySplitTable::slotLoadEditWidgets);

  destroyEditWidget(d->m_currentRow, 0);
  destroyEditWidget(d->m_currentRow, 1);
  destroyEditWidget(d->m_currentRow, 2);
  destroyEditWidget(d->m_currentRow, 3);
  destroyEditWidget(d->m_currentRow + 1, 0);
}

void KMyMoneySplitTable::destroyEditWidget(int r, int c)
{
  if (QWidget* cw = cellWidget(r, c))
    cw->hide();
  removeCellWidget(r, c);
}

KMyMoneyCategory* KMyMoneySplitTable::createEditWidgets(bool setFocus)
{
  MYMONEYTRACER(tracer);

  emit editStarted();

  Q_D(KMyMoneySplitTable);
  auto cellFont = KMyMoneySettings::listCellFontEx();
  d->m_tabOrderWidgets.clear();

  // create the widgets
  d->m_editAmount = new AmountEdit;
  d->m_editAmount->setFont(cellFont);
  d->m_editAmount->setCalculatorButtonVisible(true);
  d->m_editAmount->setPrecision(d->m_precision);

  d->m_editCategory = new KMyMoneyCategory();
  d->m_editCategory->setPlaceholderText(i18n("Category"));
  d->m_editCategory->setFont(cellFont);
  connect(d->m_editCategory, SIGNAL(createItem(QString,QString&)), this, SIGNAL(createCategory(QString,QString&)));
  connect(d->m_editCategory, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  d->m_editMemo = new KMyMoneyLineEdit(0, false, Qt::AlignLeft | Qt::AlignVCenter);
  d->m_editMemo->setPlaceholderText(i18n("Memo"));
  d->m_editMemo->setFont(cellFont);

  d->m_editTag = new KTagContainer;
  d->m_editTag->tagCombo()->setPlaceholderText(i18n("Tag"));
  d->m_editTag->tagCombo()->setFont(cellFont);
  d->m_editTag->loadTags(MyMoneyFile::instance()->tagList());
  connect(d->m_editTag->tagCombo(), SIGNAL(createItem(QString,QString&)), this, SIGNAL(createTag(QString,QString&)));
  connect(d->m_editTag->tagCombo(), SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  // create buttons for the mouse users
  d->m_registerButtonFrame = new QFrame(this);
  d->m_registerButtonFrame->setContentsMargins(0, 0, 0, 0);
  d->m_registerButtonFrame->setAutoFillBackground(true);

  QHBoxLayout* l = new QHBoxLayout(d->m_registerButtonFrame);
  l->setContentsMargins(0, 0, 0, 0);
  l->setSpacing(0);
  d->m_registerEnterButton = new QPushButton(Icons::get(Icon::DialogOK)
                                          , QString(), d->m_registerButtonFrame);
  d->m_registerCancelButton = new QPushButton(Icons::get(Icon::DialogCancel)
                                           , QString(), d->m_registerButtonFrame);

  l->addWidget(d->m_registerEnterButton);
  l->addWidget(d->m_registerCancelButton);
  l->addStretch(2);

  connect(d->m_registerEnterButton.data(), &QAbstractButton::clicked, this, &KMyMoneySplitTable::slotEndEdit);
  connect(d->m_registerCancelButton.data(), &QAbstractButton::clicked, this, &KMyMoneySplitTable::slotCancelEdit);

  // setup tab order
  addToTabOrder(d->m_editCategory);
  addToTabOrder(d->m_editMemo);
  addToTabOrder(d->m_editTag);
  addToTabOrder(d->m_editAmount);
  addToTabOrder(d->m_registerEnterButton);
  addToTabOrder(d->m_registerCancelButton);

  if (!d->m_split.accountId().isEmpty()) {
    d->m_editCategory->setSelectedItem(d->m_split.accountId());
  } else {
    // check if the transaction is balanced or not. If not,
    // assign the remainder to the amount.
    MyMoneyMoney diff;
    QList<MyMoneySplit> list = d->m_transaction.splits();
    QList<MyMoneySplit>::ConstIterator it_s;
    for (it_s = list.constBegin(); it_s != list.constEnd(); ++it_s) {
      if (!(*it_s).accountId().isEmpty())
        diff += (*it_s).value();
    }
    d->m_split.setValue(-diff);
  }

  QList<QString> t = d->m_split.tagIdList();
  if (!t.isEmpty()) {
    for (int i = 0; i < t.size(); i++)
      d->m_editTag->addTagWidget(t[i]);
  }

  d->m_editMemo->loadText(d->m_split.memo());
  // don't allow automatically calculated values to be modified
  if (d->m_split.value() == MyMoneyMoney::autoCalc) {
    d->m_editAmount->setEnabled(false);
    d->m_editAmount->setText("will be calculated");
  } else
    d->m_editAmount->setValue(d->m_split.value());

  setCellWidget(d->m_currentRow, 0, d->m_editCategory);
  setCellWidget(d->m_currentRow, 1, d->m_editMemo);
  setCellWidget(d->m_currentRow, 2, d->m_editTag);
  setCellWidget(d->m_currentRow, 3, d->m_editAmount);
  setCellWidget(d->m_currentRow + 1, 0, d->m_registerButtonFrame);

  // load e.g. the category widget with the account list
  slotLoadEditWidgets();
  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KMyMoneySplitTable::slotLoadEditWidgets);

  foreach (QWidget* w, d->m_tabOrderWidgets) {
    if (w) {
      w->installEventFilter(this);
    }
  }

  if (setFocus) {
    d->m_editCategory->lineEdit()->setFocus();
    d->m_editCategory->lineEdit()->selectAll();
  }

  // resize the rows so the added edit widgets would fit appropriately
  resizeRowsToContents();

  return d->m_editCategory;
}

void KMyMoneySplitTable::slotLoadEditWidgets()
{
  Q_D(KMyMoneySplitTable);
  // reload category widget
  auto categoryId = d->m_editCategory->selectedItem();

  AccountSet aSet;
  aSet.addAccountGroup(eMyMoney::Account::Type::Asset);
  aSet.addAccountGroup(eMyMoney::Account::Type::Liability);
  aSet.addAccountGroup(eMyMoney::Account::Type::Income);
  aSet.addAccountGroup(eMyMoney::Account::Type::Expense);
  if (KMyMoneySettings::expertMode())
    aSet.addAccountGroup(eMyMoney::Account::Type::Equity);

  // remove the accounts with invalid types at this point
  aSet.removeAccountType(eMyMoney::Account::Type::CertificateDep);
  aSet.removeAccountType(eMyMoney::Account::Type::Investment);
  aSet.removeAccountType(eMyMoney::Account::Type::Stock);
  aSet.removeAccountType(eMyMoney::Account::Type::MoneyMarket);

  aSet.load(d->m_editCategory->selector());

  // if an account is specified then remove it from the widget so that the user
  // cannot create a transfer with from and to account being the same account
  if (!d->m_account.id().isEmpty())
    d->m_editCategory->selector()->removeItem(d->m_account.id());

  if (!categoryId.isEmpty())
    d->m_editCategory->setSelectedItem(categoryId);
}

void KMyMoneySplitTable::addToTabOrder(QWidget* w)
{
  Q_D(KMyMoneySplitTable);
  if (w) {
    while (w->focusProxy())
      w = w->focusProxy();
    d->m_tabOrderWidgets.append(w);
  }
}

bool KMyMoneySplitTable::focusNextPrevChild(bool next)
{
  MYMONEYTRACER(tracer);
  Q_D(KMyMoneySplitTable);
  auto rc = false;
  if (isEditMode()) {
    QWidget *w = 0;

    w = qApp->focusWidget();
    int currentWidgetIndex = d->m_tabOrderWidgets.indexOf(w);
    while (w && currentWidgetIndex == -1) {
      // qDebug("'%s' not in list, use parent", w->className());
      w = w->parentWidget();
      currentWidgetIndex = d->m_tabOrderWidgets.indexOf(w);
    }

    if (currentWidgetIndex != -1) {
      // if(w) qDebug("tab order is at '%s'", w->className());
      currentWidgetIndex += next ? 1 : -1;
      if (currentWidgetIndex < 0)
        currentWidgetIndex = d->m_tabOrderWidgets.size() - 1;
      else if (currentWidgetIndex >= d->m_tabOrderWidgets.size())
        currentWidgetIndex = 0;

      w = d->m_tabOrderWidgets[currentWidgetIndex];

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
