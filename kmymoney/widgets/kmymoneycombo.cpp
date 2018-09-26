/*
 * Copyright 2001       Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2002-2011  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kmymoneycombo.h"
#include "kmymoneycombo_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRect>
#include <QStyle>
#include <QPainter>
#include <QKeyEvent>
#include <QList>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QMetaMethod>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfig>
#include <KConfigGroup>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyselector.h"
#include "kmymoneycompletion.h"
#include "kmymoneylineedit.h"

KMyMoneyCombo::KMyMoneyCombo(QWidget *parent) :
    KComboBox(parent),
    d_ptr(new KMyMoneyComboPrivate)
{
}

KMyMoneyCombo::KMyMoneyCombo(bool rw, QWidget *parent) :
  KComboBox(rw, parent),
  d_ptr(new KMyMoneyComboPrivate)
{
  Q_D(KMyMoneyCombo);
  if (rw) {
    d->m_edit = new KMyMoneyLineEdit(this, true);
    setLineEdit(d->m_edit);
  }
}

KMyMoneyCombo::KMyMoneyCombo(KMyMoneyComboPrivate &dd, bool rw, QWidget *parent) :
  KComboBox(rw, parent),
  d_ptr(&dd)
{
  Q_D(KMyMoneyCombo);
  if (rw) {
    d->m_edit = new KMyMoneyLineEdit(this, true);
    setLineEdit(d->m_edit);
  }
}

KMyMoneyCombo::~KMyMoneyCombo()
{
  Q_D(KMyMoneyCombo);
  delete d;
}

void KMyMoneyCombo::setCurrentTextById(const QString& id)
{
  clearEditText();
  if (!id.isEmpty()) {
    QTreeWidgetItem* item = selector()->item(id);
    if (item) {
      setCompletedText(item->text(0));
      setEditText(item->text(0));
    }
  }
}

void KMyMoneyCombo::slotItemSelected(const QString& id)
{
  Q_D(KMyMoneyCombo);
  if (isEditable()) {
    bool blocked = signalsBlocked();
    blockSignals(true);
    setCurrentTextById(id);
    blockSignals(blocked);
  }

  d->m_completion->hide();

  if (d->m_id != id) {
    d->m_id = id;
    emit itemSelected(id);
  }
}

void KMyMoneyCombo::setEditable(bool y)
{
  Q_D(KMyMoneyCombo);
  if (y == isEditable())
    return;

  KComboBox::setEditable(y);

  // make sure we use our own line edit style
  if (y) {
    d->m_edit = new KMyMoneyLineEdit(this, true);
    setLineEdit(d->m_edit);
    d->m_edit->setPalette(palette());
  } else {
    d->m_edit = 0;
  }
}

void KMyMoneyCombo::setPlaceholderText(const QString& hint) const
{
  Q_D(const KMyMoneyCombo);
  if (d->m_edit)
    d->m_edit->setPlaceholderText(hint);
}

void KMyMoneyCombo::paintEvent(QPaintEvent* ev)
{
  Q_D(KMyMoneyCombo);
  KComboBox::paintEvent(ev);
  // if we don't have an edit field, we need to paint the text onto the button
  if (!d->m_edit) {
    if (d->m_completion) {
      QStringList list;
      selector()->selectedItems(list);
      if (!list.isEmpty()) {
        QString str = selector()->item(list[0])->text(0);
        // we only paint, if the text is longer than 1 char. Assumption
        // is that length 1 is the blank case so no need to do painting
        if (str.length() > 1) {
          QPainter p(this);
          p.setPen(palette().text().color());
          QStyleOptionComboBox opt;
          initStyleOption(&opt);
          QRect re = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
          p.setClipRect(re);
          p.save();
          p.setFont(font());
          QFontMetrics fm(font());
          int x = re.x(), y = re.y() + fm.ascent();
          p.drawText(x, y, str);
          p.restore();
        }
      }
    }
  }
}

void KMyMoneyCombo::mousePressEvent(QMouseEvent *e)
{
  Q_D(KMyMoneyCombo);
  // mostly copied from QCombo::mousePressEvent() and adjusted for our needs
  if (e->button() != Qt::LeftButton)
    return;

  if (((!isEditable() || isInArrowArea(e->globalPos())) && selector()->itemList().count()) && !d->m_completion->isVisible()) {
    d->m_completion->setVisible(true);
  }

  if (d->m_timer.isActive()) {
    d->m_timer.stop();
    d->m_completion->slotMakeCompletion(QString());
    // the above call clears the selection in the selector but maintains the current index, use that index to restore the selection
    QTreeWidget* listView = selector()->listView();
    QModelIndex currentIndex = listView->currentIndex();
    if (currentIndex.isValid()) {
      listView->selectionModel()->select(currentIndex, QItemSelectionModel::Select);
      listView->scrollToItem(listView->currentItem());
    }
  } else {
    KConfig config("kcminputrc");
    KConfigGroup grp = config.group("KDE");
    d->m_timer.setSingleShot(true);
    d->m_timer.start(grp.readEntry("DoubleClickInterval", 400));
  }
}

bool KMyMoneyCombo::isInArrowArea(const QPoint& pos) const
{
  QStyleOptionComboBox opt;
  initStyleOption(&opt);
  QRect arrowRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);

  // Correction for motif style, where arrow is smaller
  // and thus has a rect that doesn't fit the button.
  arrowRect.setHeight(qMax(height() - (2 * arrowRect.y()), arrowRect.height()));

  // if the button is not isEditable, it covers the whole widget
  if (!isEditable())
    arrowRect = rect();

  return arrowRect.contains(mapFromGlobal(pos));
}


void KMyMoneyCombo::setSuppressObjectCreation(bool suppress)
{
  Q_D(KMyMoneyCombo);
  d->m_canCreateObjects = !suppress;
}

void KMyMoneyCombo::setCurrentText(const QString& txt)
{
  KComboBox::setItemText(KComboBox::currentIndex(), txt);
}

void KMyMoneyCombo::setCurrentText()
{
  KComboBox::setItemText(KComboBox::currentIndex(), QString());
}

void KMyMoneyCombo::keyPressEvent(QKeyEvent* e)
{
  Q_D(KMyMoneyCombo);
  if ((e->key() == Qt::Key_F4 && e->modifiers() == 0) ||
      (e->key() == Qt::Key_Down && (e->modifiers() & Qt::AltModifier)) ||
      (!isEditable() && e->key() == Qt::Key_Space)) {
    // if we have at least one item in the list, we open the dropdown
    if (selector()->listView()->itemAt(0, 0))
      d->m_completion->setVisible(true);
    e->ignore();
    return;
  }
  KComboBox::keyPressEvent(e);
}

void KMyMoneyCombo::connectNotify(const QMetaMethod & signal)
{
  Q_D(KMyMoneyCombo);
  if (signal != QMetaMethod::fromSignal(&KMyMoneyCombo::createItem)) {
    d->m_canCreateObjects = true;
  }
}

void KMyMoneyCombo::disconnectNotify(const QMetaMethod & signal)
{
  Q_D(KMyMoneyCombo);
  if (signal != QMetaMethod::fromSignal(&KMyMoneyCombo::createItem)) {
    d->m_canCreateObjects = false;
  }
}

void KMyMoneyCombo::focusOutEvent(QFocusEvent* e)
{
  Q_D(KMyMoneyCombo);
  // don't do anything if the focus is lost due to window activation, this way switching
  // windows while typing a category will not popup the category creation dialog
  // also ignore the fact that the focus is lost because of Qt::PopupFocusReason (context menu)
  if (e->reason() == Qt::ActiveWindowFocusReason || e->reason() == Qt::PopupFocusReason)
    return;

  if (d->m_inFocusOutEvent) {
    KComboBox::focusOutEvent(e);
    return;
  }

  d->m_inFocusOutEvent = true;
  if (isEditable() && !currentText().isEmpty()) {
    if (d->m_canCreateObjects) {
      if (!d->m_completion->selector()->contains(currentText())) {
        QString id;
        // announce that we go into a possible dialog to create an object
        // This can be used by upstream widgets to disable filters etc.
        emit objectCreation(true);

        emit createItem(currentText(), id);

        // Announce that we return from object creation
        emit objectCreation(false);

        // update the field to a possibly created object
        d->m_id = id;
        setCurrentTextById(id);

        // make sure the completion does not show through
        d->m_completion->hide();
      }

      // else if we cannot create objects, and the current text is not
      // in the list, then we clear the text and the selection.
    } else if (!d->m_completion->selector()->contains(currentText())) {
      clearEditText();
    }
  }

  KComboBox::focusOutEvent(e);

  // force update of hint and id if there is no text in the widget
  if (isEditable() && currentText().isEmpty()) {
    QString id = d->m_id;
    d->m_id.clear();
    if (!id.isEmpty())
      emit itemSelected(d->m_id);
    update();
  }
  d->m_inFocusOutEvent = false;
}

KMyMoneySelector* KMyMoneyCombo::selector() const
{
  Q_D(const KMyMoneyCombo);
  return d->m_completion->selector();
}

KMyMoneyCompletion* KMyMoneyCombo::completion() const
{
  Q_D(const KMyMoneyCombo);
  return d->m_completion;
}

void KMyMoneyCombo::selectedItems(QStringList& list) const
{
  Q_D(const KMyMoneyCombo);
  if (lineEdit() && lineEdit()->text().length() == 0) {
    list.clear();
  } else {
    d->m_completion->selector()->selectedItems(list);
  }
}

QString KMyMoneyCombo::selectedItem() const
{
  Q_D(const KMyMoneyCombo);
  return d->m_id;
}

void KMyMoneyCombo::setSelectedItem(const QString& id)
{
  Q_D(KMyMoneyCombo);
  d->m_completion->selector()->setSelected(id, true);
  blockSignals(true);
  slotItemSelected(id);
  blockSignals(false);
  update();
}

QSize KMyMoneyCombo::sizeHint() const
{
  return KComboBox::sizeHint();

  // I wanted to use the code below to adjust the size of the combo box
  // according to the largest item in the selector list. Apparently that
  // does not work too well in the enter and edit schedule dialog for
  // the category combo box. So we just use the standard implementation for now.
#if 0
  constPolish();
  int i, w;
  QFontMetrics fm = fontMetrics();

  int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
  int maxH = qMax(fm.lineSpacing(), 14) + 2;

  w = selector()->optimizedWidth();
  if (w > maxW)
    maxW = w;

  QSize sizeHint = (style().sizeFromContents(QStyle::CT_ComboBox, this,
                    QSize(maxW, maxH)).
                    expandedTo(QApplication::globalStrut()));

  return sizeHint;
#endif
}
