/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "securitydlg.h"

#include <QPushButton>

#include "ui_securitydlg.h"

#include "mymoneyfile.h"
#include "mymoneysecurity.h"

SecurityDlg::SecurityDlg() : ui(new Ui::SecurityDlg)
{
  ui->setupUi(this);
  m_buttonOK = ui->buttonBox->button(QDialogButtonBox::Ok);
  m_buttonOK->setDefault(true);
  m_buttonOK->setShortcut(Qt::CTRL | Qt::Key_Return);
  m_buttonOK->setEnabled(false);

  connect(ui->cbSecurity, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SecurityDlg::slotIndexChanged);
  connect(ui->leNewSymbol, &QLineEdit::editingFinished, this, &SecurityDlg::slotEditingFinished);
  connect(ui->leNewName, &QLineEdit::editingFinished, this, &SecurityDlg::slotEditingFinished);
}

SecurityDlg::~SecurityDlg()
{
  delete ui;
}

void SecurityDlg::initializeSecurities(const QString& presetSymbol, const QString& presetName)
{
  QList<MyMoneySecurity> securities = MyMoneyFile::instance()->securityList();
  if (!securities.isEmpty())
    ui->cbSecurity->setEnabled(true);
  else {
    ui->cbSecurity->setEnabled(false);
    return;
  }
  ui->cbSecurity->blockSignals(true);
  int presetIndex = -1;
  for (QList<MyMoneySecurity>::const_iterator security = securities.cbegin(); security != securities.cend(); ++security) {
    QString symbol = (*security).tradingSymbol();
    QString name = (*security).name();
    if(symbol == presetSymbol && name == presetName)
      presetIndex = ui->cbSecurity->count();
    ui->cbSecurity->addItem((*security).name(), QVariant(((*security).id())));
  }
  ui->cbSecurity->blockSignals(false);
  ui->cbSecurity->setCurrentIndex(presetIndex);
  emit ui->cbSecurity->currentIndexChanged(presetIndex); // in case currentIndex == presetIndex and no signal would be emitted
}

QString SecurityDlg::security() {
  return ui->cbSecurity->currentData().toString();
}

QString SecurityDlg::name() {
  return ui->leNewName->text();
}

QString SecurityDlg::symbol() {
  return ui->leNewSymbol->text();
}

int SecurityDlg::dontAsk() {
  return int(ui->cbDontAsk->isChecked());
}

void SecurityDlg::slotEditingFinished()
{
  if (ui->leNewName->text().isEmpty() && ui->leNewSymbol->text().isEmpty()
      && ui->cbSecurity->currentIndex() == -1) {
    if (ui->cbSecurity->count() > 0)
      ui->cbSecurity->setEnabled(true);
    m_buttonOK->setEnabled(false);
  } else if (!ui->leNewName->text().isEmpty() || !ui->leNewSymbol->text().isEmpty()) {
    ui->cbSecurity->setEnabled(false);
    ui->cbSecurity->setCurrentIndex(-1);
    if (!ui->leNewName->text().isEmpty() && !ui->leNewSymbol->text().isEmpty())
      m_buttonOK->setEnabled(true);
  }
}

void SecurityDlg::slotIndexChanged(int index)
{
  if (index != -1)
    m_buttonOK->setEnabled(true);
  else
    m_buttonOK->setEnabled(false);
}
