/***************************************************************************
                          kmymoneytitlelabel.h
                             -------------------
    begin                : Sun Feb 05 2005
    copyright            : (C) 2005 by Ace Jones
    email                : acejones@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYTITLELABEL_H
#define KMYMONEYTITLELABEL_H

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
class KMyMoneyTitleLabel : public QLabel
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

public slots:
  virtual void setText(const QString& txt);

protected:
  void updatePixmap();
  void paintEvent(QPaintEvent *) override;

private:
  KMyMoneyTitleLabelPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMyMoneyTitleLabel)
};

#endif
