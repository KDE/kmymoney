/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Christian David <c.david@christian-david.de>
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

#include <QDebug>
#include <KLocalizedString>

#include "models/payeeidentifiermodel.h"
#include "ibanbicitemedit.h"

ibanBicItemDelegate::ibanBicItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{

}

/** @todo elide texts */
void ibanBicItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItemV4 opt = option;
  initStyleOption(&opt, index);

  //qDebug() << "row" << index.row() << "paint in " << opt.rect;

  // Background
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
  const QRect textArea = QRect(opt.rect.x()+margin, opt.rect.y()+margin, opt.rect.width()-2*margin, opt.rect.height()-2*margin);

  // Get data
  payeeIdentifiers::ibanBic::constPtr ibanBic = ibanBicByIndex( index );

  // Paint Bic
  painter->save();
  QFont smallFont = painter->font();
  QFontMetrics metrics( opt.font );
  QFontMetrics smallMetrics( smallFont );
  QRect nameRect = style->alignedRect(opt.direction, Qt::AlignBottom, QSize(textArea.width(), smallMetrics.lineSpacing()), textArea);
  painter->setFont( smallFont );
  style->drawItemText( painter, nameRect, Qt::AlignBottom, QApplication::palette(), true, ibanBic->storedBic(), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text );
  painter->restore();

  // Paint BIC
  painter->save();
  QFont normal = painter->font();
  normal.setBold(true);
  painter->setFont( normal );
  QRect bicRect = style->alignedRect(opt.direction, Qt::AlignTop, QSize(textArea.width(), metrics.lineSpacing()), textArea);
  const QString bic = index.model()->data(index, Qt::DisplayRole).toString();
  style->drawItemText(painter, bicRect, Qt::AlignTop, QApplication::palette(), true, ibanBic->paperformatIban(), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();

  // Paint type
  painter->save();
  QRect typeRect = style->alignedRect( opt.direction, Qt::AlignTop | Qt::AlignRight, QSize(textArea.width()/5, metrics.lineSpacing()), textArea);
  style->drawItemText( painter, typeRect, Qt::AlignTop | Qt::AlignRight, QApplication::palette(), true, i18n("IBAN & BIC"), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();

  emit sizeHintChanged(index);
}

QSize ibanBicItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItemV4 opt = option;
  initStyleOption(&opt, index);

  if (opt.state.testFlag(QStyle::State_HasFocus)) {
    qDebug() << "Got QStyle::State_HasFocus in sizeHint!";
  }

  // Set allways to true because QStyle::State_Editing is never set (seems to be a bug in Qt)
  if ( true || opt.state.testFlag(QStyle::State_Editing) ) {
    ibanBicItemEdit* edit = new ibanBicItemEdit();
    QSize hint = edit->sizeHint();
    delete edit;
    return hint;
  }

  QFontMetrics metrics(option.font);
  const QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

  // A bic has maximal 11 characters, an IBAN 32
  //qDebug() << "row" <<  index.row() << "size hint" << QSize( (32+11)*metrics.width(QLatin1Char('X')) + 3*margin, 2*metrics.lineSpacing() + metrics.leading() + 2*margin );
  return QSize( (32+11)*metrics.width(QLatin1Char('X')) + 3*margin, 2*metrics.lineSpacing() + metrics.leading() + 2*margin );
}

QWidget* ibanBicItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);
  ibanBicItemEdit* edit = new ibanBicItemEdit(parent);
  emit sizeHintChanged(index);
  return edit;
}

void ibanBicItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  payeeIdentifiers::ibanBic::constPtr ibanBic = ibanBicByIndex(index);
  ibanBicItemEdit* ibanEditor = qobject_cast< ibanBicItemEdit* >(editor);
  Q_CHECK_PTR( ibanEditor );

  ibanEditor->setBic( ibanBic->storedBic() );
  ibanEditor->setIban( ibanBic->electronicIban() );
}

void ibanBicItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  Q_CHECK_PTR( editor );
  Q_CHECK_PTR( model );
  Q_ASSERT( index.isValid() );

  ibanBicItemEdit* ibanEditor = qobject_cast< ibanBicItemEdit* >(editor);
  Q_CHECK_PTR( ibanEditor );

  payeeIdentifiers::ibanBic::ptr ibanBic = ibanBicByIndex(index)->cloneSharedPtr();
  ibanBic->setBic( ibanEditor->bic() );
  ibanBic->setIban( ibanEditor->iban() );
  model->setData(index, QVariant::fromValue<payeeIdentifier::ptr>( ibanBic ), payeeIdentifierModel::payeeIdentifierPtr);
}

void ibanBicItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}

/**
 * Internal helper to direcly convert the QVariant into the correct pointer type.
 */
payeeIdentifiers::ibanBic::constPtr ibanBicItemDelegate::ibanBicByIndex(const QModelIndex& index) const
{
  const payeeIdentifiers::ibanBic::constPtr ibanBic = index.model()->data(index, payeeIdentifierModel::payeeIdentifierPtr).value<payeeIdentifier::constPtr>().staticCast<const payeeIdentifiers::ibanBic>();
  Q_ASSERT( !ibanBic.isNull() );
  return ibanBic;
}
