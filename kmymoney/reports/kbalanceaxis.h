/***************************************************************************
                          kbalanceaxis.h  -  description
                             -------------------
    begin                : Sun Jul 18 2010
    copyright            : (C) 2010 by Alvaro Soliverez
    email                : asoliverez@kde.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBALANCEAXIS_H
#define KBALANCEAXIS_H

#include <qobjectdefs.h>

#include <KChartCartesianAxis>

namespace KChart { class AbstractCartesianDiagram; }

class KBalanceAxis : public KChart::CartesianAxis
{
  Q_OBJECT
public:
  KBalanceAxis();
  explicit KBalanceAxis(KChart::AbstractCartesianDiagram* parent);
  const QString customizedLabel(const QString& label) const;
};


#endif
