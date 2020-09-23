/*
 * Copyright 2001       Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2002       Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2006-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KMYMONEYLINEEDIT_H
#define KMYMONEYLINEEDIT_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_base_widgets_export.h"

/**
  * This class represents a special verson of a KLineEdit object that
  * supports the display of a hint if the display area is empty. It also
  * overrides the comma key on the numeric keypad with the currently
  * selected monetaryDecimalSymbol if selected during creation of the object.
  *
  * @author Michael Edwardes
  * @author Thomas Baumgart
  */
class QFocusEvent;
class QKeyEvent;
class KMM_BASE_WIDGETS_EXPORT KMyMoneyLineEdit : public KLineEdit
{
  Q_OBJECT
public:
  /**
    * @param w pointer to parent
    * @param forceMonetaryDecimalSymbol if @a true, the numeric keypad comma key will have a fixed
    *            value and does not follow the keyboard layout (default: @p false)
    * @param alignment Controls the alignment of the text. Default is Qt::AlignLeft | Qt::AlignVCenter.
    *                  See Qt::AlignmentFlags for other possible values.
    */
  explicit KMyMoneyLineEdit(QWidget *w = 0, bool forceMonetaryDecimalSymbol = false, Qt::Alignment alignment = (Qt::AlignLeft | Qt::AlignVCenter));
  ~KMyMoneyLineEdit();

  /**
    * This method is used to set the value of the widget back to
    * the one passed using loadText().
    */
  void resetText();

  /**
    * Do not select the text upon the next focus in event if
    * @p skipIt is set to @p true. When object is constructed,
    * the default is @p false.
    */
  void skipSelectAll(bool skipIt);

public Q_SLOTS:
  void loadText(const QString& text);

Q_SIGNALS:
  /**
    * This signal is emitted when the focus leaves the object and the text
    * has been changed. The new text is passed as @a str.
    */
  void lineChanged(const QString& str);

protected:
  void focusOutEvent(QFocusEvent *ev) override;
  void focusInEvent(QFocusEvent *ev) override;

  /**
    * Overridden so that the period key on the numeric keypad always sends
    * out the currently selected monetary decimal symbol instead of the
    * key defined by the keymap.
    *
    * Example: If you have set the keymap (keyboard layout) as English, then
    * the numeric keypad will send a period but if you have set the keymap to
    * be German, the same key will send a comma.
    *
    * @param ev pointer to current QKeyEvent
    */
  void keyPressEvent(QKeyEvent* ev) override;

  /**
    * Overridden so that the period key on the numeric keypad always sends
    * out the currently selected monetary decimal symbol instead of the
    * key defined by the keymap.
    *
    * Example: If you have set the keymap (keyboard layout) as English, then
    * the numeric keypad will send a period but if you have set the keymap to
    * be German, the same key will send a comma.
    *
    * @param ev pointer to current QKeyEvent
    */
  void keyReleaseEvent(QKeyEvent* ev) override;

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

#endif
