/*
 * Copyright 2013-2014  Christian Dávid <christian-david@web.de>
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

#ifndef KMYMONEYTEXTEDITHIGHLIGHTER_H
#define KMYMONEYTEXTEDITHIGHLIGHTER_H

#include <KTextEdit>
#include "kmm_base_widgets_export.h"

/**
 * @brief KTextEdit with restricted character set and length
 *
 * Used to set constraints on input. It allows to set readOnly property by
 * slots as well (not possible with KTextEdit).
 */
class KMyMoneyTextEditPrivate;
class KMM_BASE_WIDGETS_EXPORT KMyMoneyTextEdit : public KTextEdit
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyTextEdit)

  /**
   * @brief Maximal number of characters allowed
   */
  Q_PROPERTY(int maxLength READ maxLength WRITE setMaxLength)

  /**
   * @brief Maximal number of characters allowed per line
   */
  Q_PROPERTY(int maxLineLength READ maxLineLength WRITE setMaxLineLength)

  /**
   * @brief Maximal number of lines
   */
  Q_PROPERTY(int maxLines READ maxLines WRITE setMaxLines)

  /**
   * @brief List of all allowed chars
   */
  Q_PROPERTY(QString allowedChars READ allowedChars WRITE setAllowedChars)

  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)

public:
  explicit KMyMoneyTextEdit(QWidget* parent = nullptr);
  ~KMyMoneyTextEdit();

  int maxLength() const;
  int maxLineLength() const;
  int maxLines() const;
  QString allowedChars() const;
  bool isValid() const;

public Q_SLOTS:
  void setMaxLength(const int& maxLength);
  void setMaxLineLength(const int& maxLineLength);
  void setMaxLines(const int& maxLines);
  void setAllowedChars(const QString& allowedChars);

  /** @brief Slot to set this text edit read only */
  void setReadOnly(bool) override;

protected:
  virtual void keyReleaseEvent(QKeyEvent* e) override;
  virtual void keyPressEvent(QKeyEvent* e) override;

private:
  KMyMoneyTextEditPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyTextEdit)
};

#endif // KMYMONEYTEXTEDITHIGHLIGHTER_H
