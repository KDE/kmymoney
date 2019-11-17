/*
 * Copyright 2015-2019  Thomas Baumgart <tbaumgart@kde.org>
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
  : m_editor(nullptr)
  , m_view(nullptr)
  , m_lineHeight(12)
  , m_margin(2)

  {}

  ~Private()
  {
  }

  NewTransactionEditor*         m_editor;
  LedgerView*                   m_view;
  int                           m_lineHeight;
  int                           m_margin;
};


SpecialDateDelegate::SpecialDateDelegate(LedgerView* parent)
  : QStyledItemDelegate(parent)
  , d(new Private)
{
  d->m_view = parent;
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

  // if selected, always show as active, so that the
  // background does not change when the editor is shown
  if (opt.state & QStyle::State_Selected) {
    opt.state |= QStyle::State_Active;
  }
  // never draw it as selected but always enabled
  opt.state &= ~QStyle::State_Selected;
  opt.state |= QStyle::State_Enabled;

  painter->save();

  QAbstractItemView* view = qobject_cast< QAbstractItemView* >(parent());

  // Background
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
  const int lineHeight = opt.fontMetrics.lineSpacing() + 2;

  const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);

  QPalette::ColorGroup cg;

  KColorScheme::BackgroundRole role = KColorScheme::PositiveBackground; // : KColorScheme::NegativeBackground;

  KColorScheme::adjustBackground(opt.palette, role, QPalette::Base, KColorScheme::View, KSharedConfigPtr());
  // opt.rect.setHeight(lineHeight);
  opt.backgroundBrush = opt.palette.base();

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
      const auto idx = index.model()->index(index.row(), 0, index.parent());
      QString txt = idx.data(eMyMoney::Model::TransactionPostDateRole).toDate().toString(Qt::ISODate);
      painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
      painter->drawText(opt.rect, Qt::AlignCenter, QString("Special Date: %1 - %2").arg(txt, idx.data(Qt::DisplayRole).toString()));
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
