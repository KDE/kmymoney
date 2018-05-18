/***************************************************************************
                          splitdelegate.cpp
                             -------------------
    begin                : Wed Apr 6 2016
    copyright            : (C) 2016 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "splitdelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerview.h"
#include "models.h"
#include "accountsmodel.h"
#include "ledgermodel.h"
#include "splitmodel.h"
#include "newspliteditor.h"
#include "mymoneyaccount.h"
#include "modelenums.h"

using namespace eLedgerModel;

QColor SplitDelegate::m_erroneousColor = QColor(Qt::red);
QColor SplitDelegate::m_importedColor = QColor(Qt::yellow);

class SplitDelegate::Private
{
public:
  Private()
  : m_editor(0)
  , m_editorRow(-1)
  , m_showValuesInverted(false)
  {}

  NewSplitEditor*               m_editor;
  int                           m_editorRow;
  bool                          m_showValuesInverted;
};


SplitDelegate::SplitDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
  , d(new Private)
{
}

SplitDelegate::~SplitDelegate()
{
  delete d;
}

void SplitDelegate::setErroneousColor(const QColor& color)
{
  m_erroneousColor = color;
}

QWidget* SplitDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);

  if(index.isValid()) {
    Q_ASSERT(parent);
    LedgerView* view = qobject_cast<LedgerView*>(parent->parentWidget());
    Q_ASSERT(view != 0);

    if(view->selectionModel()->selectedRows().count() > 1) {
      qDebug() << "Editing multiple splits at once is not yet supported";

      /**
       * @todo replace the following three lines with the creation of a special
       * editor that can handle multiple splits at once or show a message to the user
       * that this is not possible
       */
      d->m_editor = 0;
      SplitDelegate* const that = const_cast<SplitDelegate* const>(this);
      emit that->closeEditor(d->m_editor, NoHint);

    } else {
      d->m_editor = new NewSplitEditor(parent, view->accountId());
    }

    if(d->m_editor) {
      d->m_editorRow = index.row();
      connect(d->m_editor, SIGNAL(done()), this, SLOT(endEdit()));
      emit sizeHintChanged(index);
    }

  } else {
    qFatal("SplitDelegate::createEditor(): we should never end up here");
  }
  return d->m_editor;
}

int SplitDelegate::editorRow() const
{
  return d->m_editorRow;
}

void SplitDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // never change the background of the cell the mouse is hovering over
  opt.state &= ~QStyle::State_MouseOver;

  // show the focus only on the detail column
  opt.state &= ~QStyle::State_HasFocus;
  if(index.column() == (int)Column::Detail) {
    QAbstractItemView* view = qobject_cast< QAbstractItemView* >(parent());
    if(view) {
      if(view->currentIndex().row() == index.row()) {
        opt.state |= QStyle::State_HasFocus;
      }
    }
  }

  painter->save();

  // Background
  auto bgOpt = opt;
  // if selected, always show as active, so that the
  // background does not change when the editor is shown
  if (opt.state & QStyle::State_Selected) {
    bgOpt.state |= QStyle::State_Active;
  }
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &bgOpt, painter, opt.widget);

  // Do not paint text if the edit widget is shown
  const LedgerView *view = qobject_cast<const LedgerView *>(opt.widget);
  if (view && view->indexWidget(index)) {
    painter->restore();
    return;
  }

  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
  const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);

  QStringList lines;
  if(index.column() == (int)Column::Detail) {
    lines << index.model()->data(index, (int)Role::Account).toString();
    lines << index.model()->data(index, (int)Role::SingleLineMemo).toString();
    lines.removeAll(QString());
  }

  // draw the text items
  if(!opt.text.isEmpty() || !lines.isEmpty()) {

    QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                              ? QPalette::Normal : QPalette::Disabled;

    if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
      cg = QPalette::Inactive;
    }
    if (opt.state & QStyle::State_Selected) {
      painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
    } else {
      painter->setPen(opt.palette.color(cg, QPalette::Text));
    }
    if (opt.state & QStyle::State_Editing) {
      painter->setPen(opt.palette.color(cg, QPalette::Text));
      painter->drawRect(textArea.adjusted(0, 0, -1, -1));
    }

    // collect data for the various columns
    if(index.column() == (int)Column::Detail) {
      for(int i = 0; i < lines.count(); ++i) {
        painter->drawText(textArea.adjusted(0, (opt.fontMetrics.lineSpacing() + 5) * i, 0, 0), opt.displayAlignment, lines[i]);
      }

    } else {
      painter->drawText(textArea, opt.displayAlignment, opt.text);
    }
  }

  // draw the focus rect
  if(opt.state & QStyle::State_HasFocus) {
    QStyleOptionFocusRect o;
    o.QStyleOption::operator=(opt);
    o.rect = style->proxy()->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt, opt.widget);
    o.state |= QStyle::State_KeyboardFocusChange;
    o.state |= QStyle::State_Item;

    QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                              ? QPalette::Normal : QPalette::Disabled;
    o.backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                             ? QPalette::Highlight : QPalette::Window);
    style->proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, opt.widget);
  }

