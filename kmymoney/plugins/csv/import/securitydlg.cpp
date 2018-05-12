/*
 * Copyright 2017  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

  connect(ui->cbSecurity,  SIGNAL(currentIndexChanged(int)), this,  SLOT(slotIndexChanged(int)));
  connect(ui->leNewSymbol,  SIGNAL(editingFinished()), this,  SLOT(slotEditingFinished()));
  connect(ui->leNewName,  SIGNAL(editingFinished()), this,  SLOT(slotEditingFinished()));
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
