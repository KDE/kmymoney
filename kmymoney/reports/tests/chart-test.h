/***************************************************************************
                          reportcharttest.cpp
                          -------------------
    copyright            : (C) 2017 by Ralf Habacker
    email                : ralf.habacker@freenet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QObject>

class ChartTest: public QObject
{
  Q_OBJECT

private slots:
  void createChart();

};
