/***************************************************************************
                             kbankaccountedit.cpp
                             -------------------
    copyright            : (C) 2013 by Stephan Zodrow
    email                : szodrow@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kbankaccountedit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QTabWidget>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QPixmap>
#include <QLayout>
#include <QList>
#include <QDebug> // szo test only
#include <QMessageBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KListWidget>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <KToggleAction>
#include <fixx11h.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kmymoneyview.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "kmymoneyaccounttreeview.h"
#include "models.h"

#include <ktoblzcheck.h>

KBankAccountEdit::KBankAccountEdit(QWidget *parent, KBankAccountEdit::editMode editAccountMode,
  KBankAccountEdit::transactionSystemType transactionSystem
) :
    QWidget(parent)
{
  qDebug() << "KBankAccountEdit Constructor";
  setupUi(this);
  m_editAccountMode = allFields;
  m_transactionSystem = transactionSystem;
  
  m_countryCode = QString("DE"); // szo testing with "DE"
  setIsDefaultAccount(false); // Default must be inline with GUI checkbox Default
  
  connect (transactionSystemEdit, SIGNAL(currentIndexChanged (const QString)), SLOT(slotTransActionSystemChanged(const QString )));
  connect (bankAccountOwnerNameEdit, SIGNAL(textEdited(QString)), this, SLOT(slotTextChanged()));
  connect (bankAccountNumberEdit, SIGNAL(textEdited(QString)), this, SLOT(slotTextChanged()));
  connect (bankCodeEdit, SIGNAL(textEdited(QString)), this, SLOT(slotTextChanged()));
  connect (bankNameEdit, SIGNAL(textEdited(QString)), this, SLOT(slotTextChanged()));
  connect (accountNameEdit, SIGNAL(textEdited(QString)), this, SLOT(slotTextChanged()));

  connect (bankCodeEdit, SIGNAL(returnPressed(QString)), this, SLOT(slotBankCodeChanged()));

  if (m_editAccountMode == KBankAccountEdit::transactionOnly ) {
    transactionSystemEdit->setEnabled(false);
    countryCodeEdit->setEnabled(false);
    accountNameLabel->setVisible(false);
    accountNameEdit->setVisible(false);
    defaultAccountLabel->setVisible(false);
    defaultAccountEdit->setVisible(false);
    defaultAccountEdit->setEnabled(false);
  }
  setTransactionProperties(m_countryCode, m_transactionSystem); 
}

KBankAccountEdit::~KBankAccountEdit()
{
}

void KBankAccountEdit::slotTextChanged()
{
  emit dataChanged();
}

void KBankAccountEdit::slotBankCodeChanged()
{
  if ( ! bankCodeEdit->text().isEmpty() ) {
    isBankCodeOk(taSystemTextToEnmum(transactionSystemEdit->currentText(), GuiText), true);
  }
}

void KBankAccountEdit::slotTransActionSystemChanged(const QString & text)
{
  qDebug() << "slotTransActionSystemChanged: " << text;
  switchEditorLayout(taSystemTextToEnmum(text, GuiText));
  setTransactionProperties(countryCodeEdit-> currentText(), taSystemTextToEnmum(text, GuiText));
}



bool KBankAccountEdit::isData()
{
  // this check depends on the editMode
  if (   ! bankAccountOwnerNameEdit->text().isEmpty()
      || ! bankAccountNumberEdit->text().isEmpty()
      || ! bankCodeEdit->text().isEmpty()
      || ! bankNameEdit->text().isEmpty()
      || (! accountNameEdit->text().isEmpty() || m_editAccountMode == transactionOnly)
     ) {
    return true;
  }
  return false;
}

bool KBankAccountEdit::isDataChanged()
{
  bool rc = false;

  
  rc |= bankAccountOwnerNameEdit->isModified();
  rc |= bankAccountNumberEdit->isModified();
  rc |= bankCodeEdit->isModified();
  rc |= bankNameEdit->isModified();
  if ( transactionSystemEdit->isEnabled() ) {
    if ( taSystemTextToEnmum(transactionSystemEdit->currentText(), GuiText) != m_transactionSystem ) {
      rc |= true;
    }
  }
  if ( countryCodeEdit->isEnabled() ) {
    if (   countryCodeEdit->currentText() != m_countryCode ) {
      rc |= true;
    }
  }
  if ( defaultAccountEdit->isEnabled() ) {
    if (   defaultAccountEdit->isChecked() ) { // if enabled and checked then change has occurred
      rc |= true;
    }
  }

  return rc;
}

bool KBankAccountEdit::isDataOk()
{
  bool errorFound = false;
  m_errorMsg =QString("");
  int i;
  transactionSystemType m_ta;
  m_ta = taSystemTextToEnmum(transactionSystemEdit->currentText(), GuiText );
  QString m_country_code;
  m_country_code = countryCodeEdit-> currentText();
  
  if ( !isData() ) {
    return true;
  }
  
  if (! errorFound && ! rxOwnerName.exactMatch(bankAccountOwnerNameEdit->text()) ) {
    m_errorMsg = i18n("Account Owner: wrong format");
    errorFound = true;
  }
  if (! errorFound && ! rxAccountNumber.exactMatch(bankAccountNumberEdit->text()) ) {
    m_errorMsg = i18n("Account Number: wrong format");
    errorFound = true;
  }
  if (! errorFound && ! rxBankCode.exactMatch(bankCodeEdit->text()) ) {
    m_errorMsg = i18n("Bank Code: wrong format");
    errorFound = true;
  }
  // check length of BIC
  if (! errorFound && m_ta == sepa ) {
    i = bankCodeEdit->text().length();
    if (i != 8 && i != 11) {
      m_errorMsg = i18n("BIC must have 8 or 11 digits");
      errorFound = true;
    }
  }
  if (! errorFound && bankNameEdit->text().isEmpty() ) {
    m_errorMsg = i18n("Bank Name: missing");
    errorFound = true;
  }
  if ( errorFound ) {
    qDebug() << "KBankAccountEdit::isDataOk" << m_errorMsg;
    return false;
  }
  return true;
}

bool KBankAccountEdit::isBankCodeOk (transactionSystemType transactionSystem, bool signalErrorMsg)
{
  qDebug() << "slotCheckBankCode: " << bankCodeEdit->text();

  if ( transactionSystem = sepa ) 
    return true;
  AccountNumberCheck ktoblzcheck;
  AccountNumberCheck::Record bank;
  try {
    bank = ktoblzcheck.findBank(std::string(bankCodeEdit->text().toLatin1()));
  } catch (int) {
    // Exception sent if bank not found
    QMessageBox msgBox;
    msgBox.setText("Bank Code is invalid.");
    msgBox.exec();
    bankCodeEdit->setFocus();
    return false;
  }
  QString bankName = QString("%1, %2").arg(QString::fromLatin1(bank.bankName.c_str()))
  .arg(QString::fromLatin1(bank.location.c_str()));
  if ( bankName.isEmpty() ) {
    QMessageBox msgBox;
    msgBox.setText("Bank Code not found.");
    msgBox.exec();
    bankCodeEdit->setFocus();
    return false;
  }
    return true;
    
}

void KBankAccountEdit::switchEditorLayout(const transactionSystemType & transactionSystem)
{
  QString x;
  x = taSystemEnmumToText(transactionSystem, GuiText);
  qDebug() << "switchEditorLayout: " << x;
  if ( transactionSystem == national ) {
    bankAccountNumberLabel->setText(i18n("Account Number"));
    bankCodeLabel->setText(i18n("Bank Code"));
    bankNameLabel->setText(i18n("Bank Name"));
  }
  if ( transactionSystem == sepa ) {
    bankAccountNumberLabel->setText(i18n("IBAN"));
    bankCodeLabel->setText(i18n("BIC"));
    bankNameLabel->setText(i18n("Bank Name"));
  }
  if ( transactionSystem == paypal ) {
    bankAccountNumberLabel->setText(i18n("PayPalNo?"));
    bankCodeLabel->setText(i18n("PayPalBankCode?"));
    bankNameLabel->setText(i18n("PayPal?"));
  }

}


void KBankAccountEdit::setTransactionProperties(const QString& countryCode, const transactionSystemType transactionSystem)
{
  QString lineEditText;
  bankAccountOwnerNameEdit->setInputMask(NULL);
  bankAccountOwnerNameEdit->setMaxLength(32767);
  bankAccountOwnerNameEdit->setValidator(NULL);
  bankAccountNumberEdit->setInputMask(NULL);
  bankAccountNumberEdit->setMaxLength(32767);
  bankAccountNumberEdit->setValidator(NULL);
  bankCodeEdit->setInputMask(NULL);
  bankCodeEdit->setMaxLength(32767);
  bankCodeEdit->setValidator(NULL);
  
  if ( transactionSystem == national ) {
    if ( countryCode == QString("DE") ) {
      
      lineEditText = bankAccountOwnerNameEdit->text();
      bankAccountOwnerNameEdit->setValidator(0);
      bankAccountOwnerNameEdit->setText("");
      rxOwnerName.setPattern("[a-zA-Z0-9\':\?,-\(+_)/ ]{0,27}"); // allowed according SEPA, also OK for HBCI
      QValidator *validatorOwnerName = new QRegExpValidator(rxOwnerName, this);
      bankAccountOwnerNameEdit->setValidator(validatorOwnerName);
      bankAccountOwnerNameEdit->setText(lineEditText);

      lineEditText = bankAccountNumberEdit->text();
      bankAccountNumberEdit->setValidator(0);
      bankAccountNumberEdit->setText("");
      rxAccountNumber.setPattern("\\d{1,10}"); 
      QValidator *validatorAccountNumber = new QRegExpValidator(rxAccountNumber, this);
      bankAccountNumberEdit->setValidator(validatorAccountNumber);
      bankAccountNumberEdit->setText(lineEditText);

      lineEditText = bankCodeEdit->text();
      bankCodeEdit->setValidator(0);
      bankCodeEdit->setText("");
      rxBankCode.setPattern("\\d{8}"); 
      QValidator *validatorBankCode = new QRegExpValidator(rxBankCode, this);
      bankCodeEdit->setValidator(validatorBankCode);
      bankCodeEdit->setText(lineEditText);
    }
  }
  
  if ( transactionSystem == sepa ) {
    lineEditText = bankAccountOwnerNameEdit->text();
    bankAccountOwnerNameEdit->setValidator(0);
    bankAccountOwnerNameEdit->setText("");
    rxOwnerName.setPattern("[a-zA-Z0-9\':\?,-\(+_)/ ]{0,27}"); // allowed according SEPA, also OK for HBCI
    QValidator *validatorOwnerName = new QRegExpValidator(rxOwnerName, this);
    bankAccountOwnerNameEdit->setValidator(validatorOwnerName);
    bankAccountOwnerNameEdit->setText(lineEditText);

    lineEditText = bankAccountNumberEdit->text();
    bankAccountNumberEdit->setValidator(0);
    bankAccountNumberEdit->setText("");
    rxAccountNumber.setPattern("[a-zA-Z]{2}\\d{20}"); 
    QValidator *validatorAccountNumber = new QRegExpValidator(rxAccountNumber, this);
    bankAccountNumberEdit->setValidator(validatorAccountNumber);
    bankAccountNumberEdit->setText(lineEditText);

    lineEditText = bankCodeEdit->text();
    bankCodeEdit->setValidator(0);
    bankCodeEdit->setText("");
    rxBankCode.setPattern("[a-zA-Z]{8,11}"); 
    QValidator *validatorBankCode = new QRegExpValidator(rxBankCode, this);
    bankCodeEdit->setValidator(validatorBankCode);
    bankCodeEdit->setText(lineEditText);
  }
  switchEditorLayout(transactionSystem);
}

void KBankAccountEdit::clearItemData(void)
{
  setAccountOwner(QString());
  setAccountNumber(QString());
  setBankCode(QString());
  setBankName(QString());
  setTransactionSystem(national);
}

void KBankAccountEdit::setAccountOwner(const QString& val)
{
  bankAccountOwnerNameEdit->setText(val);
}
void KBankAccountEdit::setAccountNumber(const QString& val)
{
  bankAccountNumberEdit->setText(val);
}
void KBankAccountEdit::setBankCode(const QString& val)
{
  bankCodeEdit->setText(val);
}
void KBankAccountEdit::setBankName(const QString& val)
{
  bankNameEdit->setText(val);
}
void KBankAccountEdit::setCountryCode(const QString& val)
{
  if ( val == QString("DE") ) {
    countryCodeEdit->setCurrentIndex(countryCodeEdit->findText("DE"));
  } else {
    countryCodeEdit->setCurrentIndex(countryCodeEdit->findText(""));
  };
}

void KBankAccountEdit::setTransactionSystem(transactionSystemType val)
{
  m_transactionSystem = val;
  QString x;
  x = taSystemEnmumToText(val, GuiText);
  qDebug() << "setTransactionSystem: " << x;
  transactionSystemEdit->setCurrentIndex(transactionSystemEdit->findText( taSystemEnmumToText(val, GuiText) ));
}

void KBankAccountEdit::setTransactionSystem(const QString& val, translation translateFrom)
{
  setTransactionSystem(taSystemTextToEnmum(val,translateFrom));
}

void KBankAccountEdit::setAccountDataMinimal(const QString& accountOwner, const QString& accountNumber, 
			  const QString& bankCode, const QString& bankName,
			  const QString& countryCode, transactionSystemType transactionSystem)
{
  setAccountOwner(accountOwner);
  setAccountNumber(accountNumber);
  setBankCode(bankCode);
  setBankName(bankName);
  setCountryCode(countryCode);
  setTransactionSystem(transactionSystem);
}

void KBankAccountEdit::setAccountName(const QString& val)
{
  accountNameEdit->setText(val);
}

void KBankAccountEdit::setIsDefaultAccount(const QString& val)
{
  if ( val == "X" ) 
    setIsDefaultAccount(true);
  else
    setIsDefaultAccount(false);
}

void KBankAccountEdit::setIsDefaultAccount(bool val)
{
  if ( val ) 
    defaultAccountEdit->setCheckState ( Qt::Checked);
  else
    defaultAccountEdit->setCheckState ( Qt::Unchecked);
}

void KBankAccountEdit::enableIsDefaultAccount(bool val)
{
  if ( ! defaultAccountEdit == NULL ) 
    defaultAccountEdit->setEnabled(val);
}

QString KBankAccountEdit::transactionSystemText(translation translateFrom) //  default GuiText
{
  return taSystemEnmumToText(m_transactionSystem, translateFrom);
}

KBankAccountEdit::transactionSystemType KBankAccountEdit::taSystemTextToEnmum(const QString & text, translation translateFrom)
{
  transactionSystemType rtn;
  if ( (text == i18n("National") && translateFrom == GuiText) 
      || (text == "national" && translateFrom == DataText)   ) 
	rtn = national;
  if ( (text == i18n("Sepa") && translateFrom == GuiText) 
      || (text == "sepa" && translateFrom == DataText)   ) 
	rtn = sepa;
  if ( (text == i18n("PayPal") && translateFrom == GuiText) 
      || (text == "paypal" && translateFrom == DataText)   ) 
	rtn = paypal;
  return rtn;
}

QString KBankAccountEdit::taSystemEnmumToText(transactionSystemType taSystem, translation translateTo)
{
  QString rtn;
  switch(translateTo) {
    case GuiText:
      switch(taSystem) {
	case national:
	  rtn = i18n("National");
	  break;
	case sepa:
	  rtn = i18n("Sepa");
	  break;
	case paypal:
	  rtn = i18n("PayPal");
	  break;
      };
    break;
  case DataText:
      switch(taSystem) {
	case national:
	  rtn = "national";
	  break;
	case sepa:
	  rtn = "sepa";
	  break;
	case paypal:
	  rtn = "paypal";
	  break;
      };
    break;
  }
  return rtn;
}

#include "kbankaccountedit.moc"
