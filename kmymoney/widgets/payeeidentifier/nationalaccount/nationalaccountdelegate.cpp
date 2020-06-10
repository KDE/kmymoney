/*
 * Copyright 2014-2015  Christian Dávid <christian-david@web.de>
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

#include "nationalaccountdelegate.h"

#include <QApplication>
#include <QPainter>
#include <QAbstractItemView>

#include <KLocalizedString>

#include "models/payeeidentifiercontainermodel.h"
#include "nationalaccountedit.h"

nationalAccountDelegate::nationalAccountDelegate(QObject* parent, const QVariantList&)
    : QStyledItemDelegate(parent)
{

}

/** @todo elide texts */
void nationalAccountDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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
  payeeIdentifierTyped<payeeIdentifiers::nationalAccount> ident = identByIndex(index);

  // Paint bank code
  painter->save();
  const QFont smallFont = painter->font();
  const QFontMetrics metrics(opt.font);
  const QFontMetrics smallMetrics(smallFont);
  const QRect bicRect = style->alignedRect(opt.direction, Qt::AlignTop, QSize(textArea.width(), smallMetrics.lineSpacing()),
                        QRect(textArea.left(), metrics.lineSpacing() + textArea.top(), textArea.width(), smallMetrics.lineSpacing())
                                          );
  painter->setFont(smallFont);
  style->drawItemText(painter, bicRect, Qt::AlignBottom, QApplication::palette(), true, ident->bankCode(), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();

  // Paint bank name
  painter->save();
  const QRect nameRect = style->alignedRect(opt.direction, Qt::AlignTop, QSize(textArea.width(), smallMetrics.lineSpacing()),
                         QRect(textArea.left(), metrics.lineSpacing() + smallMetrics.lineSpacing() + textArea.top(), textArea.width(), smallMetrics.lineSpacing())
                                           );
  style->drawItemText(painter, nameRect, Qt::AlignBottom, QApplication::palette(), true, ident->bankName(), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();

  // Paint account number
  painter->save();
  QFont normal = painter->font();
  normal.setBold(true);
  painter->setFont(normal);
  const QRect ibanRect = style->alignedRect(opt.direction, Qt::AlignTop, QSize(textArea.width(), metrics.lineSpacing()), textArea);
  style->drawItemText(painter, ibanRect, Qt::AlignTop, QApplication::palette(), true, ident->accountNumber(), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();

  // Paint type
  painter->save();
  QRect typeRect = style->alignedRect(opt.direction, Qt::AlignTop | Qt::AlignRight, QSize(textArea.width() / 5, metrics.lineSpacing()), textArea);
  style->drawItemText(painter, typeRect, Qt::AlignTop | Qt::AlignRight, QApplication::palette(), true, i18n("National Account"), opt.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);
  painter->restore();
}

QSize nationalAccountDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // QStyle::State_Editing is never set (seems to be a bug in Qt)! This code is here only because it was written already
  const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(opt.widget);
  if (view && view->indexWidget(index))
    return view->indexWidget(index)->sizeHint();

  QFontMetrics metrics(option.font);
  const QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

  // An iban has maximal 32 characters, so national accounts should be shorter than 28
  return QSize((28)*metrics.width(QLatin1Char('X')) + 2*margin, 3*metrics.lineSpacing() + metrics.leading() + 2*margin);
}

QWidget* nationalAccountDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);
  nationalAccountEdit* edit = new nationalAccountEdit(parent);
  connect(edit, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
  connect(edit, SIGNAL(closeEditor(QWidget*)), this, SIGNAL(closeEditor(QWidget*)));
  emit sizeHintChanged(index);
  return edit;
}

void nationalAccountDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  nationalAccountEdit* nationalEditor = qobject_cast< nationalAccountEdit* >(editor);
  Q_CHECK_PTR(nationalEditor);

  nationalEditor->setIdentifier(identByIndex(index));
}

void nationalAccountDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  Q_CHECK_PTR(editor);
  Q_CHECK_PTR(model);
  Q_ASSERT(index.isValid());

  nationalAccountEdit* nationalEditor = qobject_cast< nationalAccountEdit* >(editor);
  Q_CHECK_PTR(nationalEditor);

  payeeIdentifierTyped<payeeIdentifiers::nationalAccount> ident = identByIndex(index);
  ident->setAccountNumber(nationalEditor->accountNumber());
  ident->setBankCode(nationalEditor->institutionCode());
  model->setData(index, QVariant::fromValue<payeeIdentifier>(ident), payeeIdentifierContainerModel::payeeIdentifier);
}

void nationalAccountDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(index);
  editor->setGeometry(option.rect);
}

/**
 * Internal helper to directly convert the QVariant into the correct pointer type.
 */
payeeIdentifierTyped<payeeIdentifiers::nationalAccount> nationalAccountDelegate::identByIndex(const QModelIndex& index) const
{
  payeeIdentifierTyped<payeeIdentifiers::nationalAccount> ident = payeeIdentifierTyped<payeeIdentifiers::nationalAccount>(
        index.model()->data(index, payeeIdentifierContainerModel::payeeIdentifier).value<payeeIdentifier>()
      );

  Q_ASSERT(!ident.isNull());
  return ident;
}
