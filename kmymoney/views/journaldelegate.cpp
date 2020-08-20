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

#include "journaldelegate.h"

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
#include "schedulesjournalmodel.h"
#include "accountsmodel.h"
#include "payeesmodel.h"
#include "newtransactioneditor.h"
#include "investtransactioneditor.h"
#include "mymoneyutils.h"
#include "mymoneysecurity.h"

static unsigned char attentionSign[] = {
  0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
  0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
  0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x14,
  0x08, 0x06, 0x00, 0x00, 0x00, 0x8D, 0x89, 0x1D,
  0x0D, 0x00, 0x00, 0x00, 0x04, 0x73, 0x42, 0x49,
  0x54, 0x08, 0x08, 0x08, 0x08, 0x7C, 0x08, 0x64,
  0x88, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58,
  0x74, 0x53, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72,
  0x65, 0x00, 0x77, 0x77, 0x77, 0x2E, 0x69, 0x6E,
  0x6B, 0x73, 0x63, 0x61, 0x70, 0x65, 0x2E, 0x6F,
  0x72, 0x67, 0x9B, 0xEE, 0x3C, 0x1A, 0x00, 0x00,
  0x02, 0x05, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8D,
  0xAD, 0x95, 0xBF, 0x4B, 0x5B, 0x51, 0x14, 0xC7,
  0x3F, 0x2F, 0xBC, 0x97, 0x97, 0x97, 0x97, 0x77,
  0xF3, 0xF2, 0x1C, 0xA4, 0x54, 0x6B, 0x70, 0x10,
  0x44, 0x70, 0x2A, 0x91, 0x2E, 0x52, 0x02, 0x55,
  0x8A, 0xB5, 0xA3, 0xAB, 0x38, 0x08, 0x66, 0xCC,
  0xEE, 0xE0, 0xE2, 0x20, 0xB8, 0x38, 0xB8, 0xB8,
  0xF8, 0x1F, 0x38, 0x29, 0xA5, 0x29, 0x74, 0x90,
  0x0E, 0x0D, 0x0E, 0x22, 0x1D, 0x44, 0xA8, 0xD0,
  0xD4, 0xB4, 0x58, 0x4B, 0x09, 0xF9, 0xF1, 0x4A,
  0x3B, 0xD4, 0xD3, 0xE1, 0x55, 0xD3, 0x34, 0xAF,
  0x49, 0x6C, 0x3D, 0xF0, 0x85, 0x7B, 0xCF, 0xFD,
  0x9E, 0xEF, 0x3D, 0xE7, 0xFE, 0xD4, 0x44, 0x84,
  0xDB, 0xB4, 0x48, 0x2F, 0xA4, 0x94, 0xAB, 0xE5,
  0x52, 0xAE, 0x96, 0xEB, 0x49, 0x51, 0x44, 0x3A,
  0x02, 0x18, 0x88, 0xC7, 0xF1, 0xE3, 0x71, 0x7C,
  0x60, 0xA0, 0x1B, 0xBF, 0x6B, 0x86, 0x49, 0xC5,
  0x46, 0x3E, 0x47, 0x34, 0x9F, 0x23, 0x9A, 0x54,
  0x6C, 0xFC, 0x57, 0x86, 0x40, 0xC6, 0x4B, 0xE1,
  0x37, 0xCA, 0x48, 0xA3, 0x8C, 0x78, 0x29, 0x7C,
  0x20, 0xD3, 0x31, 0xA6, 0xD3, 0xA0, 0x52, 0x1C,
  0x6D, 0x6F, 0x72, 0xD9, 0x28, 0x23, 0xFE, 0x07,
  0x64, 0x7B, 0x93, 0x4B, 0xA5, 0x38, 0xFA, 0x27,
  0x41, 0x60, 0x6E, 0x74, 0x84, 0x7A, 0xE5, 0x1D,
  0x92, 0x54, 0x88, 0xE7, 0x22, 0xD5, 0x12, 0x32,
  0x3A, 0x42, 0x1D, 0x98, 0xBB, 0x91, 0x20, 0x60,
  0xDA, 0x36, 0x17, 0xFB, 0x7B, 0xC8, 0xC1, 0x4B,
  0x04, 0x02, 0xBC, 0x7E, 0x81, 0xEC, 0xEF, 0x21,
  0xB6, 0xCD, 0x05, 0x60, 0xF6, 0x2C, 0x68, 0x9A,
  0x2C, 0xCF, 0x4C, 0xE1, 0x4B, 0x05, 0x39, 0x3F,
  0x69, 0x0A, 0xBE, 0x7F, 0x83, 0x48, 0x05, 0x99,
  0x99, 0xC2, 0x37, 0x4D, 0x96, 0x7B, 0x12, 0x04,
  0xFA, 0x2D, 0x8B, 0xC6, 0xE9, 0x61, 0x10, 0x2C,
  0x15, 0xC4, 0x8A, 0x21, 0x86, 0x8E, 0xFC, 0xF8,
  0x12, 0xF4, 0x4F, 0x0F, 0x11, 0xCB, 0xA2, 0x01,
  0xF4, 0x77, 0x3D, 0x36, 0x4E, 0x82, 0xF5, 0xA5,
  0x05, 0x8C, 0xE1, 0x74, 0xD3, 0x37, 0x34, 0x18,
  0x20, 0xF2, 0x8B, 0x3D, 0x9C, 0x86, 0xA5, 0x05,
  0x0C, 0x27, 0xC1, 0x7A, 0xC7, 0x63, 0x03, 0x8C,
  0x2B, 0x07, 0xBF, 0x5A, 0x6A, 0x66, 0x27, 0x15,
  0x64, 0x3A, 0x8B, 0x3C, 0x7A, 0xD8, 0xEA, 0xAB,
  0x96, 0x10, 0xE5, 0xE0, 0x03, 0xE3, 0x7F, 0xCD,
  0x50, 0x39, 0x6C, 0xAD, 0xAD, 0x10, 0x53, 0xAA,
  0x75, 0xD2, 0xF4, 0xBD, 0x00, 0x2D, 0x5C, 0x05,
  0x6B, 0x2B, 0xC4, 0x94, 0xC3, 0xD6, 0xEF, 0xFE,
  0x6B, 0x41, 0x4D, 0xD3, 0x66, 0xFB, 0x3C, 0xC6,
  0x16, 0xE7, 0xDB, 0x97, 0x61, 0xE2, 0x3E, 0x3C,
  0xC8, 0xB4, 0x15, 0xC7, 0xE2, 0x3C, 0x91, 0x3E,
  0x8F, 0x31, 0x4D, 0xD3, 0x66, 0x5B, 0x4A, 0x06,
  0x8C, 0x84, 0xCD, 0x59, 0x61, 0xA7, 0xB5, 0xAC,
  0x2B, 0x9C, 0x1C, 0x04, 0x08, 0x1B, 0x2B, 0xEC,
  0x20, 0x09, 0x9B, 0x33, 0xC0, 0xB8, 0xDE, 0x65,
  0x43, 0x27, 0x9F, 0x9D, 0xA4, 0x1E, 0x16, 0xF0,
  0xF9, 0x6D, 0xB0, 0xC3, 0x86, 0x1E, 0xB4, 0xC3,
  0x38, 0xD9, 0x49, 0xEA, 0x86, 0x4E, 0xFE, 0xEA,
  0x29, 0xF4, 0x2C, 0x8B, 0xDA, 0x71, 0x31, 0x9C,
  0xFC, 0xF5, 0x23, 0x32, 0x34, 0x88, 0xDC, 0xBD,
  0x13, 0x5C, 0xBF, 0x30, 0xCE, 0x71, 0x11, 0xB1,
  0x2C, 0x6A, 0x80, 0xA7, 0xDB, 0x36, 0xAB, 0x4F,
  0xA6, 0x89, 0xBA, 0x49, 0x38, 0xFF, 0xD4, 0xBE,
  0x4E, 0x00, 0xAF, 0x9E, 0x81, 0x08, 0xD4, 0xEA,
  0x01, 0xFE, 0x34, 0x37, 0x09, 0x4F, 0x1F, 0x13,
  0xDD, 0x7D, 0xCE, 0xAA, 0x96, 0x72, 0x29, 0x7C,
  0xFB, 0xCE, 0x44, 0xB8, 0xD4, 0xCD, 0x2C, 0x66,
  0x52, 0xD4, 0x6E, 0xFB, 0x0B, 0xF8, 0x09, 0x63,
  0x63, 0x31, 0xE4, 0x85, 0x76, 0x2E, 0x0E, 0x00,
  0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE,
  0x42, 0x60, 0x82
};

