/*
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2020       Robert Szczesiak <dev.rszczesiak@gmail.com>
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

#ifndef KMYMONEYVIEWBASEPRIVATE_H
#define KMYMONEYVIEWBASEPRIVATE_H

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

class KMyMoneyViewBasePrivate
{
public:
  virtual ~KMyMoneyViewBasePrivate(){}

   /**
   * @brief Sets label text and font
   * @param label pointer to a QLabel object to update
   * @param labelText reference to a QString object to set as label text
   */
  void updateViewLabel(QLabel* label, const QString& labelText)
  {
    label->setFont(KMyMoneySettings::listCellFontEx());
    label->setText(labelText);
  }

  /**
   * @brief Returns formatted rich text
   * @param value reference to a MyMoneyMoney object containing the value to be formatted
   * @param scheme reference to a QColor object containging color scheme
   * @return Qstring representing label value with added formatting
   */
  QString formatViewLabelValue(const MyMoneyMoney& value, const QColor& scheme)
  {
    const QString colorDef = QStringLiteral("#%1%2%3").arg(scheme.red(), 2, 16, QLatin1Char('0')).arg(scheme.green(), 2, 16, QLatin1Char('0')).arg(scheme.blue(), 2, 16, QLatin1Char('0'));

    QString s(MyMoneyUtils::formatMoney(value, MyMoneyFile::instance()->baseCurrency()));
    s.prepend(QStringLiteral("<b><font color=\"%1\">").arg(colorDef));
    s.append(QLatin1String("</font></b>"));

    return s;
  }

  QString formatViewLabelValue(const MyMoneyMoney& value)
  {
    const QColor scheme = KMyMoneySettings::schemeColor(value.isNegative() ? SchemeColor::Negative : SchemeColor::Positive).convertTo(QColor::Rgb);
    return formatViewLabelValue(value, scheme);
  }

  bool m_needsRefresh;
};

#endif
