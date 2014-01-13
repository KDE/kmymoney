/*
  This file is part of KMyMoney, A Personal Finance Manager for KDE
  Copyright (C) 2013 Christian Dávid <christian-david@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sepacredittransferedit.h"
#include "sepacredittransferedit_p.h"
#include "ui_sepacredittransferedit.h"
#include "kguiutils.h"

#include "mymoney/swiftaccountidentifier.h"

sepaCreditTransferEdit::sepaCreditTransferEdit(QWidget *parent) :
    IonlineJobEdit(parent),
    ui(new Ui::sepaCreditTransferEdit),
    m_onlineJob( onlineJobTyped<sepaOnlineTransfer>( new sepaOnlineTransfer ) ),
    m_requiredFields( new kMandatoryFieldGroup(this) )
{
    ui->setupUi(this);
    
    ui->beneficiaryBankCode->setValidator( new bicValidator(ui->beneficiaryBankCode) );
    ui->beneficiaryAccNum->setValidator( new ibanValidator(ui->beneficiaryAccNum) );
    
    ui->statusBeneficiaryName->setVisible(false);
    ui->statusIban->setVisible(false);
    ui->statusBic->setVisible(false);
    ui->statusAmount->setVisible(false);
    ui->statusPurpose->setVisible(false);
    ui->statusReference->setVisible(false);
    
    m_requiredFields->add(ui->beneficiaryAccNum);
    m_requiredFields->add(ui->value);
    // Other required fields are set in updateSettings()
    
    connect(m_requiredFields, SIGNAL(stateChanged(bool)), this, SLOT(requiredFieldsCompleted(bool)));
    
    connect(ui->beneficiaryName, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryNameChanged(QString)));
    connect(ui->beneficiaryAccNum, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryIbanChanged(QString)));
    connect(ui->beneficiaryBankCode, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryBicChanged(QString)));
    connect(ui->value, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged()));
    connect(ui->sepaReference, SIGNAL(textChanged(QString)), this, SLOT(endToEndReferenceChanged(QString)));
    connect(ui->purpose, SIGNAL(textChanged()), this, SLOT(purposeChanged()));
    
    connect(ui->beneficiaryName, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->beneficiaryAccNum, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->beneficiaryBankCode, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->value, SIGNAL(valueChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->sepaReference, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
    connect(ui->purpose, SIGNAL(textChanged()), this, SIGNAL(onlineJobChanged()));
}

sepaCreditTransferEdit::~sepaCreditTransferEdit()
{
    delete ui;
}

onlineJobTyped<sepaOnlineTransfer> sepaCreditTransferEdit::getOnlineJobTyped() const
{
  onlineJobTyped<sepaOnlineTransfer> sepaJob( m_onlineJob );

  sepaJob.task()->setValue( ui->value->value() );
  sepaJob.task()->setPurpose( ui->purpose->toPlainText() );
  sepaJob.task()->setEndToEndReference( ui->sepaReference->text() );

  sepaAccountIdentifier accIdent;
  accIdent.setOwnerName( ui->beneficiaryName->text() );
  accIdent.setAccountNumber( ui->beneficiaryAccNum->text() );
  accIdent.setBankCode( ui->beneficiaryBankCode->text() );
  sepaJob.task()->setRecipient(accIdent);

  return sepaJob;
}

void sepaCreditTransferEdit::setOnlineJob(const onlineJobTyped<sepaOnlineTransfer>& job )
{
  m_onlineJob = job;
  ui->purpose->setText( job.task()->purpose() );
  ui->sepaReference->setText( job.task()->endToEndReference() );
  ui->value->setValue( job.task()->value() );
  ui->beneficiaryName->setText( job.task()->getRecipient().ownerName() );
  ui->beneficiaryAccNum->setText( job.task()->getRecipient().accountNumber() );
  ui->beneficiaryBankCode->setText( job.task()->getRecipient().bankCode() );
  updateSettings();
}

bool sepaCreditTransferEdit::setOnlineJob( const onlineJob& job )
{
  if( !job.isNull() && job.task()->taskHash() == sepaOnlineTransfer::hash ) {
    setOnlineJob( onlineJobTyped<sepaOnlineTransfer>(job) );
    return true;
  }
  return false;
}

void sepaCreditTransferEdit::setOriginAccount(const QString& accountId)
{
  m_onlineJob.task()->setOriginAccount( accountId );
  updateSettings();
}

void sepaCreditTransferEdit::updateEveryStatus()
{
    beneficiaryNameChanged( ui->beneficiaryName->text() );
    beneficiaryIbanChanged( ui->beneficiaryAccNum->text() );
    beneficiaryBicChanged( ui->beneficiaryBankCode->text() );
    purposeChanged();
    valueChanged();
    endToEndReferenceChanged( ui->sepaReference->text() );
}

void sepaCreditTransferEdit::updateSettings()
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
  // Reference
  ui->sepaReference->setMaxLength( settings->endToEndReferenceLength() );
  if (settings->endToEndReferenceLength() == 0)
    ui->sepaReference->setEnabled( false );
  else
    ui->sepaReference->setEnabled( true );

  // Purpose
  ui->purpose->setAllowedChars( settings->allowedChars() );
  ui->purpose->setMaxLineLength( settings->purposeLineLength() );
  ui->purpose->setMaxLines( settings->purposeMaxLines() );
  if (settings->purposeMinLength())
    m_requiredFields->add(ui->purpose);
  else
    m_requiredFields->remove(ui->purpose);
  
  // Beneficiary Name
  ui->beneficiaryName->setValidator( new QRegExpValidator(QRegExp( QString("[%1]*").arg(settings->allowedChars()) ), ui->beneficiaryName) );
  ui->beneficiaryName->setMaxLength( settings->recipientNameLineLength() );
  
  if (settings->recipientNameMinLength() != 0)
    m_requiredFields->add(ui->beneficiaryName);
  else
    m_requiredFields->remove(ui->beneficiaryName);
  
  updateEveryStatus();
}

void sepaCreditTransferEdit::beneficiaryIbanChanged(const QString& iban)
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
  if ( settings->isIbanValid( iban ) ) {
    ui->statusIban->setToolTip(QString());
    ui->statusIban->setVisible(false);
  } else {
    ui->statusIban->setToolTip( i18n("The checksum for this IBAN is invalid.") );
    ui->statusIban->setVisible(true);
  }
  
  if (settings->isBicMandatory( iban ))
    m_requiredFields->add( ui->beneficiaryBankCode );
  else
    m_requiredFields->remove( ui->beneficiaryBankCode );
}

void sepaCreditTransferEdit::beneficiaryNameChanged( const QString& name )
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
  if ( name.length() < settings->recipientNameMinLength() ) {
      ui->statusBeneficiaryName->setToolTip( i18np("A beneficiary name is needed.", "The beneficiary name must be at least %i characters long",
        settings->recipientNameMinLength()
      ) );
      ui->statusBeneficiaryName->setVisible( true );
  } else {
      ui->statusBeneficiaryName->setToolTip( QString() );
      ui->statusBeneficiaryName->setVisible( false );
  }
}

void sepaCreditTransferEdit::beneficiaryBicChanged( const QString& bic )
{
    if ( bic.length() == 8 || bic.length() == 11 ) {
        ui->statusBic->setToolTip( QString() );
        ui->statusBic->setVisible( false );
    } else {
        ui->statusBic->setToolTip( i18n("A valid BIC must contain 8 or 11 characters.") );
        ui->statusBic->setVisible( true );
    }
}

void sepaCreditTransferEdit::valueChanged()
{
    if ( !ui->value->isValid() || !ui->value->value().isPositive() ) {
        ui->statusAmount->setToolTip( i18n("A positiv amount to transfer is needed.") );
        ui->statusAmount->setColor( Qt::red );
        ui->statusAmount->setVisible( true );
        return;
    }
    
    const MyMoneyAccount account = getOnlineJob().responsibleMyMoneyAccount();
    const MyMoneyMoney expectedBalance = account.balance() - ui->value->value();
    
    if ( expectedBalance < MyMoneyMoney(  account.value("maxCreditAbsolute") ) ) {
        ui->statusAmount->setToolTip(i18n("After this credit transfer the account's balance will be below your credit limit."));
        ui->statusAmount->setColor( Qt::darkYellow );
        ui->statusAmount->setVisible( true );
    } else if ( expectedBalance < MyMoneyMoney( account.value("minBalanceAbsolute") )) {
        ui->statusAmount->setToolTip(i18n("After this credit transfer the account's balance will be below the minium balance."));
        ui->statusAmount->setColor( Qt::yellow );
        ui->statusAmount->setVisible( true );
    } else {
        ui->statusAmount->setToolTip(QString());
        ui->statusAmount->setVisible( false );
    }
}

void sepaCreditTransferEdit::endToEndReferenceChanged( const QString& reference )
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
    if ( settings->checkEndToEndReferenceLength( reference ) == sepaOnlineTransfer::settings::tooLong) {
        ui->statusReference->setToolTip( i18np("The end-to-end refence cannot contain more than one character.",
                                               "The end-to-end refence cannot contain more than %1 characters.",
                                               settings->endToEndReferenceLength()
                                              ) );
        ui->statusReference->setVisible(true);
    } else {
        ui->statusReference->setVisible(false);
    }
}

void sepaCreditTransferEdit::purposeChanged()
{
    const QString purpose = ui->purpose->toPlainText();
    QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();

    QString tooltip = QString("");
    if (!settings->checkPurposeLineLength( purpose ))
        tooltip = i18np("The maximal line length of %1 character per line is exceeded.", "The maximal line length of %1 characters per line is exceeded.",
                        settings->purposeLineLength())
        .append('\n');
    if (!settings->checkPurposeCharset( purpose ))
        tooltip.append( i18n("The purpose can only contain the letters A-Z, spaces and ':?,-()+ and /") ).append('\n');
    if ( !settings->checkPurposeMaxLines(purpose) ) {
        tooltip.append( i18np("In the purpose only a single line is allowed.", "The purpose cannot contain more than %1 lines.",
                              settings->purposeMaxLines()) )
        .append('\n');
    } else if (settings->checkPurposeLength(purpose) == sepaOnlineTransfer::settings::tooShort) {
        tooltip.append( i18np("A purpose is needed.", "The purpose must be at least %1 characters long.", settings->purposeMinLength()) );
    }

    // Set tooltip and remove the last '\n'
    tooltip.chop(1);
    ui->statusPurpose->setToolTip( tooltip );
    
    if (tooltip.isEmpty())
        ui->statusPurpose->setVisible( false );
    else
        ui->statusPurpose->setVisible( true );
}

QSharedPointer< const sepaOnlineTransfer::settings > sepaCreditTransferEdit::taskSettings()
{
  return getOnlineJobTyped().task()->getSettings();
}

QValidator::State ibanValidator::validate(QString& string, int&) const
{
    if( string.length() >= 1 ) {
        if( !string.at(0).isLetter() )
            return Invalid;
        if ( string.at(0).isLower() )
            string[0] = string.at(0).toUpper();
    }

    if ( string.length() >= 2 ) {
        if ( !string.at(1).isLetter() )
            return Invalid;
        if (string.at(1).isLower())
            string[1] = string.at(1).toUpper();
    }

    int digitCount = 0;
    for(int i = 2; i < string.length(); ++i) {
      if (string.at(i).isDigit()) {
          ++digitCount;
      } else if ( !string.at(i).isSpace() ) {
        return Invalid;
      }
    }

    if (digitCount > 32)
        return Invalid;

    if (digitCount > 2)
        return Acceptable;

    return Intermediate;
}

QValidator::State bicValidator::validate(QString &string, int&) const
{
  for (int i = 0; i < std::min(string.length(), 6); ++i) {
    if ( !string.at(i).isLetter() )
      return Invalid;
    if (string.at(i).isLower())
        string[i] = string.at(i).toUpper();
  }
  for (int i = 6; i < string.length(); ++i) {
    if ( !string.at(i).isLetterOrNumber() )
      return Invalid;
    if (string.at(i).isLower())
        string[i] = string.at(i).toUpper();
  }

  if ( string.length() > 11 )
    return Invalid;
  else if (string.length() == 8 || string.length() == 11)
    return Acceptable;

  return Intermediate;
}