QColor JournalDelegate::m_erroneousColor = QColor(Qt::red);
QColor JournalDelegate::m_importedColor = QColor(Qt::yellow);
QColor JournalDelegate::m_separatorColor = QColor(0xff, 0xf2, 0x9b);




class JournalDelegate::Private
{
public:
  Private()
  : m_editor(nullptr)
  , m_view(nullptr)
  , m_editorRow(-1)
  , m_singleLineRole(eMyMoney::Model::SplitPayeeRole)
  , m_lineHeight(-1)
  , m_margin(2)

  {}

  ~Private()
  {
  }

  QStringList displayString(const QModelIndex& index, const QStyleOptionViewItem& opt)
  {
    QStringList lines;
    if(index.column() == JournalModel::Column::Detail) {
      if (index.data(eMyMoney::Model::TransactionIsInvestmentRole).toBool()) {
        if(opt.state & QStyle::State_Selected) {
          lines << index.data(eMyMoney::Model::SplitActivityRole).toString();
          lines << index.data(eMyMoney::Model::TransactionBrokerageAccountRole).toString();
          lines << index.data(eMyMoney::Model::TransactionInterestCategoryRole).toString();
          lines << index.data(eMyMoney::Model::TransactionFeesCategoryRole).toString();
          lines << index.data(eMyMoney::Model::SplitSingleLineMemoRole).toString();
        } else {
          lines << index.data(eMyMoney::Model::SplitActivityRole).toString();
        }

      } else {
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
      }
      lines.removeAll(QString());

    } else if(index.column() == JournalModel::Column::Quantity) {
      if (index.data(eMyMoney::Model::TransactionIsInvestmentRole).toBool()) {
        lines << opt.text;
        if(opt.state & QStyle::State_Selected) {
          // we have to pay attention here as later on empty items will be removed
          // from the lines all together. Since we use the column detail as label
          // we have to make that we are not off. Therefor, if the detail column
          // is filled, we add a simple blank here instead of an empty line.
          // The first line is always present, so we make sure it is not empty in this column.
          if (lines[0].isEmpty())
            lines[0] = QStringLiteral(" ");
          lines << (index.data(eMyMoney::Model::TransactionBrokerageAccountRole).toString().isEmpty() ? QString() : QStringLiteral(" "));

          MyMoneySecurity currency = MyMoneyFile::instance()->currency(index.data(eMyMoney::Model::TransactionCommodityRole).toString());

          if (index.data(eMyMoney::Model::TransactionInterestSplitPresentRole).toBool()) {
            const auto value = index.data(eMyMoney::Model::TransactionInterestValueRole).value<MyMoneyMoney>();
            lines << (index.data(eMyMoney::Model::TransactionInterestCategoryRole).toString().isEmpty() ? QString() : MyMoneyUtils::formatMoney(value.abs(), currency));
          }

          if (index.data(eMyMoney::Model::TransactionFeeSplitPresentRole).toBool()) {
            const auto value = index.data(eMyMoney::Model::TransactionFeesValueRole).value<MyMoneyMoney>();
            lines << (index.data(eMyMoney::Model::TransactionFeesCategoryRole).toString().isEmpty() ? QString() : MyMoneyUtils::formatMoney(value.abs(), currency));
          }
        } else {
          lines << opt.text;
        }
      }
      lines.removeAll(QString());

    } else {
      lines << opt.text;
    }
    return lines;
  }

