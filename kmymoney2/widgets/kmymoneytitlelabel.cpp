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

KMyMoneyTitleLabel::KMyMoneyTitleLabel(QWidget *parent, const char *name) :
  QLabel(parent, name),
  m_bgColor( KColorScheme::ActiveBackground ),
  m_textColor( KColorScheme::ActiveText )
{
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

void KMyMoneyTitleLabel::resizeEvent ( QResizeEvent * )
{
#warning "port to kde4"	
#if 0	
  QRect cr = contentsRect();
  QImage output( cr.width(), cr.height(), 32 );
  output.fill( m_bgColor.rgb() );

  bitBlt ( &output, cr.width() - m_rightImage.width(), 0, &m_rightImage, 0, 0, m_rightImage.width(), m_rightImage.height(), 0 );
  bitBlt ( &output, 0, 0, &m_leftImage, 0, 0, m_leftImage.width(), m_leftImage.height(), 0 );

  QPixmap pix;
  pix.convertFromImage(output);
  setPixmap(pix);
  setMinimumWidth( m_rightImage.width() );
#endif
}

void KMyMoneyTitleLabel::drawContents(QPainter *p)
{
#warning "port to kde4"	
#if 0	
  // first draw pixmap
  QLabel::drawContents(p);

  // then draw text on top
  style().drawItem( p, contentsRect(), alignment(), colorGroup(), isEnabled(),
                          0, QString("   ")+m_text, -1, &m_textColor );
#endif
}

void KMyMoneyTitleLabel::setText(const QString& txt)
{
  m_text = txt;
  update();
}

#include "kmymoneytitlelabel.moc"
