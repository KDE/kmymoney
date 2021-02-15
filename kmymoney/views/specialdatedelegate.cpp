/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "specialdatedelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QScrollBar>
#include <QPainter>
#include <QDate>
#include <QSortFilterProxyModel>
#include <QColor>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "ledgerview.h"
#include "journalmodel.h"
#include "payeesmodel.h"
#include "newtransactioneditor.h"

QColor SpecialDateDelegate::m_separatorColor = QColor(0xff, 0xf2, 0x9b);




class SpecialDateDelegate::Private
{
public:
  Private()
  : m_lineHeight(12)
  , m_margin(2)

  {}

  ~Private()
  {
  }

  int                           m_lineHeight;
  int                           m_margin;
};


SpecialDateDelegate::SpecialDateDelegate(LedgerView* parent)
  : KMMStyledItemDelegate(parent)
  , d(new Private)
{
}

SpecialDateDelegate::~SpecialDateDelegate()
{
  delete d;
}

void SpecialDateDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // never change the background of the cell the mouse is hovering over
  opt.state &= ~QStyle::State_MouseOver;

  // never show the focus
  opt.state &= ~QStyle::State_HasFocus;

  // always draw it as selected and enabled but inactive
  opt.state |= QStyle::State_Selected;
  opt.state |= QStyle::State_Enabled;
  opt.state &= ~QStyle::State_Active;

  painter->save();

  QAbstractItemView* view = qobject_cast< QAbstractItemView* >(parent());

  // Background
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

  opt.rect.setX(opt.rect.x()-2);
  opt.rect.setWidth(opt.rect.width()+5);
  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

  switch (index.column()) {
    case JournalModel::Column::Detail:
      // adjust the rect to cover all columns
      if(view && view->viewport()) {
        opt.rect.setX(0);
        opt.rect.setWidth(view->viewport()->width());
      }
      // always paint in bold
      auto font = painter->font();
      font.setBold(true);
      painter->setFont(font);
      const auto idx = index.model()->index(index.row(), 0, index.parent());
      QString txt = idx.data(eMyMoney::Model::TransactionPostDateRole).toDate().toString(Qt::ISODate);
      painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
      painter->drawText(opt.rect, Qt::AlignCenter, idx.data(Qt::DisplayRole).toString());
      break;
  }

  painter->restore();
}

QSize SpecialDateDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  d->m_margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
  d->m_lineHeight = opt.fontMetrics.lineSpacing();

  QSize size(10, d->m_lineHeight + 2 * d->m_margin);

  return size;
}

/**
 * This eventfilter seems to do nothing but it prevents that selecting a
 * different row with the mouse closes the editor
 */
bool SpecialDateDelegate::eventFilter(QObject* o, QEvent* event)
{
  return QAbstractItemDelegate::eventFilter(o, event);
}
