/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kbicedit.h"

#include <QDebug>
#include <QApplication>
#include <QCompleter>
#include <QListView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyle>

#include "../bicmodel.h"
#include "bicvalidator.h"

class bicItemDelegate : public QStyledItemDelegate
{
public:
  explicit bicItemDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}
  void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
  inline QFont getSmallFont(const QStyleOptionViewItem& option) const;
};

KBicEdit::KBicEdit(QWidget* parent)
  : KLineEdit(parent)
{
  QCompleter* completer = new QCompleter(this);

  bicModel* model = new bicModel(this);
  completer->setModel(model);
  m_popupDelegate = new bicItemDelegate(this);
  completer->popup()->setItemDelegate(m_popupDelegate);

  setCompleter(completer);

  bicValidator *const validator = new bicValidator(this);
  setValidator(validator);
}

KBicEdit::~KBicEdit()
{
  delete m_popupDelegate;
}

QFont bicItemDelegate::getSmallFont(const QStyleOptionViewItem& option) const
{
  QFont smallFont = option.font;
  smallFont.setPointSize(0.9*smallFont.pointSize());
  return smallFont;
}

QSize bicItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  QFontMetrics metrics(option.font);
  QFontMetrics smallMetrics(getSmallFont(option));
  const QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

  // A bic has maximal 11 characters. So we guess, we want to display 11 characters. The name of the institution has to adapt to what is given
  return QSize(metrics.width(QLatin1Char('X')) + 2*margin, metrics.lineSpacing() + smallMetrics.lineSpacing() + smallMetrics.leading() + 2*margin);
}

/**
 * @todo enable eliding (use QFontMetrics::elidedText() )
 */
void bicItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // Background
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
  const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);

  // Paint name
  painter->save();
  QFont smallFont = getSmallFont(opt);
  QFontMetrics metrics(opt.font);
  QFontMetrics smallMetrics(smallFont);
  QRect nameRect = style->alignedRect(opt.direction, Qt::AlignBottom, QSize(textArea.width(), smallMetrics.lineSpacing()), textArea);
  painter->setFont(smallFont);
  style->drawItemText(painter, nameRect, Qt::AlignBottom, QApplication::palette(), true, index.model()->data(index, bicModel::InstitutionNameRole).toString(), option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Mid);
  painter->restore();

  // Paint BIC
  painter->save();
  QFont normal = painter->font();
  normal.setBold(true);
  painter->setFont(normal);
  QRect bicRect = style->alignedRect(opt.direction, Qt::AlignTop, QSize(textArea.width(), metrics.lineSpacing()), textArea);
  const QString bic = index.model()->data(index, Qt::DisplayRole).toString();
  style->drawItemText(painter, bicRect, Qt::AlignTop, QApplication::palette(), true, bic, option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);

  painter->restore();
}
