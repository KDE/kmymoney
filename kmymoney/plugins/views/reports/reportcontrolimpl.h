/*  This file is part of the KDE project
    Copyright (C) 2009 Alvaro Soliverez <asoliverez@gmail.com>
    (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