  TransactionEditorBase*        m_editor;
  LedgerView*                   m_view;
  int                           m_editorRow;
  eMyMoney::Model::Roles        m_singleLineRole;
  int                           m_lineHeight;
  int                           m_margin;
  int                           m_editorWidthOfs;
};


JournalDelegate::JournalDelegate(LedgerView* parent)
  : KMMStyledItemDelegate(parent)
  , d(new Private)
{
  d->m_view = parent;
}

JournalDelegate::~JournalDelegate()
{
  delete d;
}

void JournalDelegate::setErroneousColor(const QColor& color)
{
  m_erroneousColor = color;
}

void JournalDelegate::setSingleLineRole(eMyMoney::Model::Roles role)
{
  d->m_singleLineRole = role;
}

QWidget* JournalDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);

  if(index.isValid()) {
    if(d->m_view->selectionModel()->selectedRows().count() > 1) {
      qDebug() << "Editing multiple transactions at once is not yet supported";

      /**
       * @todo replace the following three lines with the creation of a special
       * editor that can handle multiple transactions at once
       */
      d->m_editor = nullptr;
      JournalDelegate* that = const_cast<JournalDelegate*>(this);
      emit that->closeEditor(d->m_editor, NoHint);

    } else {
      auto accountId = index.data(eMyMoney::Model::SplitAccountIdRole).toString();
      if (accountId.isEmpty() || (accountId == MyMoneyFile::instance()->journalModel()->fakeId())) {
        accountId = d->m_view->accountId();
      }
      if (!accountId.isEmpty()) {
        // now determine which editor to use. In case we have no transaction (yet)
        // we use the account type
        if (index.data(eMyMoney::Model::JournalTransactionIdRole).toString().isEmpty()) {
          const auto acc = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
          if (acc.accountType() == eMyMoney::Account::Type::Investment) {
            d->m_editor = new InvestTransactionEditor(parent, accountId);
          } else {
            d->m_editor = new NewTransactionEditor(parent, accountId);
          }
        } else {
          if (index.data(eMyMoney::Model::TransactionIsInvestmentRole).toBool()) {
            // in case of an investment transaction we need to use
            // the parent account of the security account and pass
            // it to the editor.
            accountId = index.data(eMyMoney::Model::TransactionInvestmentAccountIdRole).toString();
            d->m_editor = new InvestTransactionEditor(parent, accountId);
          } else {
            d->m_editor = new NewTransactionEditor(parent, accountId);
          }
        }
        d->m_editorWidthOfs = 8;
        if(d->m_view) {
          if(d->m_view->verticalScrollBar()->isVisible()) {
            d->m_editorWidthOfs += d->m_view->verticalScrollBar()->width();
          }
        }

      } else {
        qDebug() << "Unable to determine account for editing";

        d->m_editor = nullptr;
        JournalDelegate* that = const_cast<JournalDelegate*>(this);
        emit that->closeEditor(d->m_editor, NoHint);
      }
    }

    if(d->m_editor) {
      d->m_editorRow = index.row();
      connect(d->m_editor, &TransactionEditorBase::done, this, &JournalDelegate::endEdit);
      JournalDelegate* that = const_cast<JournalDelegate*>(this);
      emit that->sizeHintChanged(index);
    }

  } else {
    qFatal("JournalDelegate::createEditor(): we should never end up here");
  }
  return d->m_editor;
}

