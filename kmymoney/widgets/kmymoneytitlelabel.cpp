/***************************************************************************
                          kmymoneytitlelabel.cpp
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

#include "kmymoneytitlelabel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QStyle>
#include <QPainter>
#include <QLabel>
#include <QStandardPaths>
#include <QFontDatabase>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

KMyMoneyTitleLabel::KMyMoneyTitleLabel(QWidget *parent) :
    QLabel(parent)
{
  m_bgColor = KColorScheme(isEnabled() ? QPalette::Active : QPalette::Inactive, KColorScheme::Selection).background(KColorScheme::NormalBackground).color();

  setFont(QFontDatabase::systemFont(QFontDatabase::TitleFont));
}

KMyMoneyTitleLabel::~KMyMoneyTitleLabel()
{
}

void KMyMoneyTitleLabel::setLeftImageFile(const QString& _file)
{
  m_leftImageFile = _file;
  QString lfullpath = QStandardPaths::locate(QStandardPaths::DataLocation, m_leftImageFile);
  m_leftImage.load(lfullpath);
}

void KMyMoneyTitleLabel::setRightImageFile(const QString& _file)
{
  m_rightImageFile = _file;
  QString rfullpath = QStandardPaths::locate(QStandardPaths::DataLocation, m_rightImageFile);
  m_rightImage.load(rfullpath);
  if (m_rightImage.height() < 30)
    setMinimumHeight(30);
  else {
    setMinimumHeight(m_rightImage.height());
    setMaximumHeight(m_rightImage.height());
  }
}

void KMyMoneyTitleLabel::paintEvent(QPaintEvent *e)
{
  QLabel::paintEvent(e);

  QPainter painter(this);
  QRect cr = contentsRect();

  // prepare the pixmap
  QImage output(cr.width(), cr.height(), QImage::Format_RGB32);
  output.fill(m_bgColor.rgb());

  QPixmap result = QPixmap::fromImage(output);
  QPixmap overlay = QPixmap::fromImage(m_rightImage);
  QPainter pixmapPainter(&result);
  pixmapPainter.drawPixmap(cr.width() - m_rightImage.width(), 0, overlay, 0, 0, overlay.width(), overlay.height());
  overlay = QPixmap::fromImage(m_leftImage);
  pixmapPainter.drawPixmap(0, 0, overlay, 0, 0, overlay.width(), overlay.height());

  // first draw pixmap
  style()->drawItemPixmap(&painter, contentsRect(), alignment(), result);

  // then draw text on top with a larger font (relative to the pixmap size) and with the appropriate color
  QFont font = painter.font();
  font.setPointSizeF(qMax(result.height() / static_cast<qreal>(2.5), font.pointSizeF()));
  painter.setFont(font);
  painter.setPen(KColorScheme(QPalette::Active, KColorScheme::Selection).foreground(KColorScheme::NormalText).color());
  style()->drawItemText(&painter, contentsRect(), alignment(), palette(), isEnabled(), QString("   ") + m_text);
}

void KMyMoneyTitleLabel::setText(const QString& txt)
{
  m_text = txt;
  m_text.replace('\n', QLatin1String(" "));
  update();
}
