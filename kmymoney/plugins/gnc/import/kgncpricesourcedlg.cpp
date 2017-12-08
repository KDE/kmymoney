/***************************************************************************
                          kgncpricesourcedlg.cpp
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield <tonybloom@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

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
#include <QButtonGroup>
#include <QListWidget>
#include <QDialogButtonBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "webpricequote.h"
#include "ui_kgncpricesourcedlg.h"

class KGncPriceSourceDlgPrivate
{
  Q_DISABLE_COPY(KGncPriceSourceDlgPrivate)

public:
  KGncPriceSourceDlgPrivate() :
    ui(new Ui::KGncPriceSourceDlg),
    currentButton(0)
  {
  }

  ~KGncPriceSourceDlgPrivate()
  {
    delete ui;
  }

  Ui::KGncPriceSourceDlg  *ui;
  int currentButton;
};


KGncPriceSourceDlg::KGncPriceSourceDlg(const QString &stockName, const QString& gncSource, QWidget * parent) :
  QDialog(parent),
  d_ptr(new KGncPriceSourceDlgPrivate)
{
  Q_D(KGncPriceSourceDlg);
  d->ui->setupUi(this);
  // signals and slots connections
  connect(d->ui->buttonsSource, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &KGncPriceSourceDlg::buttonPressed);
  connect(d->ui->buttonBox, &QDialogButtonBox::helpRequested, this, &KGncPriceSourceDlg::slotHelp);
  // initialize data fields
  d->ui->textStockName->setText(i18n("Investment: %1", stockName));
  d->ui->textGncSource->setText(i18n("Quote source: %1", gncSource));
  d->ui->listKnownSource->clear();
//  TODO: return this feature
//  d->ui->listKnownSource->insertItems(0, WebPriceQuote::quoteSources());
  d->ui->lineUserSource->setText(gncSource);
  d->ui->checkAlwaysUse->setChecked(true);
  d->ui->buttonsSource->setId(d->ui->buttonNoSource, 0);
  d->ui->buttonsSource->setId(d->ui->buttonSelectSource, 1);
  d->ui->buttonsSource->setId(d->ui->buttonUserSource, 2);
  d->ui->buttonsSource->button(0)->setChecked(true);
  buttonPressed(0);
}

KGncPriceSourceDlg::~KGncPriceSourceDlg()
{
  Q_D(KGncPriceSourceDlg);
  delete d;
}

enum ButtonIds {NOSOURCE = 0, KMMSOURCE, USERSOURCE};

void KGncPriceSourceDlg::buttonPressed(int buttonId)
{
  Q_D(KGncPriceSourceDlg);
  d->currentButton = buttonId;
  switch (d->currentButton) {
    case NOSOURCE:
      d->ui->listKnownSource->clearSelection();
      d->ui->listKnownSource->setEnabled(false);
      d->ui->lineUserSource->deselect();
      d->ui->lineUserSource->setEnabled(false);
      break;
    case KMMSOURCE:
      d->ui->lineUserSource->deselect();
      d->ui->lineUserSource->setEnabled(false);
      d->ui->listKnownSource->setEnabled(true);
      d->ui->listKnownSource->setFocus();
      d->ui->listKnownSource->setCurrentRow(0);
      break;
    case USERSOURCE:
      d->ui->listKnownSource->clearSelection();
      d->ui->listKnownSource->setEnabled(false);
      d->ui->lineUserSource->setEnabled(true);
      d->ui->lineUserSource->selectAll();
      d->ui->lineUserSource->setFocus();
      break;
  }
}

QString KGncPriceSourceDlg::selectedSource() const
{
  Q_D(const KGncPriceSourceDlg);
  switch (d->currentButton) {
    case KMMSOURCE:
      return d->ui->listKnownSource->currentItem()->text();
    case USERSOURCE:
      return d->ui->lineUserSource->text();
    case NOSOURCE:
    default:
      return QString();
  }
}

bool KGncPriceSourceDlg::alwaysUse() const
{
  Q_D(const KGncPriceSourceDlg);
  return d->ui->checkAlwaysUse->isChecked();
}

void KGncPriceSourceDlg::slotHelp()
{
  KHelpClient::invokeHelp("details.impexp.gncquotes");
}
