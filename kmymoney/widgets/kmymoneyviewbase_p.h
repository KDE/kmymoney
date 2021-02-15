/*
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
#include "selectedobjects.h"

class KMyMoneyViewBase;
class KMyMoneyViewBasePrivate
{
public:

  explicit KMyMoneyViewBasePrivate(KMyMoneyViewBase* parent)
  : q_ptr(parent)
  , m_needsRefresh(false)
  , m_havePendingChanges(false)
  {}

  virtual ~KMyMoneyViewBasePrivate() {}

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
   * @param scheme reference to a QColor object containing color scheme
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

  KMyMoneyViewBase*     q_ptr;
  SelectedObjects       m_selections;
  bool                  m_needsRefresh;
  bool                  m_havePendingChanges;
};

#endif
