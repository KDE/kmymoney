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

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QVariant>
#include <QStyle>
#include <QPainter>
//Added by qt3to4:
#include <QResizeEvent>
#include <QLabel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneytitlelabel.h"

KMyMoneyTitleLabel::KMyMoneyTitleLabel(QWidget *parent) :
  QLabel(parent)
{
  m_bgColor = KColorScheme(isEnabled() ? QPalette::Active : QPalette::Inactive, KColorScheme::Selection).background(KColorScheme::NormalBackground).color();

  setFont(KGlobalSettings::windowTitleFont());
}

KMyMoneyTitleLabel::~KMyMoneyTitleLabel()
{
}

void KMyMoneyTitleLabel::setLeftImageFile(const QString& _file)
{
  m_leftImageFile = _file;
  QString lfullpath = KGlobal::dirs()->findResource("appdata", m_leftImageFile);
  m_leftImage.load(lfullpath);
  m_leftImage.setAlphaBuffer(true);
}

void KMyMoneyTitleLabel::setRightImageFile(const QString& _file)
{
  m_rightImageFile = _file;
  QString rfullpath = KGlobal::dirs()->findResource("appdata", m_rightImageFile);
  m_rightImage.load(rfullpath);
  m_rightImage.setAlphaBuffer(true);
  if(m_rightImage.height() < 30)
    setMinimumHeight(30);
  else {
    setMinimumHeight( m_rightImage.height() );
    setMaximumHeight( m_rightImage.height() );
  }
}

void KMyMoneyTitleLabel::paintEvent(QPaintEvent *e)
{
  QLabel::paintEvent(e);

  QPainter painter(this);
  QRect cr = contentsRect();

  // prepare the pixmap
  QImage output( cr.width(), cr.height(), QImage::Format_RGB32 );
  output.fill( m_bgColor.rgb() );

  QPixmap result = QPixmap::fromImage(output);
  QPixmap overlay = QPixmap::fromImage(m_rightImage);
  QPainter pixmapPainter(&result);
  pixmapPainter.drawPixmap(cr.width() - m_rightImage.width(), 0, overlay, 0, 0, overlay.width(), overlay.height());
  overlay = QPixmap::fromImage(m_leftImage);
  pixmapPainter.drawPixmap(0, 0, overlay, 0, 0, overlay.width(), overlay.height());

  // first draw pixmap
  style()->drawItemPixmap(&painter, contentsRect(), alignment(), result);
  // then draw text on top
  style()->drawItemText(&painter, contentsRect(), alignment(), colorGroup(), isEnabled(), QString("   ")+m_text);
}

void KMyMoneyTitleLabel::setText(const QString& txt)
{
  m_text = txt;
  update();
}

#include "kmymoneytitlelabel.moc"
