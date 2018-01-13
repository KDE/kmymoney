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

#include "ibanbicitemdelegate.h"

#include <QApplication>
#include <QPainter>
#include <QAbstractItemView>

#include <KLocalizedString>

#include "models/payeeidentifiercontainermodel.h"
#include "ibanbicitemedit.h"

ibanBicItemDelegate::ibanBicItemDelegate(QObject* parent, const QVariantList&)
    : QStyledItemDelegate(parent)
{

}

/** @todo elide texts */
void ibanBicItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // Background
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
  const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);

  // Do not paint text if the edit widget is shown
  const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(opt.widget);
  if (view && view->indexWidget(index))
    return;

  // Get data
  payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBic = ibanBicByIndex(index);

  // Paint Bic
  painter->save();
  const QFont smallFont = painter->font();
  const QFontMetrics metrics(opt.font);
  const QFontMetrics smallMetrics(smallFont);
  const QRect bicRect = style->alignedRect((opt.direction  == Qt::RightToLeft) ? Qt::LeftToRight : Qt::RightToLeft, Qt::AlignTop, QSize(textArea.width(), smallMetrics.lineSpacing()),
                        QRect(textArea.left(), metrics.lineSpacing() + textArea.top(), textArea.width(), smallMetrics.lineSpacing())
                                          );
  painter->setFont(smallFont);
  style->drawItemText(painter, bicRect, Qt::AlignBottom | Qt::AlignRight, QApplication::palette(), true, ibanBic->storedBic(), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();

  // Paint Bank name
  painter->save();
  const QRect nameRect = style->alignedRect(opt.direction, Qt::AlignTop, QSize(textArea.width(), smallMetrics.lineSpacing()),
                         QRect(textArea.left(), metrics.lineSpacing() + textArea.top(), textArea.width(), smallMetrics.lineSpacing())
                                           );
  style->drawItemText(painter, nameRect, Qt::AlignBottom, QApplication::palette(), true, ibanBic->institutionName(), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();

  // Paint IBAN
  painter->save();
  QFont normal = painter->font();
  normal.setBold(true);
  painter->setFont(normal);
  const QRect ibanRect = style->alignedRect(opt.direction, Qt::AlignTop, QSize(textArea.width(), metrics.lineSpacing()), textArea);
  const QString bic = index.model()->data(index, Qt::DisplayRole).toString();
  style->drawItemText(painter, ibanRect, Qt::AlignTop, QApplication::palette(), true, ibanBic->paperformatIban(), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();

  // Paint type
  painter->save();
  QRect typeRect = style->alignedRect(opt.direction, Qt::AlignTop | Qt::AlignRight, QSize(textArea.width() / 5, metrics.lineSpacing()), textArea);
  style->drawItemText(painter, typeRect, Qt::AlignTop | Qt::AlignRight, QApplication::palette(), true, i18n("IBAN & BIC"), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();
}

QSize ibanBicItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // Test if current index is edited at the moment
  const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(opt.widget);
  if (view && view->indexWidget(index))
    return view->indexWidget(index)->sizeHint();

  QFontMetrics metrics(option.font);
  const QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

  // A bic has maximal 11 characters, an IBAN 32
  return QSize((32 + 11)*metrics.width(QLatin1Char('X')) + 3*margin, 3*metrics.lineSpacing() + metrics.leading() + 2*margin);
}

QWidget* ibanBicItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);
  ibanBicItemEdit* edit = new ibanBicItemEdit(parent);
  connect(edit, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(edit, SIGNAL(closeEditor(QWidget*)), this, SIGNAL(closeEditor(QWidget*)));
  emit sizeHintChanged(index);
  return edit;
}

void ibanBicItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBic = ibanBicByIndex(index);
  ibanBicItemEdit* ibanEditor = qobject_cast< ibanBicItemEdit* >(editor);
  Q_CHECK_PTR(ibanEditor);

  ibanEditor->setIdentifier(ibanBic);
}

void ibanBicItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  Q_CHECK_PTR(editor);
  Q_CHECK_PTR(model);
  Q_ASSERT(index.isValid());

  ibanBicItemEdit* ibanEditor = qobject_cast< ibanBicItemEdit* >(editor);
  Q_CHECK_PTR(ibanEditor);

  model->setData(index, QVariant::fromValue<payeeIdentifier>(ibanEditor->identifier()), payeeIdentifierContainerModel::payeeIdentifier);
}

void ibanBicItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(index);
  editor->setGeometry(option.rect);
}

/**
 * Internal helper to direcly convert the QVariant into the correct pointer type.
 */
payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBicItemDelegate::ibanBicByIndex(const QModelIndex& index) const
{
  payeeIdentifierTyped<payeeIdentifiers::ibanBic> ibanBic{
        index.model()->data(index, payeeIdentifierContainerModel::payeeIdentifier).value<payeeIdentifier>()
      };
  Q_ASSERT(!ibanBic.isNull());
  return ibanBic;
}
