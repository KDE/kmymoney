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

#include "kmymoneytitlelabel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPixmap>
#include <QStyle>
#include <QPainter>
#include <QLabel>
#include <QStandardPaths>
#include <QFontDatabase>
#include <QImage>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyTitleLabelPrivate
{
  Q_DISABLE_COPY(KMyMoneyTitleLabelPrivate)

public:
  KMyMoneyTitleLabelPrivate()
  {
  }

  QImage m_leftImage;
  QImage m_rightImage;
  QColor m_bgColor;
  QString m_text;

  QString m_leftImageFile;
  QString m_rightImageFile;
};

KMyMoneyTitleLabel::KMyMoneyTitleLabel(QWidget *parent) :
    QLabel(parent),
    d_ptr(new KMyMoneyTitleLabelPrivate)
{
  Q_D(KMyMoneyTitleLabel);
  d->m_bgColor = KColorScheme(isEnabled() ? QPalette::Active : QPalette::Inactive, KColorScheme::Selection).background(KColorScheme::NormalBackground).color();

  setFont(QFontDatabase::systemFont(QFontDatabase::TitleFont));
}

KMyMoneyTitleLabel::~KMyMoneyTitleLabel()
{
  Q_D(KMyMoneyTitleLabel);
  delete d;
}

void KMyMoneyTitleLabel::setBgColor(const QColor& _color)
{
  Q_D(KMyMoneyTitleLabel);
  d->m_bgColor = _color;
}

void KMyMoneyTitleLabel::setLeftImageFile(const QString& _file)
{
  Q_D(KMyMoneyTitleLabel);
  d->m_leftImageFile = _file;
  QString lfullpath = QStandardPaths::locate(QStandardPaths::DataLocation, d->m_leftImageFile);
  d->m_leftImage.load(lfullpath);
}

void KMyMoneyTitleLabel::setRightImageFile(const QString& _file)
{
  Q_D(KMyMoneyTitleLabel);
  d->m_rightImageFile = _file;
  QString rfullpath = QStandardPaths::locate(QStandardPaths::DataLocation, d->m_rightImageFile);
  d->m_rightImage.load(rfullpath);
  if (d->m_rightImage.height() < 30)
    setMinimumHeight(30);
  else {
    setMinimumHeight(d->m_rightImage.height());
    setMaximumHeight(d->m_rightImage.height());
  }
}

QString KMyMoneyTitleLabel::leftImageFile() const
{
  Q_D(const KMyMoneyTitleLabel);
  return d->m_leftImageFile;
}

QString KMyMoneyTitleLabel::rightImageFile() const
{
  Q_D(const KMyMoneyTitleLabel);
  return d->m_rightImageFile;
}

QColor KMyMoneyTitleLabel::bgColor() const
{
  Q_D(const KMyMoneyTitleLabel);
  return d->m_bgColor;
}

QString KMyMoneyTitleLabel::text() const
{
  Q_D(const KMyMoneyTitleLabel);
  return d->m_text;
}

void KMyMoneyTitleLabel::paintEvent(QPaintEvent *e)
{
  Q_D(KMyMoneyTitleLabel);
  QLabel::paintEvent(e);

  QPainter painter(this);
  QRect cr = contentsRect();

  // prepare the pixmap
  QImage output(cr.width(), cr.height(), QImage::Format_RGB32);
  output.fill(d->m_bgColor.rgb());

  QPixmap result = QPixmap::fromImage(output);
  QPixmap overlay = QPixmap::fromImage(d->m_rightImage);
  QPainter pixmapPainter(&result);
  pixmapPainter.drawPixmap(cr.width() - d->m_rightImage.width(), 0, overlay, 0, 0, overlay.width(), overlay.height());
  overlay = QPixmap::fromImage(d->m_leftImage);
  pixmapPainter.drawPixmap(0, 0, overlay, 0, 0, overlay.width(), overlay.height());

  // first draw pixmap
  style()->drawItemPixmap(&painter, contentsRect(), alignment(), result);

  // then draw text on top with a larger font (relative to the pixmap size) and with the appropriate color
  QFont font = painter.font();
  font.setPointSizeF(qMax(result.height() / static_cast<qreal>(2.5), font.pointSizeF()));
  painter.setFont(font);
  painter.setPen(KColorScheme(QPalette::Active, KColorScheme::Selection).foreground(KColorScheme::NormalText).color());
  style()->drawItemText(&painter, contentsRect(), alignment(), palette(), isEnabled(), QString("   ") + d->m_text);
}

void KMyMoneyTitleLabel::setText(const QString& txt)
{
  Q_D(KMyMoneyTitleLabel);
  d->m_text = txt;
  d->m_text.replace('\n', QLatin1String(" "));
  update();
}
