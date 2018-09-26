/*
 * Copyright 2010-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2010-2016  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2010       Alvaro Soliverez <asoliverez@gmail.com>
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

#include "kmymoneymvccombo.h"
#include "kmymoneymvccombo_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QCompleter>
#include <QFocusEvent>
#include <QApplication>
#include <QMetaMethod>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes


KMyMoneyMVCCombo::KMyMoneyMVCCombo(QWidget* parent) :
    KComboBox(parent),
    d_ptr(new KMyMoneyMVCComboPrivate)
{
  view()->setAlternatingRowColors(true);
  connect(this, static_cast<void (KComboBox::*)(int)>(&KMyMoneyMVCCombo::KComboBox::activated), this, &KMyMoneyMVCCombo::activated);
}

KMyMoneyMVCCombo::KMyMoneyMVCCombo(bool editable, QWidget* parent) :
    KComboBox(editable, parent),
    d_ptr(new KMyMoneyMVCComboPrivate)
{
  Q_D(KMyMoneyMVCCombo);
  d->m_completer = new QCompleter(this);
  d->m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  d->m_completer->setModel(model());
  setCompleter(d->m_completer);

  // setSubstringSearch(!KMyMoneySettings::stringMatchFromStart());

  view()->setAlternatingRowColors(true);
  setInsertPolicy(QComboBox::NoInsert); // don't insert new objects due to object creation
  connect(this, static_cast<void (KComboBox::*)(int)>(&KMyMoneyMVCCombo::KComboBox::activated), this, &KMyMoneyMVCCombo::activated);
}

KMyMoneyMVCCombo::KMyMoneyMVCCombo(KMyMoneyMVCComboPrivate &dd, QWidget* parent) :
  KComboBox(parent),
  d_ptr(&dd)
{
  view()->setAlternatingRowColors(true);
  connect(this, static_cast<void (KComboBox::*)(int)>(&KMyMoneyMVCCombo::KComboBox::activated), this, &KMyMoneyMVCCombo::activated);
}

KMyMoneyMVCCombo::KMyMoneyMVCCombo(KMyMoneyMVCComboPrivate &dd, bool editable, QWidget* parent) :
  KComboBox(editable, parent),
  d_ptr(&dd)
{
  Q_D(KMyMoneyMVCCombo);
  d->m_completer = new QCompleter(this);
  d->m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  d->m_completer->setModel(model());
  setCompleter(d->m_completer);

  // setSubstringSearch(!KMyMoneySettings::stringMatchFromStart());

  view()->setAlternatingRowColors(true);
  setInsertPolicy(QComboBox::NoInsert); // don't insert new objects due to object creation
  connect(this, static_cast<void (KComboBox::*)(int)>(&KMyMoneyMVCCombo::KComboBox::activated), this, &KMyMoneyMVCCombo::activated);
}

KMyMoneyMVCCombo::~KMyMoneyMVCCombo()
{
  Q_D(KMyMoneyMVCCombo);
  delete d;
}

void KMyMoneyMVCCombo::setEditable(bool editable)
{
  Q_D(KMyMoneyMVCCombo);
  KComboBox::setEditable(editable);

  if(editable) {
    if(!d->m_completer) {
      d->m_completer = new QCompleter(this);
      d->m_completer->setCaseSensitivity(Qt::CaseInsensitive);
      d->m_completer->setModel(model());
    }
    setCompleter(d->m_completer);
  }
}

void KMyMoneyMVCCombo::setSubstringSearch(bool enabled)
{
  Q_D(KMyMoneyMVCCombo);
  d->m_completer->setCompletionMode(QCompleter::PopupCompletion);
  d->m_completer->setModel(model());
  d->m_completer->setFilterMode(enabled ? Qt::MatchContains : Qt::MatchStartsWith);
}

void KMyMoneyMVCCombo::setSubstringSearchForChildren(QWidget*const widget, bool enabled)
{
  Q_CHECK_PTR(widget);
  QList<KMyMoneyMVCCombo *> comboList;
  comboList = widget->findChildren<KMyMoneyMVCCombo *>();
  foreach (KMyMoneyMVCCombo *combo, comboList) {
    combo->setSubstringSearch(enabled);
  }
}

void KMyMoneyMVCCombo::setPlaceholderText(const QString& hint) const
{
  KLineEdit* le = qobject_cast<KLineEdit*>(lineEdit());
  if (le) {
    le->setPlaceholderText(hint);
  }
}

QString KMyMoneyMVCCombo::selectedItem() const
{
  Q_D(const KMyMoneyMVCCombo);
  auto dataVariant = itemData(currentIndex());
  if (dataVariant.isValid())
    d->m_id = dataVariant.toString();
  else
    d->m_id.clear();
  return d->m_id;
}

void KMyMoneyMVCCombo::setSelectedItem(const QString& id)
{
  Q_D(KMyMoneyMVCCombo);
  d->m_id = id;
  setCurrentIndex(findData(QVariant(d->m_id)));
}

void KMyMoneyMVCCombo::activated(int index)
{
  Q_D(KMyMoneyMVCCombo);
  auto dataVariant = itemData(index);
  if (dataVariant.isValid()) {
    d->m_id = dataVariant.toString();
    emit itemSelected(d->m_id);
  }
}

void KMyMoneyMVCCombo::connectNotify(const QMetaMethod & signal)
{
  Q_D(KMyMoneyMVCCombo);
  if (signal != QMetaMethod::fromSignal(&KMyMoneyMVCCombo::createItem)) {
    d->m_canCreateObjects = true;
  }
}

void KMyMoneyMVCCombo::disconnectNotify(const QMetaMethod & signal)
{
  Q_D(KMyMoneyMVCCombo);
  if (signal != QMetaMethod::fromSignal(&KMyMoneyMVCCombo::createItem)) {
    d->m_canCreateObjects = false;
  }
}

void KMyMoneyMVCCombo::setCurrentText(const QString& txt)
{
  KComboBox::setItemText(KComboBox::currentIndex(), txt);
}

void KMyMoneyMVCCombo::setCurrentText()
{
  KComboBox::setItemText(KComboBox::currentIndex(), QString());
}


void KMyMoneyMVCCombo::focusOutEvent(QFocusEvent* e)
{
  Q_D(KMyMoneyMVCCombo);
  // when showing m_completion we'll receive a focus out event even if the focus
  // will still remain at this widget since this widget is the completion's focus proxy
  // so ignore the focus out event caused by showing a widget of type Qt::Popup
  if (e->reason() == Qt::PopupFocusReason)
    return;

  if (d->m_inFocusOutEvent) {
    KComboBox::focusOutEvent(e);
    return;
  }

  //check if the focus went to a widget in TransactionFrom or in the Register
  if (e->reason() == Qt::MouseFocusReason) {
    QObject *w = this->parent();
    QObject *q = qApp->focusWidget()->parent();
    // KMyMoneyTagCombo is inside KTagContainer, KMyMoneyPayeeCombo isn't it
    if (w->inherits("KTagContainer"))
      w = w->parent();
    while (q && q->objectName() != "qt_scrollarea_viewport")
      q = q->parent();
    if (q != w && qApp->focusWidget()->objectName() != "register") {
      KComboBox::focusOutEvent(e);
      return;
    }
  }

  d->m_inFocusOutEvent = true;
  if (isEditable() && !currentText().isEmpty() && e->reason() != Qt::ActiveWindowFocusReason) {
    if (d->m_canCreateObjects) {
      // in case we tab out, we make sure that if the current completion
      // contains the current text that we set the current text to
      // the full completion text but only if the completion box is visible.
      // BUG 254984 is resolved with the visibility check
      if (e->reason() != Qt::MouseFocusReason) {
        if (d->m_completer->popup() && d->m_completer->popup()->isVisible()
            && d->m_completer->currentCompletion().contains(currentText(), Qt::CaseInsensitive)) {
          lineEdit()->setText(d->m_completer->currentCompletion());
        }
      }

      //check if the current text is contained in the internal list, if not ask the user if want to create a new item.
      checkCurrentText();

      // else if we cannot create objects, and the current text is not
      // in the list, then we clear the text and the selection.
    } else if (!contains(currentText())) {
      clearEditText();
    }
    //this is to cover the case when you highlight an item but don't activate it with Enter
    if (currentText() != itemText(currentIndex())) {
      setCurrentIndex(findText(currentText(), Qt::MatchExactly));
      emit activated(currentIndex());
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
  // This is used only be KMyMoneyTagCombo at this time
  emit lostFocus();
}

void KMyMoneyMVCCombo::checkCurrentText()
{
  Q_D(KMyMoneyMVCCombo);
  if (!contains(currentText())) {
    QString id;
    // announce that we go into a possible dialog to create an object
    // This can be used by upstream widgets to disable filters etc.
    emit objectCreation(true);

    emit createItem(currentText(), id);

    // Announce that we return from object creation
    emit objectCreation(false);

    // update the field to a possibly created object
    d->m_id = id;
    addEntry(currentText(), id);
    setCurrentTextById(id);
  }
}

void KMyMoneyMVCCombo::addEntry(const QString& newTxt, const QString& id)
{
    // find the correct position in the list
    int idx;
    for(idx = 0; idx < model()->rowCount(); ++idx) {
      const QString txt = itemText(idx);
      if (txt.compare(newTxt) > 0) {
        break;
      }
    }
    // and insert the new item
    insertItem(idx - 1, QIcon(), currentText(), id);
}

void KMyMoneyMVCCombo::setCurrentTextById(const QString& id)
{
  clearEditText();
  if (!id.isEmpty()) {
    int index = findData(QVariant(id), Qt::UserRole, Qt::MatchExactly);
    if (index > -1) {
      setCompletedText(itemText(index));
      setEditText(itemText(index));
      setCurrentIndex(index);
    }
  }
}

void KMyMoneyMVCCombo::protectItem(int id, bool protect)
{
  QStandardItemModel* standardModel = qobject_cast<QStandardItemModel*> (model());
  QStandardItem* standardItem = standardModel->item(id);
  standardItem->setSelectable(!protect);
}
