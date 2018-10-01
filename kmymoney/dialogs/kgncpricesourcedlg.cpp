/***************************************************************************
                          kgncpricesourcedlg.cpp
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield <tonybloom@users.sourceforge.net>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kgncpricesourcedlg.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QLabel>
#include <QLayout>
#include <QApplication>
#include <QButtonGroup>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kapplication.h>
#include <kurlrequester.h>
#include <ktextbrowser.h>
#include <klocale.h>
#include <ktoolinvocation.h>
#include <klistwidget.h>

// ----------------------------------------------------------------------------
// Project Includes
#include <alkimia/alkonlinequotesprofilemanager.h>
#include "ui_kgncpricesourcedlgdecl.h"


class KGncPriceSourceDlgDecl : public QWidget, public Ui::KGncPriceSourceDlgDecl
{
public:
  KGncPriceSourceDlgDecl() {
    setupUi(this);
  }
};


struct KGncPriceSourceDlg::Private {
  Private() : currentButton(0), widget(0) {}
  int currentButton;
  KGncPriceSourceDlgDecl* widget;
};

KGncPriceSourceDlg::KGncPriceSourceDlg(QWidget *parent) : KDialog(parent), d(new Private)
{
}

KGncPriceSourceDlg::KGncPriceSourceDlg(const QString &stockName, const QString& gncSource, QWidget * parent) : KDialog(parent), d(new Private)
{
  setButtons(Ok | Help);
  d->widget = new KGncPriceSourceDlgDecl();
  setMainWidget(d->widget);
  // signals and slots connections
  connect(d->widget->buttonsSource, SIGNAL(buttonClicked(int)), this, SLOT(buttonPressed(int)));
  connect(this, SIGNAL(helpClicked()), this, SLOT(slotHelp()));
  // initialize data fields
  d->widget->textStockName->setText(i18n("Investment: %1", stockName));
  d->widget->textGncSource->setText(i18n("Quote source: %1", gncSource));
  d->widget->listKnownSource->clear();
  d->widget->listKnownSource->insertItems(0, AlkOnlineQuotesProfileManager::instance().profiles().first()->quoteSources());
  d->widget->lineUserSource->setText(gncSource);
  d->widget->checkAlwaysUse->setChecked(true);
  d->widget->buttonsSource->setId(d->widget->buttonNoSource, 0);
  d->widget->buttonsSource->setId(d->widget->buttonSelectSource, 1);
  d->widget->buttonsSource->setId(d->widget->buttonUserSource, 2);
  d->widget->buttonsSource->button(0)->setChecked(true);
  buttonPressed(0);
  return;
}

KGncPriceSourceDlg::~KGncPriceSourceDlg()
{
  delete d;
}

enum ButtonIds {NOSOURCE = 0, KMMSOURCE, USERSOURCE};

void KGncPriceSourceDlg::buttonPressed(int buttonId)
{
  d->currentButton = buttonId;
  switch (d->currentButton) {
    case NOSOURCE:
      d->widget->listKnownSource->clearSelection();
      d->widget->listKnownSource->setEnabled(false);
      d->widget->lineUserSource->deselect();
      d->widget->lineUserSource->setEnabled(false);
      break;
    case KMMSOURCE:
      d->widget->lineUserSource->deselect();
      d->widget->lineUserSource->setEnabled(false);
      d->widget->listKnownSource->setEnabled(true);
      d->widget->listKnownSource->setFocus();
      d->widget->listKnownSource->setCurrentRow(0);
      break;
    case USERSOURCE:
      d->widget->listKnownSource->clearSelection();
      d->widget->listKnownSource->setEnabled(false);
      d->widget->lineUserSource->setEnabled(true);
      d->widget->lineUserSource->selectAll();
      d->widget->lineUserSource->setFocus();
      break;
  }
}

QString KGncPriceSourceDlg::selectedSource() const
{
  QString s;
  switch (d->currentButton) {
    case NOSOURCE:
      s = "";
      break;
    case KMMSOURCE:
      s = d->widget->listKnownSource->currentItem()->text();
      break;
    case USERSOURCE:
      s = d->widget->lineUserSource->text();
      break;
  }
  return (s);
}

bool KGncPriceSourceDlg::alwaysUse() const
{
  return d->widget->checkAlwaysUse->isChecked();
}

void KGncPriceSourceDlg::slotHelp()
{
  KToolInvocation::invokeHelp("details.impexp.gncquotes");
}
