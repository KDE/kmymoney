/*
 * Copyright 2005       Ace Jones <acejones@users.sourceforge.net>
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

#ifndef KMYMONEYTITLELABEL_H
#define KMYMONEYTITLELABEL_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QColor>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author ace jones
  */
class KMyMoneyTitleLabelPrivate;
class KMM_BASE_WIDGETS_EXPORT KMyMoneyTitleLabel : public QLabel
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyTitleLabel)
  Q_PROPERTY(QString leftImageFile READ leftImageFile WRITE setLeftImageFile DESIGNABLE true)
  Q_PROPERTY(QString rightImageFile READ rightImageFile WRITE setRightImageFile DESIGNABLE true)
  Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor DESIGNABLE true)
  Q_PROPERTY(QString text READ text WRITE setText DESIGNABLE true)

public:
  explicit KMyMoneyTitleLabel(QWidget* parent = nullptr);
  ~KMyMoneyTitleLabel();

  void setBgColor(const QColor& _color);
  void setLeftImageFile(const QString& _file);
  void setRightImageFile(const QString& _file);

  QString leftImageFile() const;
  QString rightImageFile() const;
  QColor bgColor() const;
  QString text() const;

public Q_SLOTS:
  virtual void setText(const QString& txt);

protected:
  void updatePixmap();
  void paintEvent(QPaintEvent *) override;

private:
  KMyMoneyTitleLabelPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyTitleLabel)
};

#endif
