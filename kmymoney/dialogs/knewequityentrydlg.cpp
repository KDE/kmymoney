/*
    SPDX-FileCopyrightText: 2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knewequityentrydlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewequityentrydlg.h"

#include "mymoneymoney.h"

class KNewEquityEntryDlgPrivate
{
  Q_DISABLE_COPY(KNewEquityEntryDlgPrivate)

public:
  KNewEquityEntryDlgPrivate() :
    ui(new Ui::KNewEquityEntryDlg),
    m_fraction(2)
  {
  }

  ~KNewEquityEntryDlgPrivate()
  {
    delete ui;
  }

  Ui::KNewEquityEntryDlg  *ui;
  QString                  m_strSymbolName;
  QString                  m_strName;
  int                      m_fraction;
};

KNewEquityEntryDlg::KNewEquityEntryDlg(QWidget *parent) :
  QDialog(parent),
  d_ptr(new KNewEquityEntryDlgPrivate)
{
  Q_D(KNewEquityEntryDlg);
  d->m_fraction = 0;
  d->ui->setupUi(this);
  setModal(true);
  d->ui->edtFraction->setCalculatorButtonVisible(true);
  d->ui->edtFraction->setPrecision(0);
  d->ui->edtFraction->setText(QLatin1String("100"));

  connect(d->ui->buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, this, &KNewEquityEntryDlg::onOKClicked);

  connect(d->ui->edtFraction, &AmountEdit::textChanged, this, &KNewEquityEntryDlg::slotDataChanged);
  connect(d->ui->edtMarketSymbol, &QLineEdit::textChanged, this, &KNewEquityEntryDlg::slotDataChanged);
  connect(d->ui->edtEquityName, &QLineEdit::textChanged, this, &KNewEquityEntryDlg::slotDataChanged);

  // add icons to buttons

  slotDataChanged();

  d->ui->edtEquityName->setFocus();
}

KNewEquityEntryDlg::~KNewEquityEntryDlg()
{
  Q_D(KNewEquityEntryDlg);
  delete d;
}

/** No descriptions */
void KNewEquityEntryDlg::onOKClicked()
{
  Q_D(KNewEquityEntryDlg);
  d->m_strSymbolName = d->ui->edtMarketSymbol->text();
  d->m_strName = d->ui->edtEquityName->text();
  d->m_fraction = d->ui->edtFraction->value().abs().formatMoney("", 0, false).toUInt();
  accept();
}

void KNewEquityEntryDlg::setSymbolName(const QString& str)
{
  Q_D(KNewEquityEntryDlg);
  d->m_strSymbolName = str;
  d->ui->edtMarketSymbol->setText(d->m_strSymbolName);
}

QString KNewEquityEntryDlg::symbolName() const
{
  Q_D(const KNewEquityEntryDlg);
  return d->m_strSymbolName;
}

void KNewEquityEntryDlg::setName(const QString& str)
{
  Q_D(KNewEquityEntryDlg);
  d->m_strName = str;
  d->ui->edtEquityName->setText(d->m_strName);
}

QString KNewEquityEntryDlg::name() const
{
  Q_D(const KNewEquityEntryDlg);
  return d->m_strName;
}

int KNewEquityEntryDlg::fraction() const
{
  Q_D(const KNewEquityEntryDlg);
  return d->m_fraction;
}

void KNewEquityEntryDlg::slotDataChanged()
{
  Q_D(KNewEquityEntryDlg);
  auto okEnabled = true;

  if (!d->ui->edtFraction->value().isPositive()
      || d->ui->edtMarketSymbol->text().isEmpty()
      || d->ui->edtEquityName->text().isEmpty())
    okEnabled = false;

  d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(okEnabled);
}