#if 0
  if((index.column() == LedgerModel::DetailColumn)
  && erroneous) {
    QPixmap attention;
    attention.loadFromData(attentionSign, sizeof(attentionSign), 0, 0);
    style->proxy()->drawItemPixmap(painter, option.rect, Qt::AlignRight | Qt::AlignTop, attention);
  }
#endif

  painter->restore();
#if 0
  const QHeaderView* horizontalHeader = view->horizontalHeader();
  const QHeaderView* verticalHeader = view->verticalHeader();
  const QWidget* viewport = view->viewport();
  const bool showGrid = view->showGrid() && !view->indexWidget(index);
  const int gridSize = showGrid ? 1 : 0;
  const int gridHint = style->styleHint(QStyle::SH_Table_GridLineColor, &option, view);
  const QColor gridColor = static_cast<QRgb>(gridHint);
  const QPen gridPen = QPen(gridColor, 0, view->gridStyle());
  const bool rightToLeft = view->isRightToLeft();
  const int viewportOffset = horizontalHeader->offset();


  // QStyledItemDelegate::paint(painter, opt, index);

  if(!horizontalHeader->isSectionHidden(LedgerModel::DateColumn)) {
    QDate postDate = index.data(LedgerModel::PostDateRole).toDate();
    if(postDate.isValid()) {
      int ofs = horizontalHeader->sectionViewportPosition(LedgerModel::DateColumn) + viewportOffset;
      QRect oRect = opt.rect;
      opt.displayAlignment = Qt::AlignLeft | Qt::AlignTop;
      opt.rect.setLeft(opt.rect.left()+ofs);
      opt.rect.setTop(opt.rect.top()+margin);
      opt.rect.setWidth(horizontalHeader->sectionSize(LedgerModel::DateColumn));
      opt.text = KGlobal::locale()->formatDate(postDate, QLocale::ShortFormat);
      style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
      opt.rect = oRect;
    }
  }

  if(!horizontalHeader->isSectionHidden(LedgerModel::DetailColumn)) {
    QString payee = index.data(LedgerModel::PayeeRole).toString();
    QString counterAccount = index.data(LedgerModel::CounterAccountRole).toString();
    QString txt = payee;
    if(payee.length() > 0)
      txt += '\n';
    txt += counterAccount;
    int ofs = horizontalHeader->sectionViewportPosition(LedgerModel::DetailColumn) + viewportOffset;
    QRect oRect = opt.rect;
    opt.displayAlignment = Qt::AlignLeft | Qt::AlignTop;
    opt.rect.setLeft(opt.rect.left()+ofs);
    opt.rect.setTop(opt.rect.top()+margin);
    opt.rect.setWidth(horizontalHeader->sectionSize(LedgerModel::DetailColumn));
    opt.text = txt;
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    opt.rect = oRect;

  }
#if 0
  opt.features |= QStyleOptionViewItemV2::HasDisplay;
  QString txt = QString("%1").arg(index.isValid() ? "true" : "false");
  if(index.isValid())
    txt += QString(" %1 - %2").arg(index.row()).arg(view->verticalHeader()->sectionViewportPosition(index.row()));
  opt.text = displayText(txt, opt.locale);

  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
#endif

  // paint grid
  if(showGrid) {
    painter->save();
    QPen old = painter->pen();
    painter->setPen(gridPen);

    // qDebug() << "Paint grid for" << index.row() << "in" << opt.rect;
    for(int i=0; i < horizontalHeader->count(); ++i) {
      if(!horizontalHeader->isSectionHidden(i)) {
        int ofs = horizontalHeader->sectionViewportPosition(i) + viewportOffset;
        if(!rightToLeft) {
          ofs += horizontalHeader->sectionSize(i) - gridSize;
        }
        if(ofs-viewportOffset < viewport->width()) {
          // I have no idea, why I need to paint the grid for the selected row and the one below
          // but it was the only way to get this working correctly. Otherwise the grid was missing
          // while moving the mouse over the view from bottom to top.
          painter->drawLine(opt.rect.x()+ofs, opt.rect.y(), opt.rect.x()+ofs, opt.rect.height());
          painter->drawLine(opt.rect.x()+ofs, opt.rect.y()+verticalHeader->sectionSize(index.row()), opt.rect.x()+ofs, opt.rect.height());
        }
      }
    }
    painter->setPen(old);
    painter->restore();
  }
#endif
}

