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

// Gwenhywfar includes
#include <gwenhywfar/debug.h>

// QT includes
#include <QRadioButton>
#include <QLabel>
#include <QDateTime>

// KDE includes
#include <kpushbutton.h>
#include <kdatewidget.h>
#include <klocale.h>

// KMyMoney includes
#include "kmymoneydateinput.h"

#include "ui_kbpickstartdate.h"

struct KBPickStartDate::Private {
  Ui::KBPickStartDate ui;
  KMyMoneyBanking *banking;
  QDate firstPossible;
  QDate lastUpdate;
};


KBPickStartDate::KBPickStartDate(KMyMoneyBanking* qb,
                                 const QDate &firstPossible,
                                 const QDate &lastUpdate,
                                 const QString& accountName,
                                 int defaultChoice,
                                 QWidget* parent, bool modal) :
    QDialog(parent),
    d(new Private)
{
  d->ui.setupUi(this);
  d->firstPossible = firstPossible;
  d->lastUpdate = lastUpdate;
  setModal(modal);

  d->banking = qb;

  d->ui.buttonOk->setGuiItem(KStandardGuiItem::ok());
  d->ui.buttonCancel->setGuiItem(KStandardGuiItem::cancel());
  d->ui.buttonHelp->setGuiItem(KStandardGuiItem::help());

  QObject::connect(d->ui.buttonHelp, SIGNAL(clicked()),
                   this, SLOT(slotHelpClicked()));
  d->ui.label->setText(i18n("<qt><p>Please select the first date for which transactions are to be retrieved from <b>%1</b>.</p><p>If you specify no date then the bank will choose one.</p></qt>", accountName));

  if (lastUpdate.isValid()) {
    d->ui.lastUpdateLabel->setText(lastUpdate.toString());
    d->ui.lastUpdateButton->setEnabled(true);
    d->ui.lastUpdateLabel->setEnabled(true);
  } else {
    d->ui.lastUpdateButton->setEnabled(false);
    d->ui.lastUpdateLabel->setEnabled(false);
    if (defaultChoice == 2)
      defaultChoice = 1;
  }

  if (firstPossible.isValid()) {
    d->ui.firstDateLabel->setText(firstPossible.toString());
    d->ui.firstDateButton->setEnabled(true);
    d->ui.firstDateLabel->setEnabled(true);
    // As long as we use the KDateWidget we don't have
    // a chance to control the range. Once we are able
    // to use a KMyMoneyDateInput widget, we can make use
    // of the setRange() method again.
    // d->ui.pickDateEdit->setRange(firstPossible, QDate());
  } else {
    d->ui.firstDateButton->setEnabled(false);
    d->ui.firstDateLabel->setEnabled(false);
    if (defaultChoice == 3)
      defaultChoice = 1;
  }

  switch (defaultChoice) {
    case 2:
      d->ui.lastUpdateButton->setChecked(true);
      break;
    case 3:
      d->ui.firstDateButton->setChecked(true);
      break;
    default:
      d->ui.noDateButton->setChecked(true);
      break;
  }

  d->ui.pickDateEdit->setDate(QDate::currentDate());

  d->ui.buttonGroup->setFocus();
}



KBPickStartDate::~KBPickStartDate()
{
  delete d;
}


QDate KBPickStartDate::date()
{
  if (d->ui.noDateButton->isChecked())
    return QDate();
  else if (d->ui.firstDateButton->isChecked())
    return d->firstPossible;
  else if (d->ui.pickDateButton->isChecked())
    return d->ui.pickDateEdit->date();
  else if (d->ui.lastUpdateButton->isChecked())
    return d->lastUpdate;
  else {
    DBG_ERROR(0, "Unknown date state");
    return QDate();
  }
}



void KBPickStartDate::slotHelpClicked()
{
}

