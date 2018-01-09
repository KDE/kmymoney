/***************************************************************************
                          ledgerdelegate.cpp
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
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

#include "ledgerdelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerview.h"
#include "ledgermodel.h"
#include "newtransactioneditor.h"

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

QColor LedgerDelegate::m_erroneousColor = QColor(Qt::red);
QColor LedgerDelegate::m_importedColor = QColor(Qt::yellow);
QColor LedgerDelegate::m_separatorColor = QColor(0xff, 0xf2, 0x9b);


class LedgerSeparatorDate : public LedgerSeparator
{
public:
  LedgerSeparatorDate(eLedgerModel::Role role);
  virtual ~LedgerSeparatorDate() {}

  virtual bool rowHasSeparator(const QModelIndex& index) const;
  virtual QString separatorText(const QModelIndex& index) const;
  virtual void adjustBackgroundScheme(QPalette& palette, const QModelIndex& index) const;

protected:
  QString getEntry(const QModelIndex& index, const QModelIndex& nextIndex) const;
  QMap<QDate, QString>      m_entries;
};

class LedgerSeparatorOnlineBalance : public LedgerSeparatorDate
{
public:
  LedgerSeparatorOnlineBalance(eLedgerModel::Role role);
  virtual ~LedgerSeparatorOnlineBalance() {}

  virtual bool rowHasSeparator(const QModelIndex& index) const;
  virtual QString separatorText(const QModelIndex& index) const;
  virtual void adjustBackgroundScheme(QPalette& palette, const QModelIndex& index) const;

  void setSeparatorData(const QDate& date, const MyMoneyMoney& amount, int fraction);

private:
  QString m_balanceAmount;
};



QDate LedgerSeparator::firstFiscalDate;
bool  LedgerSeparator::showFiscalDate = true;
bool  LedgerSeparator::showFancyDate = true;


void LedgerSeparator::setFirstFiscalDate(int firstMonth, int firstDay)
{
  firstFiscalDate = QDate(QDate::currentDate().year(), firstMonth, firstDay);
  if (QDate::currentDate() < firstFiscalDate)
    firstFiscalDate = firstFiscalDate.addYears(-1);
}

QModelIndex LedgerSeparator::nextIndex(const QModelIndex& index) const
{
  const int nextRow = index.row() + 1;
  if (index.isValid() && (nextRow < index.model()->rowCount(QModelIndex()))) {
    const QAbstractItemModel* model = index.model();
    return model->index(nextRow, 0, QModelIndex());
  }
  return QModelIndex();
}

LedgerSeparatorDate::LedgerSeparatorDate(eLedgerModel::Role role)
  : LedgerSeparator(role)
{
  const QDate today = QDate::currentDate();
  const QDate thisMonth(today.year(), today.month(), 1);
  const QDate lastMonth = thisMonth.addMonths(-1);
  const QDate yesterday = today.addDays(-1);
  // a = QDate::dayOfWeek()         todays weekday (1 = Monday, 7 = Sunday)
  // b = QLocale().firstDayOfWeek() first day of week (1 = Monday, 7 = Sunday)
  int weekStartOfs = today.dayOfWeek() - QLocale().firstDayOfWeek();
  if (weekStartOfs < 0) {
    weekStartOfs = 7 + weekStartOfs;
  }
  const QDate thisWeek = today.addDays(-weekStartOfs);
  const QDate lastWeek = thisWeek.addDays(-7);
  const QDate thisYear(today.year(), 1, 1);

  m_entries[thisYear] = i18n("This year");
  m_entries[lastMonth] = i18n("Last month");
  m_entries[thisMonth] = i18n("This month");
  m_entries[lastWeek] = i18n("Last week");
  m_entries[thisWeek] = i18n("This week");
  m_entries[yesterday] = i18n("Yesterday");
  m_entries[today] = i18n("Today");
  m_entries[today.addDays(1)] = i18n("Future transactions");
  m_entries[thisWeek.addDays(7)] = i18n("Next week");
  m_entries[thisMonth.addMonths(1)] = i18n("Next month");
  if (showFiscalDate && firstFiscalDate.isValid()) {
    m_entries[firstFiscalDate] = i18n("Current fiscal year");
    m_entries[firstFiscalDate.addYears(-1)] = i18n("Previous fiscal year");
    m_entries[firstFiscalDate.addYears(1)] = i18n("Next fiscal year");
  }
}


QString LedgerSeparatorDate::getEntry(const QModelIndex& index, const QModelIndex& nextIndex) const
{
  Q_ASSERT(index.isValid());
  Q_ASSERT(nextIndex.isValid());
  Q_ASSERT(index.model() == nextIndex.model());

  const QAbstractItemModel* model = index.model();
  QString rc;
  if(!m_entries.isEmpty()) {
    if (model->data(index, (int)m_role).toDate() != model->data(nextIndex, (int)m_role).toDate()) {
      const QDate key = model->data(index, (int)m_role).toDate();
      const QDate endKey = model->data(nextIndex, (int)m_role).toDate();
      QMap<QDate, QString>::const_iterator it = m_entries.upperBound(key);
      while((it != m_entries.cend()) && (it.key() <= endKey)) {
        rc = *it;
        ++it;
      }
    }
  }
  return rc;
}

bool LedgerSeparatorDate::rowHasSeparator(const QModelIndex& index) const
{
  bool rc = false;
  if(!m_entries.isEmpty()) {
    QModelIndex nextIdx = nextIndex(index);
    if(nextIdx.isValid() ) {
      const QString id = nextIdx.model()->data(nextIdx, (int)eLedgerModel::Role::TransactionSplitId).toString();
      // For a new transaction the id is completely empty, for a split view the transaction
      // part is filled but the split id is empty and the string ends with a dash
      // and we never draw a separator in front of that row
      if(!id.isEmpty() && !id.endsWith('-')) {
        rc = !getEntry(index, nextIdx).isEmpty();
      }
    }
  }
  return rc;
}

QString LedgerSeparatorDate::separatorText(const QModelIndex& index) const
{
  QModelIndex nextIdx = nextIndex(index);
  if(nextIdx.isValid()) {
    return getEntry(index, nextIdx);
  }
  return QString();
}

void LedgerSeparatorDate::adjustBackgroundScheme(QPalette& palette, const QModelIndex& index) const
{
  Q_UNUSED(index);
  KColorScheme::adjustBackground(palette, KColorScheme::ActiveBackground, QPalette::Base, KColorScheme::Button, KSharedConfigPtr());
}



LedgerSeparatorOnlineBalance::LedgerSeparatorOnlineBalance(eLedgerModel::Role role)
  : LedgerSeparatorDate(role)
{
  // we don't need the standard values
  m_entries.clear();
}

void LedgerSeparatorOnlineBalance::setSeparatorData(const QDate& date, const MyMoneyMoney& amount, int fraction)
{
  m_entries.clear();
  if (date.isValid()) {
    m_balanceAmount = amount.formatMoney(fraction);
    m_entries[date] = i18n("Online statement balance: %1", m_balanceAmount);
  }
}

bool LedgerSeparatorOnlineBalance::rowHasSeparator(const QModelIndex& index) const
{
  bool rc = false;
  if(!m_entries.isEmpty()) {
    QModelIndex nextIdx = nextIndex(index);
    const QAbstractItemModel* model = index.model();
    const QDate date = model->data(index, (int)m_role).toDate();
    // only a real transaction can have an online balance separator
    if(model->data(index, (int) eLedgerModel::Role::ScheduleId).toString().isEmpty()) {
      // if this is not the last entry and not a schedule?
      if(nextIdx.isValid()) {
        // index points to the last entry of a date
        rc = (date != model->data(nextIdx, (int)m_role).toDate());
        if (!rc) {
          // in case it's the same date, we need to check if this is the last real transaction
          // and the next one is a scheduled transaction
          if(!model->data(nextIdx, (int) eLedgerModel::Role::ScheduleId).toString().isEmpty() ) {
            rc = true;
          }
        }
        if (rc) {
          // check if this the spot for the online balance data
          rc &= ((date <= m_entries.firstKey())
            && (model->data(nextIdx, (int)m_role).toDate() >= m_entries.firstKey()));
        }
      } else {
        rc = (date <= m_entries.firstKey());
      }
    }
  }
  return rc;
}

QString LedgerSeparatorOnlineBalance::separatorText(const QModelIndex& index) const
{
  if(rowHasSeparator(index)) {
    return m_entries.first();
  }
  return QString();
}

void LedgerSeparatorOnlineBalance::adjustBackgroundScheme(QPalette& palette, const QModelIndex& index) const
{
  const QAbstractItemModel* model = index.model();
  QModelIndex amountIndex = model->index(index.row(), (int)eLedgerModel::Column::Balance);
  QString amount = model->data(amountIndex).toString();
  KColorScheme::BackgroundRole role = KColorScheme::PositiveBackground;

  if (!m_entries.isEmpty()) {
    if(amount != m_balanceAmount) {
      role = KColorScheme::NegativeBackground;
    }
  }
  KColorScheme::adjustBackground(palette, role, QPalette::Base, KColorScheme::Button, KSharedConfigPtr());
}




class LedgerDelegate::Private
{
public:
  Private()
  : m_editor(0)
  , m_view(0)
  , m_editorRow(-1)
  , m_separator(0)
  , m_onlineBalanceSeparator(0)
  {}

  ~Private()
  {
    delete m_separator;
  }

inline bool displaySeparator(const QModelIndex& index) const
  {
    return m_separator && m_separator->rowHasSeparator(index);
  }

  inline bool displayOnlineBalanceSeparator(const QModelIndex& index) const
  {
    return m_onlineBalanceSeparator && m_onlineBalanceSeparator->rowHasSeparator(index);
  }

  NewTransactionEditor*         m_editor;
  LedgerView*                   m_view;
  int                           m_editorRow;
  LedgerSeparator*              m_separator;
  LedgerSeparatorOnlineBalance* m_onlineBalanceSeparator;
};


LedgerDelegate::LedgerDelegate(LedgerView* parent)
  : QStyledItemDelegate(parent)
  , d(new Private)
{
  d->m_view = parent;
}

LedgerDelegate::~LedgerDelegate()
{
  delete d;
}

void LedgerDelegate::setSortRole(eLedgerModel::Role role)
{
  delete d->m_separator;
  delete d->m_onlineBalanceSeparator;
  d->m_separator = 0;
  d->m_onlineBalanceSeparator = 0;

  switch(role) {
    case eLedgerModel::Role::PostDate:
      d->m_separator = new LedgerSeparatorDate(role);
      d->m_onlineBalanceSeparator = new LedgerSeparatorOnlineBalance(role);
      break;
    default:
      qDebug() << "LedgerDelegate::setSortRole role" << (int)role << "not implemented";
      break;
  }
}

void LedgerDelegate::setErroneousColor(const QColor& color)
{
  m_erroneousColor = color;
}

void LedgerDelegate::setOnlineBalance(const QDate& date, const MyMoneyMoney& amount, int fraction)
{
  if(d->m_onlineBalanceSeparator) {
    d->m_onlineBalanceSeparator->setSeparatorData(date, amount, fraction);
  }
}

QWidget* LedgerDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option);

  if(index.isValid()) {
    if(d->m_view->selectionModel()->selectedRows().count() > 1) {
      qDebug() << "Editing multiple transactions at once is not yet supported";

      /**
       * @todo replace the following three lines with the creation of a special
       * editor that can handle multiple transactions at once
       */
      d->m_editor = 0;
      LedgerDelegate* const that = const_cast<LedgerDelegate* const>(this);
      emit that->closeEditor(d->m_editor, NoHint);

    } else {
      d->m_editor = new NewTransactionEditor(parent, d->m_view->accountId());
    }

    if(d->m_editor) {
      d->m_editorRow = index.row();
      connect(d->m_editor, SIGNAL(done()), this, SLOT(endEdit()));
      emit sizeHintChanged(index);
    }

  } else {
    qFatal("LedgerDelegate::createEditor(): we should never end up here");
  }
  return d->m_editor;
}

