/*
    SPDX-FileCopyrightText: 2009 Thomas Baumgart ipwizard @users.sourceforge.net
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2004 Martin Preuss aquamaniac @users.sourceforge.net
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
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
#include <QPushButton>

// KDE includes
#include <KLocalizedString>

// KMyMoney includes
#include "kmymoneydateinput.h"

#include "ui_kbpickstartdate.h"

struct KBPickStartDate::Private {
  Ui::KBPickStartDate ui;
  KBankingExt *banking;
  QDate firstPossible;
  QDate lastUpdate;
};


KBPickStartDate::KBPickStartDate(KBankingExt* qb,
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

  QObject::connect(d->ui.buttonBox, SIGNAL(helpRequested()),
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

