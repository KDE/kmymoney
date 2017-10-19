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
#include <QButtonGroup>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "webpricequote.h"
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

KGncPriceSourceDlg::KGncPriceSourceDlg(QWidget *parent) : QDialog(parent), d(new Private)
{
}

KGncPriceSourceDlg::KGncPriceSourceDlg(const QString &stockName, const QString& gncSource, QWidget * parent) : QDialog(parent), d(new Private)
{
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Help);
  QWidget *mainWidget = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(mainWidget);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  d->widget = new KGncPriceSourceDlgDecl();
  mainLayout->addWidget(d->widget);
  // signals and slots connections
  connect(d->widget->buttonsSource, SIGNAL(buttonClicked(int)), this, SLOT(buttonPressed(int)));
  connect(this, SIGNAL(helpClicked()), this, SLOT(slotHelp()));
  // initialize data fields
  d->widget->textStockName->setText(i18n("Investment: %1", stockName));
  d->widget->textGncSource->setText(i18n("Quote source: %1", gncSource));
  d->widget->listKnownSource->clear();
  d->widget->listKnownSource->insertItems(0, WebPriceQuote::quoteSources());
  d->widget->lineUserSource->setText(gncSource);
  d->widget->checkAlwaysUse->setChecked(true);
  d->widget->buttonsSource->setId(d->widget->buttonNoSource, 0);
  d->widget->buttonsSource->setId(d->widget->buttonSelectSource, 1);
  d->widget->buttonsSource->setId(d->widget->buttonUserSource, 2);
  d->widget->buttonsSource->button(0)->setChecked(true);
  mainLayout->addWidget(buttonBox);
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
  KHelpClient::invokeHelp("details.impexp.gncquotes");
}