int LedgerDelegate::editorRow() const
{
  return d->m_editorRow;
}

void LedgerDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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
  const bool rowHasSeparator = d->displaySeparator(index);
  const bool rowHasOnlineBalance = d->displayOnlineBalanceSeparator(index);

  // Background
  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
  const int lineHeight = opt.fontMetrics.lineSpacing() + 2;

  if (rowHasSeparator) {
    // don't draw over the separator space
    opt.rect.setHeight(opt.rect.height() - lineHeight );
  }
  if (rowHasOnlineBalance) {
    // don't draw over the online balance space
    opt.rect.setHeight(opt.rect.height() - lineHeight );
  }

  style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

  QPalette::ColorGroup cg;

  // Do not paint text if the edit widget is shown
  if (!editWidgetIsVisible) {
    if(view && (index.column() == (int)eLedgerModel::Column::Detail)) {
      if(view->currentIndex().row() == index.row()) {
        opt.state |= QStyle::State_HasFocus;
      }
    }
    const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);
    const bool selected = opt.state & QStyle::State_Selected;

    QStringList lines;
    if(index.column() == (int)eLedgerModel::Column::Detail) {
      lines << index.model()->data(index, (int)eLedgerModel::Role::PayeeName).toString();
      if(selected) {
        lines << index.model()->data(index, (int)eLedgerModel::Role::CounterAccount).toString();
        lines << index.model()->data(index, (int)eLedgerModel::Role::SingleLineMemo).toString();

      } else {
        if(lines.at(0).isEmpty()) {
          lines.clear();
          lines << index.model()->data(index, (int)eLedgerModel::Role::SingleLineMemo).toString();
        }
        if(lines.at(0).isEmpty()) {
          lines << index.model()->data(index, (int)eLedgerModel::Role::CounterAccount).toString();
        }
      }
      lines.removeAll(QString());
    }

    const bool erroneous = index.model()->data(index, (int)eLedgerModel::Role::Erroneous).toBool();

    // draw the text items
    if(!opt.text.isEmpty() || !lines.isEmpty()) {

      // check if it is a scheduled transaction and display it as inactive
      if(!index.model()->data(index, (int)eLedgerModel::Role::ScheduleId).toString().isEmpty()) {
        opt.state &= ~QStyle::State_Enabled;
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
      if(index.column() == (int)eLedgerModel::Column::Detail) {
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
    if((index.column() == (int)eLedgerModel::Column::Detail)
    && erroneous) {
      QPixmap attention;
      attention.loadFromData(attentionSign, sizeof(attentionSign), 0, 0);
      style->proxy()->drawItemPixmap(painter, option.rect, Qt::AlignRight | Qt::AlignTop, attention);
    }
  }

  // draw a separator if any
  if (rowHasOnlineBalance) {
    opt.rect.setY(opt.rect.y() + opt.rect.height());
    opt.rect.setHeight(lineHeight);
    d->m_onlineBalanceSeparator->adjustBackgroundScheme(opt.palette, index);
    opt.backgroundBrush = opt.palette.base();

    // never draw it as selected but always enabled
    opt.state &= ~QStyle::State_Selected;
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    // when the editor is shown, the row has only a single column
    // so we need to paint the seperator if we get here in this casee
    bool needPaint = editWidgetIsVisible;

    if(!needPaint && (index.column() == (int)eLedgerModel::Column::Detail)) {
      needPaint = true;
      // adjust the rect to cover all columns
      if(view && view->viewport()) {
        opt.rect.setX(0);
        opt.rect.setWidth(view->viewport()->width());
      }
    }

    if(needPaint) {
      painter->setPen(opt.palette.color(QPalette::Normal, QPalette::Text));
      painter->drawText(opt.rect, Qt::AlignCenter, d->m_onlineBalanceSeparator->separatorText(index));
    }
  }

  if (rowHasSeparator) {
    opt.rect.setY(opt.rect.y() + opt.rect.height());
    opt.rect.setHeight(lineHeight);
    d->m_separator->adjustBackgroundScheme(opt.palette, index);
    opt.backgroundBrush = opt.palette.base();

    // never draw it as selected but always enabled
    opt.state &= ~QStyle::State_Selected;
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    // when the editor is shown, the row has only a single column
    // so we need to paint the seperator if we get here in this casee
    bool needPaint = editWidgetIsVisible;

    if(!needPaint && (index.column() == (int)eLedgerModel::Column::Detail)) {
      needPaint = true;
      // adjust the rect to cover all columns
      if(view && view->viewport()) {
        opt.rect.setX(0);
        opt.rect.setWidth(view->viewport()->width());
      }
    }

    if(needPaint) {
      painter->setPen(opt.palette.foreground().color());
      painter->drawText(opt.rect, Qt::AlignCenter, d->m_separator->separatorText(index));
    }
  }
  painter->restore();
}

QSize LedgerDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  bool fullDisplay = false;
  if(d->m_view) {
    QModelIndex currentIndex = d->m_view->currentIndex();
    if(currentIndex.isValid()) {
      QString currentId = currentIndex.model()->data(currentIndex, (int)eLedgerModel::Role::TransactionSplitId).toString();
      QString myId = index.model()->data(index, (int)eLedgerModel::Role::TransactionSplitId).toString();
      fullDisplay = (currentId == myId);
    }
  }

  QSize size;
  QStyleOptionViewItem opt = option;
  int rows = 1;
  initStyleOption(&opt, index);

  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
  const int lineHeight = opt.fontMetrics.lineSpacing();

  if(index.isValid()) {
    // check if we are showing the edit widget
    // const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(opt.widget);
    if (d->m_view) {
      QModelIndex editIndex = d->m_view->model()->index(index.row(), 0);
      if(editIndex.isValid()) {
        QWidget* editor = d->m_view->indexWidget(editIndex);
        if(editor) {
          size = editor->minimumSizeHint();
          if(d->displaySeparator(index)) {
            // don't draw over the separator space
            size += QSize(0, lineHeight + margin);
          }
          if(d->displayOnlineBalanceSeparator(index)) {
            // don't draw over the separator space
            size += QSize(0, lineHeight + margin);
          }
          return size;
        }
      }
    }
  }

  size = QSize(100, lineHeight + 2*margin);

  if(fullDisplay) {
    auto payee = index.data((int)eLedgerModel::Role::PayeeName).toString();
    auto counterAccount = index.data((int)eLedgerModel::Role::CounterAccount).toString();
    auto memo = index.data((int)eLedgerModel::Role::SingleLineMemo).toString();

    rows = (payee.length() > 0 ? 1 : 0) + (counterAccount.length() > 0 ? 1 : 0) + (memo.length() > 0 ? 1 : 0);
    // make sure we show at least one row
    if(!rows) {
      rows = 1;
    }
    // leave a few pixels as margin for each space between rows
    size.setHeight((size.height() * rows) - (margin * (rows - 1)));

  }

  if (d->m_separator && d->m_separator->rowHasSeparator(index)) {
    size.setHeight(size.height() + lineHeight + margin);
  }
  if (d->m_onlineBalanceSeparator && d->m_onlineBalanceSeparator->rowHasSeparator(index)) {
    size.setHeight(size.height() + lineHeight + margin);
  }
  return size;
}

void LedgerDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(index);

  QStyle *style = option.widget ? option.widget->style() : QApplication::style();
  const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
  const int lineHeight = option.fontMetrics.lineSpacing();

  int ofs = 8;
  if(d->m_view) {
    if(d->m_view->verticalScrollBar()->isVisible()) {
      ofs += d->m_view->verticalScrollBar()->width();
    }
  }

  QRect r(option.rect);
  r.setWidth(option.widget->width() - ofs);

  if(d->displaySeparator(index)) {
    // consider the separator space
    r.setHeight(r.height() - lineHeight - margin);
  }
  if(d->displayOnlineBalanceSeparator(index)) {
    // consider the separator space
    r.setHeight(r.height() - lineHeight - margin);
  }
  editor->setGeometry(r);
  editor->update();
}

void LedgerDelegate::endEdit()
{
  if(d->m_editor) {
    if(d->m_editor->accepted()) {
      emit commitData(d->m_editor);
    }
    emit closeEditor(d->m_editor, NoHint);
    d->m_editorRow = -1;
  }
}

/**
 * This eventfilter seems to do nothing but it prevents that selecting a
 * different row with the mouse closes the editor
 */
bool LedgerDelegate::eventFilter(QObject* o, QEvent* event)
{
  return QAbstractItemDelegate::eventFilter(o, event);
}

void LedgerDelegate::setEditorData(QWidget* editWidget, const QModelIndex& index) const
{
  NewTransactionEditor* editor = qobject_cast<NewTransactionEditor*>(editWidget);
  if(editor) {
    editor->loadTransaction(index.model()->data(index, (int)eLedgerModel::Role::TransactionSplitId).toString());
  }
}

void LedgerDelegate::setModelData(QWidget* editWidget, QAbstractItemModel* model, const QModelIndex& index) const
{
  Q_UNUSED(model)
  Q_UNUSED(index)

  NewTransactionEditor* editor = qobject_cast<NewTransactionEditor*>(editWidget);
  if(editor) {
    editor->saveTransaction();
  }
}