QSize SplitDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  bool fullDisplay = false;
  LedgerView* view = qobject_cast<LedgerView*>(parent());
  if(view) {
    QModelIndex currentIndex = view->currentIndex();
    if(currentIndex.isValid()) {
      QString currentId = currentIndex.model()->data(currentIndex, (int)Role::TransactionSplitId).toString();
      QString myId = index.model()->data(index, (int)Role::TransactionSplitId).toString();
      fullDisplay = (currentId == myId);
    }
  }

  QSize size;
  QStyleOptionViewItem opt = option;
  if(index.isValid()) {
    // check if we are showing the edit widget
    const QAbstractItemView *viewWidget = qobject_cast<const QAbstractItemView *>(opt.widget);
    if (viewWidget) {
      QModelIndex editIndex = viewWidget->model()->index(index.row(), 0);
      if(editIndex.isValid()) {
        QWidget* editor = viewWidget->indexWidget(editIndex);
        if(editor) {
          size = editor->minimumSizeHint();
          return size;
        }
      }
    }
  }

  int rows = 1;
  if(fullDisplay) {
    initStyleOption(&opt, index);
    auto payee = index.data((int)Role::PayeeName).toString();
    auto account = index.data((int)Role::Account).toString();
    auto memo = index.data((int)Role::SingleLineMemo).toString();

    rows = (payee.length() > 0 ? 1 : 0) + (account.length() > 0 ? 1 : 0) + (memo.length() > 0 ? 1 : 0);
    // make sure we show at least one row
    if(!rows) {
      rows = 1;
    }
  }

  // leave a 5 pixel margin for each row
  size = QSize(100, (opt.fontMetrics.lineSpacing() + 5) * rows);
  return size;
}

void SplitDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(index);
  QStyleOptionViewItem opt = option;
  int ofs = 8;
  const LedgerView* view = qobject_cast<const LedgerView*>(opt.widget);
  if(view) {
    if(view->verticalScrollBar()->isVisible()) {
      ofs += view->verticalScrollBar()->width();
    }
  }

  QRect r(opt.rect);
  r.setWidth(opt.widget->width() - ofs);
  editor->setGeometry(r);
  editor->update();
}

void SplitDelegate::endEdit()
{
  if(d->m_editor) {
    if(d->m_editor->accepted()) {
      emit commitData(d->m_editor);
    }
    emit closeEditor(d->m_editor, NoHint);
    d->m_editorRow = -1;
  }
}

void SplitDelegate::setEditorData(QWidget* editWidget, const QModelIndex& index) const
{
  const SplitModel* model = qobject_cast<const SplitModel*>(index.model());
  NewSplitEditor* editor = qobject_cast<NewSplitEditor*>(editWidget);

  if(model && editor) {
    editor->setShowValuesInverted(d->m_showValuesInverted);
    editor->setMemo(model->data(index, (int)Role::Memo).toString());
    editor->setAccountId(model->data(index, (int)Role::AccountId).toString());
    editor->setAmount(model->data(index, (int)Role::SplitShares).value<MyMoneyMoney>());
    editor->setCostCenterId(model->data(index, (int)Role::CostCenterId).toString());
    editor->setNumber(model->data(index, (int)Role::Number).toString());
  }
}

void SplitDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  NewSplitEditor* splitEditor = qobject_cast< NewSplitEditor* >(editor);
  if(splitEditor) {
    model->setData(index, splitEditor->number(), (int)Role::Number);
    model->setData(index, splitEditor->memo(), (int)Role::Memo);
    model->setData(index, splitEditor->accountId(), (int)Role::AccountId);
    model->setData(index, splitEditor->costCenterId(), (int)Role::CostCenterId);
    model->setData(index, QVariant::fromValue<MyMoneyMoney>(splitEditor->amount()), (int)Role::SplitShares);
    model->setData(index, QVariant::fromValue<MyMoneyMoney>(splitEditor->amount()), (int)Role::SplitValue);

    const QString transactionCommodity = model->data(index, (int)Role::TransactionCommodity).toString();
    QModelIndex accIndex = Models::instance()->accountsModel()->accountById(splitEditor->accountId());
    if(accIndex.isValid()) {
      MyMoneyAccount acc = Models::instance()->accountsModel()->data(accIndex, (int)eAccountsModel::Role::Account).value<MyMoneyAccount>();
      if(transactionCommodity != acc.currencyId()) {
#if 0
        ///  @todo call KCurrencyConversionDialog and update the model data
        MyMoneyMoney value;
        model->setData(index, QVariant::fromValue<MyMoneyMoney>(value), SplitModel::SplitValueRole);
#endif
      }
    } else {
      qWarning() << "Unable to get account index in SplitDelegate::setModelData";
    }

    // the following forces to send a dataChanged signal
    model->setData(index, QVariant(), (int)Role::EmitDataChanged);

    // in case this was a new split, we nned to create a new empty one
    SplitModel* splitModel = qobject_cast<SplitModel*>(model);
    if(splitModel) {
      splitModel->addEmptySplitEntry();
    }
  }
}

/**
 * This eventfilter seems to do nothing but it prevents that selecting a
 * different row with the mouse closes the editor
 */
bool SplitDelegate::eventFilter(QObject* o, QEvent* event)
{
  return QAbstractItemDelegate::eventFilter(o, event);
}

void SplitDelegate::setShowValuesInverted(bool inverse)
{
  d->m_showValuesInverted = inverse;
}

bool SplitDelegate::showValuesInverted()
{
  return d->m_showValuesInverted;
}