int JournalDelegate::editorRow() const
{
  return d->m_editorRow;
}

void JournalDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);

  // never change the background of the cell the mouse is hovering over
  opt.state &= ~QStyle::State_MouseOver;

  // show the focus only on the detail column
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

  // Do not paint text if the edit widget is shown
  if (!editWidgetIsVisible) {
    bool isOverdue = false;
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

      // check if it is a scheduled transaction and display it as inactive
      if (MyMoneyFile::baseModel()->baseModel(index) == MyMoneyFile::instance()->schedulesJournalModel()) {
        opt.state &= ~QStyle::State_Enabled;
        isOverdue = index.data(eMyMoney::Model::ScheduleIsOverdueRole).toBool();
      }
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
      for(int i = 0; i < lines.count(); ++i) {
        painter->drawText(textArea.adjusted(0, lineHeight * i, 0, 0), opt.displayAlignment, lines[i]);
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
    && (erroneous || isOverdue)) {
      QPixmap attention;
      attention.loadFromData(attentionSign, sizeof(attentionSign), 0, 0);
      style->proxy()->drawItemPixmap(painter, option.rect, Qt::AlignRight | Qt::AlignTop, attention);
    }
  }

  painter->restore();
}

QSize JournalDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // get parameters only once per update to speed things up
  if (d->m_lineHeight == -1) {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    d->m_margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    d->m_lineHeight = opt.fontMetrics.lineSpacing();
  }
  int rows = 1;

  if(index.isValid()) {
    // check if we are showing the edit widget
    // const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(opt.widget);
    if (d->m_view) {
      QModelIndex editIndex = d->m_view->model()->index(index.row(), 0);
      if(editIndex.isValid()) {
        QWidget* editor = d->m_view->indexWidget(editIndex);
        if(editor) {
          return editor->minimumSizeHint();
        }
      }
    }
  }

  QSize size(10, d->m_lineHeight + 2 * d->m_margin);

  if(option.state & QStyle::State_Selected) {
    rows = d->displayString(index, option).count();

    // make sure we show at least one row
    if(!rows) {
      rows = 1;
    }
    // leave a few pixels as margin for each space between rows
    size.setHeight((size.height() * rows) - (d->m_margin * (rows - 1)));
  }
  return size;
}

void JournalDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(index);

  QRect r(option.rect);
  if (option.widget)
    r.setWidth(option.widget->width() - d->m_editorWidthOfs);

  editor->setGeometry(r);
  editor->update();
}

void JournalDelegate::endEdit()
{
  if(d->m_editor) {
    if(d->m_editor->accepted()) {
      emit commitData(d->m_editor);
    }
    emit closeEditor(d->m_editor, NoHint);
    d->m_editorRow = -1;
    d->m_editor = nullptr;
  }
}

/**
 * This eventfilter seems to do nothing but it prevents that selecting a
 * different row with the mouse closes the editor
 */
bool JournalDelegate::eventFilter(QObject* o, QEvent* event)
{
  return QAbstractItemDelegate::eventFilter(o, event);
}

void JournalDelegate::setEditorData(QWidget* editWidget, const QModelIndex& index) const
{
  auto* editor = qobject_cast<TransactionEditorBase*>(editWidget);
  if(editor) {
    editor->loadTransaction(index);
  }
}

void JournalDelegate::setModelData(QWidget* editWidget, QAbstractItemModel* model, const QModelIndex& index) const
{
  Q_UNUSED(model)
  Q_UNUSED(index)

  auto* editor = qobject_cast<TransactionEditorBase*>(editWidget);
  if(editor) {
    // saving the transaction may move the selected transaction(s) around
    // we keep the transaction IDs here and take care of them when we return
    const auto selection = d->m_view->selectedTransactions();

    editor->saveTransaction();

    d->m_view->setSelectedTransactions(selection);
  }
}
