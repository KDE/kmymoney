/***************************************************************************
 *   Copyright 2009  Thomas Baumgart ipwizard@users.sourceforge.net        *
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
#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif

// QBanking includes
#include "kbpickstartdate.h"
#include <q4banking/qbanking.h>

// Gwenhywfar includes
#include <gwenhywfar/debug.h>

// QT includes
#include <QRadioButton>
#include <QLabel>
#include <QDateTime>

// KDE includes
#include <kpushbutton.h>
#include <kdatewidget.h>
#include <kstdguiitem.h>
#include <klocale.h>

// KMyMoney includes
#include "kmymoneydateinput.h"



KBPickStartDate::KBPickStartDate(QBanking *qb,
                                 const QDate &firstPossible,
                                 const QDate &lastUpdate,
                                 const QString& accountName,
                                 int defaultChoice,
                                 QWidget* parent, bool modal) :
    QDialog(parent),
    _banking(qb),
    _firstPossible(firstPossible),
    _lastUpdate(lastUpdate)
{
  setupUi(this);
  setModal(modal);

  buttonOk->setGuiItem(KStandardGuiItem::ok());
  buttonCancel->setGuiItem(KStandardGuiItem::cancel());
  buttonHelp->setGuiItem(KStandardGuiItem::help());

  QObject::connect(buttonHelp, SIGNAL(clicked()),
                   this, SLOT(slotHelpClicked()));
  label->setText(i18n("<qt><p>Please select the first date for which transactions are to be retrieved from <b>%1</b>.</p><p>If you specify no date then the bank will choose one.</p></qt>", accountName));

  if (_lastUpdate.isValid()) {
    lastUpdateLabel->setText(_lastUpdate.toString());
    lastUpdateButton->setEnabled(true);
    lastUpdateLabel->setEnabled(true);
  } else {
    lastUpdateButton->setEnabled(false);
    lastUpdateLabel->setEnabled(false);
    if (defaultChoice == 2)
      defaultChoice = 1;
  }

  if (_firstPossible.isValid()) {
    firstDateLabel->setText(_firstPossible.toString());
    firstDateButton->setEnabled(true);
    firstDateLabel->setEnabled(true);
    // As long as we use the KDateWidget we don't have
    // a chance to control the range. Once we are able
    // to use a KMyMoneyDateInput widget, we can make use
    // of the setRange() method again.
    // pickDateEdit->setRange(_firstPossible, QDate());
  } else {
    firstDateButton->setEnabled(false);
    firstDateLabel->setEnabled(false);
    if (defaultChoice == 3)
      defaultChoice = 1;
  }

  switch (defaultChoice) {
    case 2:
      lastUpdateButton->setChecked(true);
      break;
    case 3:
      firstDateButton->setChecked(true);
      break;
    default:
      noDateButton->setChecked(true);
      break;
  }

  pickDateEdit->setDate(QDate::currentDate());

  buttonGroup->setFocus();
}



KBPickStartDate::~KBPickStartDate()
{
}


QDate KBPickStartDate::date()
{
  if (noDateButton->isChecked())
    return QDate();
  else if (firstDateButton->isChecked())
    return _firstPossible;
  else if (pickDateButton->isChecked())
    return pickDateEdit->date();
  else if (lastUpdateButton->isChecked())
    return _lastUpdate;
  else {
    DBG_ERROR(0, "Unknown date state");
    return QDate();
  }
}



void KBPickStartDate::slotHelpClicked()
{
  _banking->invokeHelp("KBPickStartDate", "none");
}

