/***************************************************************************
                          kmymoneytitlelabel.h
                             -------------------
    begin                : Sun Feb 05 2005
    copyright            : (C) 2005 by Ace Jones
    email                : acejones@users.sourceforge.net
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
#include <QImage>
#include <QColor>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author ace jones
  */
class KMyMoneyTitleLabel : public QLabel
{
  Q_OBJECT
  Q_PROPERTY(QString leftImageFile READ leftImageFile WRITE setLeftImageFile DESIGNABLE true)
  Q_PROPERTY(QString rightImageFile READ rightImageFile WRITE setRightImageFile DESIGNABLE true)
  Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor DESIGNABLE true)
  Q_PROPERTY(QString text READ text WRITE setText DESIGNABLE true)

public:
  KMyMoneyTitleLabel(QWidget *parent = 0);
  ~KMyMoneyTitleLabel();

  void setBgColor(const QColor& _color) {
    m_bgColor = _color;
  }
  void setLeftImageFile(const QString& _file);
  void setRightImageFile(const QString& _file);

  const QString& leftImageFile() const {
    return m_leftImageFile;
  }
  const QString& rightImageFile() const {
    return m_rightImageFile;
  }
  QColor bgColor() const {
    return m_bgColor;
  }
  QString text() const {
    return m_text;
  }

public slots:
  virtual void setText(const QString& txt);

protected:
  void updatePixmap();
  void paintEvent(QPaintEvent *);

private:
  QImage m_leftImage;
  QImage m_rightImage;
  QColor m_bgColor;
  QString m_text;

  QString m_leftImageFile;
  QString m_rightImageFile;
};

#endif
