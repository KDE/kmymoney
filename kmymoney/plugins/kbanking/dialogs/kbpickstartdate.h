/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2004  Martin Preuss aquamaniac@users.sourceforge.net        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/
#ifndef KBPICKSTARTDATE_H
#define KBPICKSTARTDATE_H

#include <ui_kbpickstartdate.h>
#include <qdatetime.h>
#include <qdialog.h>


class QBanking;

/**
  * Class derived from QBPickStartDate and modified to
  * be based on KDE widgets
  *
  * @author Martin Preuss
  * @author Thomas Baumgart
  */
class KBPickStartDate : public QDialog, public Ui::KBPickStartDateUi
{
  Q_OBJECT
private:
  QBanking *_banking;
  const QDate &_firstPossible;
  const QDate &_lastUpdate;
public:
  KBPickStartDate(QBanking *banking,
                  const QDate &firstPossible,
                  const QDate &lastUpdate,
                  const QString& accountName,
                  int defaultChoice,
                  QWidget* parent=0,
                  bool modal=false);
  ~KBPickStartDate();

  QDate date();

public slots:
  void slotHelpClicked();
};

#endif
