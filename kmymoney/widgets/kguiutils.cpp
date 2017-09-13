/***************************************************************************
                         kguiutils.cpp  -  description
                            -------------------
   begin                : Fri Jan 27 2006
   copyright            : (C) 2006 Tony Bloomfield
   email                : Tony Bloomfield <tonybloom@users.sourceforge.net>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcombobox.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kmymoneyedit.h>
#include "kmymoneymvccombo.h"
// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"
#include "onlinetasks/interfaces/ui/ionlinejobedit.h"
#include "kmymoneytextedit.h"

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

void kMandatoryFieldGroup::add(QWidget *widget)
{
  if (!m_widgets.contains(widget)) {
    if (qobject_cast<QCheckBox*>(widget))
      connect(qobject_cast<QCheckBox*>(widget),
              SIGNAL(clicked()),
              this, SLOT(changed()));

    else if (qobject_cast<KComboBox*>(widget)) {
      KComboBox* combo = qobject_cast<KComboBox*>(widget);
      KLineEdit* lineedit = qobject_cast<KLineEdit*>(combo->lineEdit());
      if (lineedit) {
        connect(lineedit, SIGNAL(textChanged(QString)), this, SLOT(changed()));
      } else {
        connect(combo, SIGNAL(highlighted(int)), this, SLOT(changed()));
      }
    }

    else if (qobject_cast<kMyMoneyEdit*>(widget)) {
      kMyMoneyEdit* amount = qobject_cast<kMyMoneyEdit*>(widget);
      KLineEdit* lineedit = qobject_cast<KLineEdit*>(amount->lineedit());
      if (lineedit) {
        connect(lineedit, SIGNAL(textChanged(QString)), this, SLOT(changed()));
      } else {
        connect(amount, SIGNAL(highlighted(int)), this, SLOT(changed()));
      }
    }

    else if (qobject_cast<QLineEdit*>(widget)) {
      connect(qobject_cast<QLineEdit*>(widget),
              SIGNAL(textChanged(QString)),
              this, SLOT(changed()));
    }

    else if (qobject_cast<QSpinBox*>(widget))
      connect(qobject_cast<QSpinBox*>(widget),
              SIGNAL(valueChanged(QString)),
              this, SLOT(changed()));

    else if (qobject_cast<QListWidget*>(widget))
      connect(qobject_cast<QListWidget*>(widget),
              SIGNAL(itemSelectionChanged()),
              this, SLOT(changed()));

    else if (qobject_cast<KUrlRequester*>(widget))
      connect(qobject_cast<KUrlRequester*>(widget),
              SIGNAL(textChanged(QString)),
              this, SLOT(changed()));

    else if (qobject_cast<KMyMoneyTextEdit*>(widget))
      connect(qobject_cast<KMyMoneyTextEdit*>(widget),
              SIGNAL(textChanged()),
              this, SLOT(changed()));

    else if (qobject_cast<IonlineJobEdit*>(widget)) {
      connect(qobject_cast<IonlineJobEdit*>(widget),
              SIGNAL(validityChanged(bool)),
              this, SLOT(changed()));

      // Do not set palette for IonlineJobEdits as they contain subwidgets
      m_widgets.append(widget);
      changed();
      return;
    }

    else {
      qWarning("MandatoryFieldGroup: unsupported class %s", (widget->metaObject()->className()));
      return;
    }

    QPalette palette = widget->palette();
    palette.setColor(QPalette::Base, KMyMoneyGlobalSettings::requiredFieldColor());
    widget->setPalette(palette);
    m_widgets.append(widget);
    changed();
  }
}

void kMandatoryFieldGroup::removeAll()
{
  while(!m_widgets.isEmpty()) {
    remove(m_widgets.at(0));
  }
}

void kMandatoryFieldGroup::remove(QWidget *widget)
{
  widget->setPalette(QApplication::palette());
  m_widgets.removeOne(widget);
  changed();
}

void kMandatoryFieldGroup::setOkButton(QPushButton *button)
{
  if (m_okButton && m_okButton != button)
    m_okButton->setEnabled(true);
  m_okButton = button;
  changed();
}

void kMandatoryFieldGroup::changed()
{
  bool enable = true;
  QList<QWidget *>::ConstIterator i;
  for (i = m_widgets.constBegin(); i != m_widgets.constEnd(); ++i) {
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
    if ((qobject_cast<kMyMoneyEdit*>(widget))) {
      if ((qobject_cast<kMyMoneyEdit*>(widget))->text() == "0/1") {
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

  if (m_okButton)
    m_okButton->setEnabled(enable);
  m_enabled = enable;

  emit stateChanged();
  emit stateChanged(enable);
}

void kMandatoryFieldGroup::clear()
{
  QList<QWidget *>::Iterator i;
  for (i = m_widgets.begin(); i != m_widgets.end(); ++i)
    (*i)->setPalette(QApplication::palette());
  m_widgets.clear();
  if (m_okButton) {
    m_okButton->setEnabled(true);
    m_okButton = 0;
    m_enabled = true;
  }
}
