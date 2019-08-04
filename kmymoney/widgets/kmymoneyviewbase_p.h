/*
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneymoney.h>
#include <mymoneyfile.h>
#include <accountsmodel.h>
#include <kmymoneysettings.h>
#include <mymoneysecurity.h>
#include <mymoneyutils.h>

#ifndef KMYMONEYVIEWBASEPRIVATE_H
#define KMYMONEYVIEWBASEPRIVATE_H

class KMyMoneyViewBasePrivate
{
public:
  virtual ~KMyMoneyViewBasePrivate(){}

  void updateNetWorthLabel(const MyMoneyMoney& value, bool isApproximate, QLabel* label, const QString& labelText)
  {
    const QColor scheme = KMyMoneySettings::schemeColor(value.isNegative() ? SchemeColor::Negative : SchemeColor::Positive).convertTo(QColor::Rgb);
    const QString colorDef = QStringLiteral("#%1%2%3").arg(scheme.red(), 2, 16, QLatin1Char('0')).arg(scheme.green(), 2, 16, QLatin1Char('0')).arg(scheme.blue(), 2, 16, QLatin1Char('0'));

    QString s(MyMoneyUtils::formatMoney(value, MyMoneyFile::instance()->baseCurrency()));
    if (isApproximate)
      s.prepend(QStringLiteral("~"));

    s.replace(QLatin1Char(' '), QStringLiteral("&nbsp;"));
    s.prepend(QStringLiteral("<b><font color=\"%1\">").arg(colorDef));
    s.append(QLatin1String("</font></b>"));
    QString txt(labelText);
    txt.replace(QLatin1Char(' '), QStringLiteral("&nbsp;"));

    label->setFont(KMyMoneySettings::listCellFontEx());
    label->setText(txt.arg(s));
  }

  bool m_needsRefresh;
};

#endif
