/*
 * SPDX-FileCopyrightText: 2009-2016 Cristian One ț <onet.cristian@gmail.com>
 * SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
 * SPDX-FileCopyrightText: 2010-2016 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KMYMONEYOCCURRENCEPERIODCOMBO_H
#define KMYMONEYOCCURRENCEPERIODCOMBO_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyoccurrencecombo.h"

/**
 * This class implements an occurrence period selector
 *
 * @author Colin Wright
 */
class KMM_BASE_WIDGETS_EXPORT KMyMoneyOccurrencePeriodCombo : public KMyMoneyOccurrenceCombo
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyOccurrencePeriodCombo)

public:
  explicit KMyMoneyOccurrencePeriodCombo(QWidget* parent = nullptr);
  ~KMyMoneyOccurrencePeriodCombo() override;

};

#endif
