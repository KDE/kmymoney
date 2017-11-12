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

#include <QKeyEvent>
#include <QFocusEvent>
#include <QTimer>
#include <QLocale>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyLineEdit::Private
{
public:
  /**
    * This member keeps the initial value. It is used during
    * resetText() to set the widgets text back to this initial value
    * and as comparison during focusOutEvent() to emit the lineChanged
    * signal if the current text is different.
    */
  QString m_text;

  /**
    * This member keeps the status if overriding the numeric keypad comma key
    * is requested or not.
    */
  bool m_forceMonetaryDecimalSymbol;
  bool  skipSelectAll;
};

KMyMoneyLineEdit::KMyMoneyLineEdit(QWidget *w, bool forceMonetaryDecimalSymbol, Qt::Alignment alignment) :
    KLineEdit(w),
    d(new Private)
{
  d->m_forceMonetaryDecimalSymbol = forceMonetaryDecimalSymbol;
  setAlignment(alignment);
  skipSelectAll(false);
}

KMyMoneyLineEdit::~KMyMoneyLineEdit()
{
  delete d;
}

void KMyMoneyLineEdit::skipSelectAll(bool skipIt)
{
  d->skipSelectAll = skipIt;
}

void KMyMoneyLineEdit::resetText()
{
  setText(d->m_text);
}

void KMyMoneyLineEdit::loadText(const QString& text)
{
  d->m_text = text;
  setText(text);
}

void KMyMoneyLineEdit::focusOutEvent(QFocusEvent *ev)
{
  // if the current text is not in the list of
  // possible completions, we have a new payee
  // and signal that to the outside world.
  if (text() != d->m_text) {
    emit lineChanged(text());
  }
  KLineEdit::focusOutEvent(ev);

  // force update of hint
  if (text().isEmpty())
    repaint();
}

void KMyMoneyLineEdit::focusInEvent(QFocusEvent *ev)
{
  KLineEdit::focusInEvent(ev);
  // select the text so it can be edited by the user - only if the widget
  // is not focused after a popup is closed (which could be the completer
  // of the KMyMoneyCombo).
  //
  // Delay that selection until the application is idle to prevent a
  // recursive loop which otherwise entered when the focus is set to this
  // widget using the mouse. (bko #259369)
  if (ev->reason() != Qt::PopupFocusReason && ev->reason() != Qt::ActiveWindowFocusReason) {
    if (!d->skipSelectAll)
      QTimer::singleShot(0, this, SLOT(selectAll()));
    d->skipSelectAll = false;
  }
}

void KMyMoneyLineEdit::keyReleaseEvent(QKeyEvent* k)
{
  if (d->m_forceMonetaryDecimalSymbol) {
    if (k->modifiers() & Qt::KeypadModifier) {
      if (k->key() == Qt::Key_Comma
          || k->key() == Qt::Key_Period) {
        if (QLocale().decimalPoint() == QLatin1Char(',')) {
          QKeyEvent newk(k->type(), Qt::Key_Comma, k->modifiers(), ",", k->isAutoRepeat(), k->count());
          KLineEdit::keyReleaseEvent(&newk);
          k->accept();
          return;
        }

        if (QLocale().decimalPoint() == QLatin1Char('.')) {
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

void KMyMoneyLineEdit::keyPressEvent(QKeyEvent* k)
{
  if (d->m_forceMonetaryDecimalSymbol) {
    if (k->modifiers() & Qt::KeypadModifier) {
      if (k->key() == Qt::Key_Comma
          || k->key() == Qt::Key_Period) {
        if (QLocale().decimalPoint() == QLatin1Char(',')) {
          QKeyEvent newk(k->type(), Qt::Key_Comma, k->modifiers(), ",", k->isAutoRepeat(), k->count());
          KLineEdit::keyPressEvent(&newk);
          k->accept();
          return;
        }

        if (QLocale().decimalPoint() == QLatin1Char('.')) {
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
