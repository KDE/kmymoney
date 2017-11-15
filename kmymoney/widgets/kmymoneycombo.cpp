/***************************************************************************
                          kmymoneycombo.cpp  -  description
                             -------------------
    begin                : Mon Mar 12 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneycombo.h"

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

KMyMoneyCombo::KMyMoneyCombo(QWidget *w) :
    KComboBox(w),
    m_completion(0),
    m_edit(0),
    m_canCreateObjects(false),
    m_inFocusOutEvent(false)
{
}

KMyMoneyCombo::KMyMoneyCombo(bool rw, QWidget *w) :
    KComboBox(rw, w),
    m_completion(0),
    m_edit(0),
    m_canCreateObjects(false),
    m_inFocusOutEvent(false)
{
  if (rw) {
    m_edit = new kMyMoneyLineEdit(this, "combo edit");
    setLineEdit(m_edit);
  }
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
  if (isEditable()) {
    bool blocked = signalsBlocked();
    blockSignals(true);
    setCurrentTextById(id);
    blockSignals(blocked);
  }

  m_completion->hide();

  if (m_id != id) {
    m_id = id;
    emit itemSelected(id);
  }
}

void KMyMoneyCombo::setEditable(bool y)
{
  if (y == isEditable())
    return;

  KComboBox::setEditable(y);

  // make sure we use our own line edit style
  if (y) {
    m_edit = new kMyMoneyLineEdit(this, "combo edit");
    setLineEdit(m_edit);
    m_edit->setPalette(palette());
  } else {
    m_edit = 0;
  }
}

void KMyMoneyCombo::setPlaceholderText(const QString& hint) const
{
  if (m_edit)
    m_edit->setPlaceholderText(hint);
}

void KMyMoneyCombo::paintEvent(QPaintEvent* ev)
{
  KComboBox::paintEvent(ev);
  // if we don't have an edit field, we need to paint the text onto the button
  if (!m_edit) {
    if (m_completion) {
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
  // mostly copied from QCombo::mousePressEvent() and adjusted for our needs
  if (e->button() != Qt::LeftButton)
    return;

  if (((!isEditable() || isInArrowArea(e->globalPos())) && selector()->itemList().count()) && !m_completion->isVisible()) {
    m_completion->setVisible(true);
  }

  if (m_timer.isActive()) {
    m_timer.stop();
    m_completion->slotMakeCompletion("");
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
    m_timer.setSingleShot(true);
    m_timer.start(grp.readEntry("DoubleClickInterval", 400));
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

void KMyMoneyCombo::keyPressEvent(QKeyEvent* e)
{
  if ((e->key() == Qt::Key_F4 && e->modifiers() == 0) ||
      (e->key() == Qt::Key_Down && (e->modifiers() & Qt::AltModifier)) ||
      (!isEditable() && e->key() == Qt::Key_Space)) {
    // if we have at least one item in the list, we open the dropdown
    if (selector()->listView()->itemAt(0, 0))
      m_completion->setVisible(true);
    e->ignore();
    return;
  }
  KComboBox::keyPressEvent(e);
}

void KMyMoneyCombo::connectNotify(const QMetaMethod & signal)
{
  if (signal != QMetaMethod::fromSignal(&KMyMoneyCombo::createItem)) {
    m_canCreateObjects = true;
  }
}

void KMyMoneyCombo::disconnectNotify(const QMetaMethod & signal)
{
  if (signal != QMetaMethod::fromSignal(&KMyMoneyCombo::createItem)) {
    m_canCreateObjects = false;
  }
}

void KMyMoneyCombo::focusOutEvent(QFocusEvent* e)
{
  // don't do anything if the focus is lost due to window activation, this way switching
  // windows while typing a category will not popup the category creation dialog
  // also ignore the fact that the focus is lost because of Qt::PopupFocusReason (context menu)
  if (e->reason() == Qt::ActiveWindowFocusReason || e->reason() == Qt::PopupFocusReason)
    return;

  if (m_inFocusOutEvent) {
    KComboBox::focusOutEvent(e);
    return;
  }

  m_inFocusOutEvent = true;
  if (isEditable() && !currentText().isEmpty()) {
    if (m_canCreateObjects) {
      if (!m_completion->selector()->contains(currentText())) {
        QString id;
        // annouce that we go into a possible dialog to create an object
        // This can be used by upstream widgets to disable filters etc.
        emit objectCreation(true);

        emit createItem(currentText(), id);

        // Announce that we return from object creation
        emit objectCreation(false);

        // update the field to a possibly created object
        m_id = id;
        setCurrentTextById(id);

        // make sure the completion does not show through
        m_completion->hide();
      }

      // else if we cannot create objects, and the current text is not
      // in the list, then we clear the text and the selection.
    } else if (!m_completion->selector()->contains(currentText())) {
      clearEditText();
    }
  }

  KComboBox::focusOutEvent(e);

  // force update of hint and id if there is no text in the widget
  if (isEditable() && currentText().isEmpty()) {
    QString id = m_id;
    m_id.clear();
    if (!id.isEmpty())
      emit itemSelected(m_id);
    update();
  }
  m_inFocusOutEvent = false;
}

KMyMoneySelector* KMyMoneyCombo::selector() const
{
  return m_completion->selector();
}

kMyMoneyCompletion* KMyMoneyCombo::completion() const
{
  return m_completion;
}

void KMyMoneyCombo::selectedItems(QStringList& list) const
{
  if (lineEdit() && lineEdit()->text().length() == 0) {
    list.clear();
  } else {
    m_completion->selector()->selectedItems(list);
  }
}

void KMyMoneyCombo::setSelectedItem(const QString& id)
{
  m_completion->selector()->setSelected(id, true);
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
