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

#include "onlinebalancedelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QDate>
#include <QSortFilterProxyModel>

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

QColor OnlineBalanceDelegate::m_erroneousColor = QColor(Qt::red);
QColor OnlineBalanceDelegate::m_importedColor = QColor(Qt::yellow);
QColor OnlineBalanceDelegate::m_separatorColor = QColor(0xff, 0xf2, 0x9b);




class OnlineBalanceDelegate::Private
{
public:
  Private()
  : m_editor(nullptr)
  , m_view(nullptr)
  , m_editorRow(-1)
  , m_singleLineRole(eMyMoney::Model::SplitPayeeRole)
  , m_lineHeight(12)
  , m_margin(2)

  {}

  ~Private()
  {
  }

  QStringList displayString(const QModelIndex& index, const QStyleOptionViewItem& opt)
  {
    QStringList lines;
    if(index.column() == JournalModel::Column::Detail) {
      lines << index.data(m_singleLineRole).toString();
      if(opt.state & QStyle::State_Selected) {
        lines.clear();
        lines << index.data(eMyMoney::Model::Roles::SplitPayeeRole).toString();
        lines << index.data(eMyMoney::Model::Roles::TransactionCounterAccountRole).toString();
        lines << index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();

      } else {
        if(lines.at(0).isEmpty()) {
          lines.clear();
          lines << index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();
        }
        if(lines.at(0).isEmpty()) {
          lines << index.data(eMyMoney::Model::Roles::TransactionCounterAccountRole).toString();
        }
      }
      lines.removeAll(QString());

    } else {
      lines << opt.text;
    }
    return lines;
  }

  NewTransactionEditor*         m_editor;
  LedgerView*                   m_view;
  int                           m_editorRow;
  eMyMoney::Model::Roles        m_singleLineRole;
  int                           m_lineHeight;
  int                           m_margin;
};


OnlineBalanceDelegate::OnlineBalanceDelegate(LedgerView* parent)
  : QStyledItemDelegate(parent)
  , d(new Private)
{
  d->m_view = parent;
}

OnlineBalanceDelegate::~OnlineBalanceDelegate()
{
  delete d;
}

void OnlineBalanceDelegate::setErroneousColor(const QColor& color)
{
  m_erroneousColor = color;
}

void OnlineBalanceDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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

  painter->save();

  QAbstractItemView* view = qobject_cast< QAbstractItemView* >(parent());
  const bool editWidgetIsVisible = d->m_view && d->m_view->indexWidget(index);

  // Background
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
  const int lineHeight = opt.fontMetrics.lineSpacing() + 2;

  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

  QPalette::ColorGroup cg;

  KColorScheme::BackgroundRole role = KColorScheme::PositiveBackground;

#if 0
  role = KColorScheme::NegativeBackground;
#endif
  KColorScheme::adjustBackground(opt.palette, role, QPalette::Base, KColorScheme::Button, KSharedConfigPtr());

  // opt.rect.setHeight(lineHeight);
  opt.backgroundBrush = opt.palette.base();

  // never draw it as selected but always enabled
  opt.state &= ~QStyle::State_Selected;
  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

  if(index.column() == (int)JournalModel::Column::Detail) {
      // adjust the rect to cover all columns
      if(view && view->viewport()) {
        opt.rect.setX(0);
        opt.rect.setWidth(view->viewport()->width());
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
      }
      painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
      painter->drawText(opt.rect, Qt::AlignCenter, "Online Balance");
  }

#if 0
  // Do not paint text if the edit widget is shown
  if (!editWidgetIsVisible) {
    if(view && (index.column() == JournalModel::Column::Detail)) {
      if(view->currentIndex().row() == index.row()) {
        opt.state |= QStyle::State_HasFocus;
      }
    }
    const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);
    const bool selected = opt.state & QStyle::State_Selected;

    QStringList lines = d->displayString(index, opt);

    const bool erroneous = index.data(eMyMoney::Model::Roles::TransactionErroneousRole).toBool();

    // draw the text items
    if(!opt.text.isEmpty() || !lines.isEmpty()) {

      /// @todo port to new model code
#if 0
      // check if it is a scheduled transaction and display it as inactive
      if(!index.model()->data(index, (int)eLedgerModel::Role::ScheduleId).toString().isEmpty()) {
        opt.state &= ~QStyle::State_Enabled;
      }
#endif
      cg = (opt.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;

      if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
      }
      if (selected) {
        // always use the normal palette since the background is also in normal
        painter->setPen(opt.palette.color(QPalette::ColorGroup(QPalette::Normal), QPalette::HighlightedText));

      } else if (erroneous) {
        painter->setPen(m_erroneousColor);

      } else {
        painter->setPen(opt.palette.color(cg, QPalette::Text));
      }

      if (opt.state & QStyle::State_Editing) {
        painter->setPen(opt.palette.color(cg, QPalette::Text));
        painter->drawRect(textArea.adjusted(0, 0, -1, -1));
      }

      // collect data for the various columns
      if(index.column() == JournalModel::Column::Detail) {
        for(int i = 0; i < lines.count(); ++i) {
          painter->drawText(textArea.adjusted(0, lineHeight * i, 0, 0), opt.displayAlignment, lines[i]);
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

      cg = (opt.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
      o.backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                              ? QPalette::Highlight : QPalette::Window);
      style->proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, opt.widget);
    }

    // draw the attention mark
    if((index.column() == JournalModel::Column::Detail)
    && erroneous) {
      QPixmap attention;
      attention.loadFromData(attentionSign, sizeof(attentionSign), 0, 0);
      style->proxy()->drawItemPixmap(painter, option.rect, Qt::AlignRight | Qt::AlignTop, attention);
    }
  }
#endif
  painter->restore();
}

QSize OnlineBalanceDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
#if 0
  // get parameters only once per update to speed things up
  if (index.row() == 0) {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    d->m_margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    d->m_lineHeight = opt.fontMetrics.lineSpacing();
  }
#endif
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
bool OnlineBalanceDelegate::eventFilter(QObject* o, QEvent* event)
{
  return QAbstractItemDelegate::eventFilter(o, event);
}
