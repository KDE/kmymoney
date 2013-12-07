#include "sepacredittransferedit.h"
#include "sepacredittransferedit_p.h"
#include "ui_sepacredittransferedit.h"

#include "mymoney/swiftaccountidentifier.h"

sepaCreditTransferEdit::sepaCreditTransferEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sepaCreditTransferEdit),
    m_onlineJob( onlineJobKnownTask<sepaOnlineTransfer>( new sepaOnlineTransfer ) )
{
    ui->setupUi(this);

    ui->beneficiaryBankCode->setValidator( new bicValidator(ui->beneficiaryBankCode) );
    ui->beneficiaryAccNum->setValidator( new ibanValidator(ui->beneficiaryAccNum) );

    //updateSettings();
}

sepaCreditTransferEdit::~sepaCreditTransferEdit()
{
    delete ui;
}

onlineJobKnownTask<sepaOnlineTransfer> sepaCreditTransferEdit::getOnlineJob() const
{
  onlineJobKnownTask<sepaOnlineTransfer> sepaJob( m_onlineJob );

  sepaJob.task()->setValue( ui->value->value() );
  sepaJob.task()->setPurpose( ui->purpose->toPlainText() );
  sepaJob.task()->setReference( ui->sepaReference->text() );

  sepaAccountIdentifier accIdent;
  accIdent.setOwnerName( ui->beneficiaryName->text() );
  accIdent.setAccountNumber( ui->beneficiaryAccNum->text() );
  accIdent.setBankCode( ui->beneficiaryBankCode->text() );
  sepaJob.task()->setRecipient(accIdent);

  return sepaJob;
}

void sepaCreditTransferEdit::setOnlineJob(const onlineJobKnownTask<sepaOnlineTransfer>& job )
{
  m_onlineJob = job;
  ui->purpose->setText( job.task()->purpose() );
  ui->sepaReference->setText( job.task()->reference() );
  ui->value->setValue( job.task()->value() );
  ui->beneficiaryName->setText( job.task()->getRecipient().ownerName() );
  ui->beneficiaryAccNum->setText( job.task()->getRecipient().accountNumber() );
  ui->beneficiaryBankCode->setText( job.task()->getRecipient().bankCode() );
  updateSettings();
}

void sepaCreditTransferEdit::setOnlineJob( const onlineJob& job )
{
  if( !job.isNull() && job.task()->taskHash() == sepaOnlineTransfer::hash ) {
    setOnlineJob( onlineJobKnownTask<sepaOnlineTransfer>(job) );
  }
}

void sepaCreditTransferEdit::setOriginAccount(const QString& accountId)
{
  m_onlineJob.task()->setOriginAccount( accountId );
  updateSettings();
}

void sepaCreditTransferEdit::updateSettings()
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = getOnlineJob().task()->getSettings();
  ui->beneficiaryName->setMaxLength( settings->recipientNameLength );
  ui->sepaReference->setMaxLength( settings->referenceLength );
  if (settings->referenceLength == 0)
    ui->sepaReference->setEnabled( false );
  else
    ui->sepaReference->setEnabled( true );
  
  ui->purpose->setLineWrapColumnOrWidth( settings->purposeLineLength );
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
