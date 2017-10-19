/***************************************************************************
                          knewbudgetdlg.cpp
                             -------------------
    begin                : Wed Jan 18 2006
    copyright            : (C) 2000-2004 by Darren Gould
    email                : darren_gould@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "knewbudgetdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewbudgetdlgdecl.h"

struct KNewBudgetDlg::Private {
  QString m_year;
  QString m_name;

  Ui::KNewBudgetDlgDecl ui;
};

// the combobox should look m_icNextYears into the future
static const int icFutureYears = 5;
static const int icPastYears = 2;

KNewBudgetDlg::KNewBudgetDlg(QWidget* parent) :
    QDialog(parent), d(new Private)
{
  d->ui.setupUi(this);
  QStringList slYear;
  QDate dToday = QDate::currentDate();
  int iYear = dToday.year();

  for (int i = 0; i <= icFutureYears; i++)
    d->ui.m_cbYear->addItem(QString::number(iYear++));

  iYear = dToday.year();
  for (int i = 0; i <= icPastYears; i++)
    d->ui.m_cbYear->addItem(QString::number(--iYear));

  connect(d->ui.buttonBox, SIGNAL(accepted()), this, SLOT(m_pbOk_clicked()));
  connect(d->ui.buttonBox, SIGNAL(rejected()), this, SLOT(m_pbCancel_clicked()));
}

KNewBudgetDlg::~KNewBudgetDlg()
{
  delete d;
}

void KNewBudgetDlg::m_pbCancel_clicked()
{
  reject();
}

void KNewBudgetDlg::m_pbOk_clicked()
{
  // force focus change to update all data
  d->ui.buttonBox->button(QDialogButtonBox::Ok)->setFocus();

  if (d->ui.m_leBudgetName->displayText().isEmpty()) {
    KMessageBox::information(this, i18n("Please specify a budget name"));
    d->ui.m_leBudgetName->setFocus();
    return;
  }

  d->m_year = d->ui.m_cbYear->currentText();
  d->m_name = d->ui.m_leBudgetName->displayText();

  accept();
}

QString& KNewBudgetDlg::getYear()
{
  return d->m_year;
}

QString& KNewBudgetDlg::getName()
{
  return d->m_name;
}
