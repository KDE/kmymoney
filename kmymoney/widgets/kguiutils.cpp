/*
 * Copyright 2006-2010  Tony Bloomfield <tonybloom@users.sourceforge.net>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kguiutils.h"

// ----------------------------------------------------------------------------
// QT Includes
// No need for QDateEdit, QSpinBox, etc., since these always return values

#include <QCheckBox>
#include <QPushButton>
#include <QWidget>
#include <QSpinBox>
#include <QApplication>
#include <QListWidget>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>
#include <KLineEdit>
#include <KUrlRequester>
#include "kmymoneyedit.h"

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"
#include "onlinetasks/interfaces/ui/ionlinejobedit.h"
#include "kmymoneytextedit.h"
#include "kmymoneypayeecombo.h"

/**************************************************************************
 *                                                                        *
 * The MandatoryFieldGroup code is courtesy of                            *
 * Mark Summerfield in Qt Quarterly                                       *
 * http://doc.trolltech.com/qq/qq11-mandatoryfields.html                  *
 *                                                                        *
 * Enhanced by Thomas Baumgart to support the lineedit field of a         *
 * a KComboBox.                                                           *
 *                                                                        *
 * With further widgets added by Allan Anderson for missing fields.       *
 **************************************************************************/

class KMandatoryFieldGroupPrivate
{
  Q_DISABLE_COPY(KMandatoryFieldGroupPrivate)

public:
  KMandatoryFieldGroupPrivate() :
    m_okButton(0),
    m_enabled(true)
  {
  }

  QList<QWidget *>      m_widgets;
  QPushButton*          m_okButton;
  bool                  m_enabled;
};

KMandatoryFieldGroup::KMandatoryFieldGroup(QObject *parent) :
    QObject(parent),
    d_ptr(new KMandatoryFieldGroupPrivate)
{
}

KMandatoryFieldGroup::~KMandatoryFieldGroup()
{
  Q_D(KMandatoryFieldGroup);
  delete d;
}

void KMandatoryFieldGroup::add(QWidget *widget)
{
  Q_D(KMandatoryFieldGroup);
  if (!d->m_widgets.contains(widget)) {
    if (qobject_cast<QCheckBox*>(widget))
      connect(qobject_cast<QCheckBox*>(widget),
              &QCheckBox::clicked,
              this, &KMandatoryFieldGroup::changed);

    else if (qobject_cast<KComboBox*>(widget)) {
      KComboBox* combo = qobject_cast<KComboBox*>(widget);
      KLineEdit* lineedit = qobject_cast<KLineEdit*>(combo->lineEdit());
      if (lineedit) {
        connect(lineedit, &QLineEdit::textChanged, this, &KMandatoryFieldGroup::changed);
      } else {
        connect(combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::highlighted), this, &KMandatoryFieldGroup::changed);
      }
    }

    else if (qobject_cast<KMyMoneyEdit*>(widget)) {
      KMyMoneyEdit* amount = qobject_cast<KMyMoneyEdit*>(widget);
      KLineEdit* lineedit = qobject_cast<KLineEdit*>(amount->lineedit());
      if (lineedit) {
        connect(lineedit, &QLineEdit::textChanged, this, &KMandatoryFieldGroup::changed);
      } else {
        connect(amount, SIGNAL(highlighted(int)), this, SLOT(changed()));
      }
    }

    else if (qobject_cast<QLineEdit*>(widget)) {
      connect(qobject_cast<QLineEdit*>(widget),
              &QLineEdit::textChanged,
              this, &KMandatoryFieldGroup::changed);
    }

    else if (qobject_cast<QSpinBox*>(widget))
      connect(qobject_cast<QSpinBox*>(widget),
              static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
              this, &KMandatoryFieldGroup::changed);

    else if (qobject_cast<QListWidget*>(widget))
      connect(qobject_cast<QListWidget*>(widget),
              &QListWidget::itemSelectionChanged,
              this, &KMandatoryFieldGroup::changed);

    else if (qobject_cast<KUrlRequester*>(widget))
      connect(qobject_cast<KUrlRequester*>(widget),
              &KUrlRequester::textChanged,
              this, &KMandatoryFieldGroup::changed);

    else if (qobject_cast<KMyMoneyTextEdit*>(widget))
      connect(qobject_cast<KMyMoneyTextEdit*>(widget),
              &KMyMoneyTextEdit::textChanged,
              this, &KMandatoryFieldGroup::changed);

    else if (qobject_cast<IonlineJobEdit*>(widget)) {
      connect(qobject_cast<IonlineJobEdit*>(widget),
              SIGNAL(validityChanged(bool)),
              this, SLOT(changed()));

      // Do not set palette for IonlineJobEdits as they contain subwidgets
      d->m_widgets.append(widget);
      changed();
      return;
    }

    else {
      qWarning("MandatoryFieldGroup: unsupported class %s", (widget->metaObject()->className()));
      return;
    }

    QPalette palette = widget->palette();
    palette.setColor(QPalette::Base, KMyMoneySettings::schemeColor(SchemeColor::FieldRequired));
    widget->setPalette(palette);
    d->m_widgets.append(widget);
    changed();
  }
}

