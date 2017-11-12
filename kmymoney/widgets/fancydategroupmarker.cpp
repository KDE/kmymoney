/***************************************************************************
                             fancydategroupmarker.cpp  -  description
                             -------------------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <config-kmymoney.h>

#include "fancydategroupmarker.h"
#include "fancydategroupmarker_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QToolTip>
#include <QPixmap>
#include <QMouseEvent>
#include <QList>
#include <QKeyEvent>
#include <QEvent>
#include <QFrame>
#include <QHeaderView>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QPushButton>
#include <QPainter>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes
#include "stdtransactiondownloaded.h"
#include "stdtransactionmatched.h"
#include "scheduledtransaction.h"
#include "kmymoneyglobalsettings.h"
#include "mymoneyfile.h"
#include "groupmarker.h"
#include "mymoneyenums.h"

FancyDateGroupMarker::FancyDateGroupMarker(Register* parent,
                                           const QDate& date,
                                           const QString& txt) :
    GroupMarker(*new FancyDateGroupMarkerPrivate, parent, txt)
{
  Q_D(FancyDateGroupMarker);
  d->m_date = date;
}

FancyDateGroupMarker::FancyDateGroupMarker(FancyDateGroupMarkerPrivate &dd, Register *parent, const QDate& date, const QString& txt) :
  GroupMarker(dd, parent, txt)
{
  Q_D(FancyDateGroupMarker);
  d->m_date = date;
}

FancyDateGroupMarker::~FancyDateGroupMarker()
{
}

QDate FancyDateGroupMarker::sortPostDate() const
{
  Q_D(const FancyDateGroupMarker);
  return d->m_date;
}

QDate FancyDateGroupMarker::sortEntryDate() const
{
  Q_D(const FancyDateGroupMarker);
  return d->m_date;
}

const char* FancyDateGroupMarker::className()
{
  return "FancyDateGroupMarker";
}
