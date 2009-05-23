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

// ----------------------------------------------------------------------------
// QT Includes
 // No need for QDateEdit, QSpinBox, etc., since these always return values

#include <qcheckbox.h>
#include <q3listbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qwidget.h>
#include <q3hbox.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kguiutils.h"
#include "kmymoneyglobalsettings.h"

 /**************************************************************************
  *                                                                        *
  * The MandatoryFieldGroup code is courtesy of                            *
  * Mark Summerfield in Qt Quarterly                                       *
  * http://doc.trolltech.com/qq/qq11-mandatoryfields.html                  *
  *                                                                        *
  * Enhanced by Thomas Baumgart to support the lineedit field of a         *
  * a QComboBox.                                                           *
  *                                                                        *
  **************************************************************************/

void kMandatoryFieldGroup::add(QWidget *widget)
{
  if (!widgets.contains(widget)) {
    if (widget->inherits("QCheckBox"))
      connect(qobject_cast<QCheckBox*>(widget),
               SIGNAL(clicked()),
               this, SLOT(changed()));

    else if (widget->inherits("QComboBox")) {
      QComboBox* combo = qobject_cast<QComboBox*>(widget);
      QLineEdit* lineedit = combo->lineEdit();
      if(lineedit) {
        connect(lineedit, SIGNAL(textChanged(const QString&)), this, SLOT(changed()));
      } else {
        connect(combo, SIGNAL(highlighted(int)), this, SLOT(changed()));
      }
    }

    else if (widget->inherits("QLineEdit"))
      connect(qobject_cast<QLineEdit*>(widget),
               SIGNAL(textChanged(const QString&)),
               this, SLOT(changed()));

    else if (widget->inherits("QSpinBox"))
      connect(qobject_cast<QSpinBox*>(widget),
               SIGNAL(valueChanged(const QString&)),
                      this, SLOT(changed()));

    else if (widget->inherits("QListBox"))
      connect(qobject_cast<Q3ListBox*>(widget),
               SIGNAL(selectionChanged()),
                      this, SLOT(changed()));

    else {
      qWarning("MandatoryFieldGroup: unsupported class %s",
               widget->className());
      return;
    }

    widget->setPaletteBackgroundColor(KMyMoneyGlobalSettings::requiredFieldColor());
    widgets.append(widget);
    changed();
  }
}


void kMandatoryFieldGroup::remove(QWidget *widget)
{
  widget->setPaletteBackgroundColor(widget->colorGroup().background());
  widgets.remove(widget);
  changed();
}


void kMandatoryFieldGroup::setOkButton(QPushButton *button)
{
  if (okButton && okButton != button)
    okButton->setEnabled(true);
  okButton = button;
  changed();
}


void kMandatoryFieldGroup::changed(void)
{
  bool enable = true;
  Q3ValueList<QWidget *>::ConstIterator i;
  for (i = widgets.begin(); i != widgets.end(); ++i) {
    QWidget *widget = *i;
    // disabled widgets don't count
    if(!(widget->isEnabled())) {
      continue;
    }
    if (widget->inherits("QCheckBox")) {
      if ((qobject_cast<QCheckBox*>(widget))->state() == QCheckBox::NoChange) {
        enable = false;
        break;
      } else
        continue;
    }
    if (widget->inherits("QComboBox")) {
      if ((qobject_cast<QComboBox*>(widget))->currentText().isEmpty()) {
        enable = false;
        break;
      } else
        continue;
    }
    if (widget->inherits("QLineEdit")) {
      if ((qobject_cast<QLineEdit*>(widget))->text().isEmpty()) {
        enable = false;
        break;
      } else
        continue;
    }
    if (widget->inherits("QListBox")) {
      if ((qobject_cast<Q3ListBox*>(widget))->selectedItem() == 0) {
        enable = false;
        break;
      } else
        continue;
    }
  }

  if (okButton)
    okButton->setEnabled(enable);
  m_enabled = enable;

  emit stateChanged();
  emit stateChanged(enable);
}


void kMandatoryFieldGroup::clear(void)
{
  Q3ValueList<QWidget *>::Iterator i;
  for (i = widgets.begin(); i != widgets.end(); ++i)
    (*i)->setPaletteBackgroundColor((*i)->colorGroup().background());
  widgets.clear();
  if (okButton) {
    okButton->setEnabled(true);
    okButton = 0;
    m_enabled = true;
  }
}


#include "kguiutils.moc"

