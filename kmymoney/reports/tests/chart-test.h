/*
 * Copyright 2016-2018  Christian Dávid <christian-david@web.de>
 * Copyright 2017       Ralf Habacker <ralf.habacker@freenet.de>
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

#ifndef CHARTTEST_H
#define CHARTTEST_H

#include <QObject>

class ChartTest: public QObject
{
  Q_OBJECT

private slots:
  void createChart();

};

#endif // CHARTTEST_H
