/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Alvaro Soliverez <asoliverez@gmail.com>
    (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef REPORTCONTROLIMPL_H
#define REPORTCONTROLIMPL_H

#include <QWidget>

namespace Ui { class ReportControl; }

class ReportControl : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(ReportControl)

public:
  explicit ReportControl(QWidget *parent);
  ~ReportControl();

  Ui::ReportControl* ui;
};
#endif /* REPORTCONTROLIMPL_H */
