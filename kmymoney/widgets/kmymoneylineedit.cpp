/***************************************************************************
                          kmymoneylineedit.cpp  -  description
                             -------------------
    begin                : Wed May 9 2001
    copyright            : (C) 2001 by Michael Edwardes <mte@users.sourceforge.net>
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneylineedit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRect>
#include <QPainter>
#include <QPalette>
#include <QKeyEvent>
#include <QFocusEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

kMyMoneyLineEdit::kMyMoneyLineEdit(QWidget *w, bool forceMonetaryDecimalSymbol, Qt::Alignment alignment) :
    KLineEdit(w),
    m_forceMonetaryDecimalSymbol(forceMonetaryDecimalSymbol)
{
  setAlignment(alignment);
}

kMyMoneyLineEdit::~kMyMoneyLineEdit()
{
}

void kMyMoneyLineEdit::resetText(void)
{
  setText(m_text);
}

void kMyMoneyLineEdit::loadText(const QString& text)
{
  m_text = text;
  setText(text);
}

void kMyMoneyLineEdit::focusOutEvent(QFocusEvent *ev)
{
  // if the current text is not in the list of
  // possible completions, we have a new payee
  // and signal that to the outside world.
  if (text() != m_text) {
    emit lineChanged(text());
  }
  KLineEdit::focusOutEvent(ev);

  // force update of hint
  if (text().isEmpty())
    repaint();
}

void kMyMoneyLineEdit::focusInEvent(QFocusEvent *ev)
{
  KLineEdit::focusInEvent(ev);
  // select the text so it can be edited by the user - only if the widget is not focused
  // after a popup is closed (which could be the completer of the KMyMoneyCombo)
  if (ev->reason() != Qt::PopupFocusReason)
    selectAll();
}

void kMyMoneyLineEdit::keyReleaseEvent(QKeyEvent* k)
{
  if (m_forceMonetaryDecimalSymbol) {
    if (k->modifiers() & Qt::KeypadModifier) {
      if (k->key() == Qt::Key_Comma
          || k->key() == Qt::Key_Period) {
        if (KGlobal::locale()->monetaryDecimalSymbol() == ",") {
          QKeyEvent newk(k->type(), Qt::Key_Comma, k->modifiers(), ",", k->isAutoRepeat(), k->count());
          KLineEdit::keyReleaseEvent(&newk);
          k->accept();
          return;
        }

        if (KGlobal::locale()->monetaryDecimalSymbol() == ".") {
          QKeyEvent newk(k->type(), Qt::Key_Comma, k->modifiers(), ".", k->isAutoRepeat(), k->count());
          KLineEdit::keyReleaseEvent(&newk);
          k->accept();
          return;
        }
      }
    }
  }
  KLineEdit::keyReleaseEvent(k);
}

void kMyMoneyLineEdit::keyPressEvent(QKeyEvent* k)
{
  if (m_forceMonetaryDecimalSymbol) {
    if (k->modifiers() & Qt::KeypadModifier) {
      if (k->key() == Qt::Key_Comma
          || k->key() == Qt::Key_Period) {
        if (KGlobal::locale()->monetaryDecimalSymbol() == ",") {
          QKeyEvent newk(k->type(), Qt::Key_Comma, k->modifiers(), ",", k->isAutoRepeat(), k->count());
          KLineEdit::keyPressEvent(&newk);
          k->accept();
          return;
        }

        if (KGlobal::locale()->monetaryDecimalSymbol() == ".") {
          QKeyEvent newk(k->type(), Qt::Key_Period, k->modifiers(), ".", k->isAutoRepeat(), k->count());
          KLineEdit::keyPressEvent(&newk);
          k->accept();
          return;
        }
      }
    }
  }
  KLineEdit::keyPressEvent(k);
}

#include "kmymoneylineedit.moc"