void KMandatoryFieldGroup::removeAll()
{
  Q_D(KMandatoryFieldGroup);
  while(!d->m_widgets.isEmpty()) {
    remove(d->m_widgets.at(0));
  }
}

void KMandatoryFieldGroup::remove(QWidget *widget)
{
  Q_D(KMandatoryFieldGroup);
  widget->setPalette(QApplication::palette());
  d->m_widgets.removeOne(widget);
  changed();
}

void KMandatoryFieldGroup::setOkButton(QPushButton *button)
{
  Q_D(KMandatoryFieldGroup);
  if (d->m_okButton && d->m_okButton != button)
    d->m_okButton->setEnabled(true);
  d->m_okButton = button;
  changed();
}

void KMandatoryFieldGroup::changed()
{
  Q_D(KMandatoryFieldGroup);
  bool enable = true;
  QList<QWidget *>::ConstIterator i;
  for (i = d->m_widgets.constBegin(); i != d->m_widgets.constEnd(); ++i) {
    QWidget *widget = *i;
    // disabled widgets don't count
    if (!(widget->isEnabled())) {
      continue;
    }
    if (qobject_cast<KMyMoneyPayeeCombo*>(widget)) {
      if ((dynamic_cast<KMyMoneyPayeeCombo*>(widget))->lineEdit()->text().isEmpty()) {
        enable = false;
        break;
      } else
        continue;
    }
    if (qobject_cast<QCheckBox*>(widget)) {
      if ((qobject_cast<QCheckBox*>(widget))->checkState() == Qt::PartiallyChecked) {
        enable = false;
        break;
      } else
        continue;
    }
    if (qobject_cast<KComboBox*>(widget)) {
      if ((qobject_cast<KComboBox*>(widget))->currentText().isEmpty()) {
        enable = false;
        break;
      } else
        continue;
    }
    if (qobject_cast<QLineEdit*>(widget)) {
      if ((qobject_cast<QLineEdit*>(widget))->text().isEmpty()) {
        enable = false;
        break;
      } else
        continue;
    }
    if ((qobject_cast<QListWidget*>(widget))) {
      if ((qobject_cast<QListWidget*>(widget))->selectedItems().count() == 0) {
        enable = false;
        break;
      } else
        continue;
    }
    if ((qobject_cast<KUrlRequester*>(widget))) {
      if ((qobject_cast<KUrlRequester*>(widget))->text().isEmpty()) {
        enable = false;
        break;
      } else
        continue;
    }
    if ((qobject_cast<KMyMoneyEdit*>(widget))) {
      if ((qobject_cast<KMyMoneyEdit*>(widget))->text() == "0/1") {
        enable = false;
        break;
      } else
        continue;
    }
    if (qobject_cast<KMyMoneyTextEdit*>(widget)) {
      if (!(qobject_cast<KMyMoneyTextEdit*>(widget))->isValid()) {
        enable = false;
        break;
      } else {
        continue;
      }
    }
    if (qobject_cast<IonlineJobEdit*>(widget)) {
      if (!(qobject_cast<IonlineJobEdit*>(widget))->isValid()) {
        enable = false;
        break;
      } else {
        continue;
      }
    }
  }

  if (d->m_okButton)
    d->m_okButton->setEnabled(enable);
  d->m_enabled = enable;

  emit stateChanged();
  emit stateChanged(enable);
}

bool KMandatoryFieldGroup::isEnabled() const
{
  Q_D(const KMandatoryFieldGroup);
  return d->m_enabled;
}

void KMandatoryFieldGroup::clear()
{
  Q_D(KMandatoryFieldGroup);
  QList<QWidget *>::Iterator i;
  for (i = d->m_widgets.begin(); i != d->m_widgets.end(); ++i)
    (*i)->setPalette(QApplication::palette());
  d->m_widgets.clear();
  if (d->m_okButton) {
    d->m_okButton->setEnabled(true);
    d->m_okButton = 0;
    d->m_enabled = true;
  }
}
