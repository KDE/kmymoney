#include "germancredittransferedit.h"
#include "germancredittransferedit_p.h"
#include "ui_germancredittransferedit.h"

#include <QDebug>

germanCreditTransferEdit::germanCreditTransferEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::germanCreditTransferEdit),
    m_germanCreditTransfer( onlineJobKnownTask<germanOnlineTransfer>(new germanOnlineTransfer) )
{
    ui->setupUi(this);
    updateTaskSettings();

    ui->beneficiaryBankCode->setValidator( new QRegExpValidator(QRegExp("\\s*(\\d\\s*){8}"), ui->beneficiaryBankCode) );
    ui->beneficiaryAccNum->setValidator( new QRegExpValidator(QRegExp("\\s*(\\d\\s*){1,10}"), ui->beneficiaryAccNum) );
    
    ui->statusBankName->setVisible(false);
    
    connect(ui->beneficiaryName, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryNameChanged(QString)));
    connect(ui->beneficiaryAccNum, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryAccountChanged(QString)));
    connect(ui->beneficiaryBankCode, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryBankCodeChanged(QString)));
    connect(ui->transferValue, SIGNAL(textChanged(QString)), this, SLOT(valueChanged()));
    connect(ui->transferPurpose, SIGNAL(textChanged()), this, SLOT(purposeChanged()));
}

germanCreditTransferEdit::~germanCreditTransferEdit()
{
    delete ui;
}

bool germanCreditTransferEdit::setOnlineJob(const onlineJob job)
{
    if (!job.isNull() && job.task()->taskHash() == germanOnlineTransfer::hash) {
        return setOnlineJob( onlineJobKnownTask<germanOnlineTransfer>(job) );
    }
    return false;
}

bool germanCreditTransferEdit::setOnlineJob(const onlineJobKnownTask<germanOnlineTransfer> job)
{
    m_germanCreditTransfer = job;
    setOriginAccount( job.task()->responsibleAccount() );
    ui->beneficiaryName->setText( m_germanCreditTransfer.task()->getRecipient().ownerName() );
    ui->beneficiaryAccNum->setText( m_germanCreditTransfer.task()->getRecipient().accountNumber() );
    ui->beneficiaryBankCode->setText( m_germanCreditTransfer.task()->getRecipient().bankCode() );
    ui->beneficiaryBankName->setText( m_germanCreditTransfer.task()->getRecipient().getBankName() );
    ui->transferValue->setValue( m_germanCreditTransfer.task()->value() );
    ui->transferPurpose->setText( m_germanCreditTransfer.task()->purpose() );
    return true;
}

void germanCreditTransferEdit::setOriginAccount( const QString& accountId )
{
  m_originAccount = accountId;
  updateTaskSettings();
}

void germanCreditTransferEdit::beneficiaryNameChanged( const QString& name )
{
    const creditTransferSettingsBase::lengthStatus status = getOnlineJob().task()->getSettings()->checkRecipientLength(name);
    if ( status == creditTransferSettingsBase::tooShort ) {
        ui->statusBeneficiaryName->setColor(Qt::red);
        ui->statusBeneficiaryName->setToolTip( i18n("A beneficiary name is needed.") );
    } else {
        ui->statusBeneficiaryName->setColor(Qt::green);
        ui->statusBeneficiaryName->setToolTip(QString());
    }
}

void germanCreditTransferEdit::beneficiaryAccountChanged(const QString& accountNumber)
{
    if (accountNumber.isEmpty()) {
        ui->statusAccountNumber->setColor( Qt::red );
        ui->statusAccountNumber->setToolTip( i18n("An account number is needed.") );
    } else {
        ui->statusAccountNumber->setColor( Qt::green );
        ui->statusAccountNumber->setToolTip(QString());
    }
}

void germanCreditTransferEdit::beneficiaryBankCodeChanged( QString bankCode )
{
  germanAccountIdentifier accountIdent;
  accountIdent.setBankCode( bankCode.remove(QRegExp("\\s")) );
  const QString bankName = accountIdent.getBankName();
  ui->beneficiaryBankName->setText( bankName );
  
  if ( bankCode.length() != 8 ) {
      ui->statusBankIdentifier->setToolTip(i18n("This bank identifier must be eigth digits long."));
      ui->statusBankIdentifier->setColor(Qt::red);
  } else if ( bankName.isEmpty() ) {
      ui->statusBankIdentifier->setToolTip(i18n("This bank identifier is unkown. Please re-check it."));
      ui->statusBankIdentifier->setColor(Qt::yellow);
  } else if (bankCode.length() == 8 ) {
      ui->statusBankIdentifier->setToolTip(QString());
      ui->statusBankIdentifier->setColor(Qt::green);
  }
}

void germanCreditTransferEdit::valueChanged()
{
    if ( !ui->transferValue->isValid() || !ui->transferValue->value().isPositive() ) {
        ui->statusAmount->setToolTip( i18n("A positiv amount to transfer is needed.") );
        ui->statusAmount->setColor( Qt::red );
        return;
    }
    
    const MyMoneyAccount account = getOnlineJob().responsibleMyMoneyAccount();
    const MyMoneyMoney expectedBalance = account.balance() - ui->transferValue->value();
    
    if ( expectedBalance < MyMoneyMoney(  account.value("maxCreditAbsolute") ) ) {
        ui->statusAmount->setToolTip(i18n("After this credit transfer the account's balance will be below your credit limit."));
        ui->statusAmount->setColor(Qt::darkYellow);
    } else if ( expectedBalance < MyMoneyMoney( account.value("minBalanceAbsolute") )) {
        ui->statusAmount->setToolTip(i18n("After this credit transfer the account's balance will be below the minium balance."));
        ui->statusAmount->setColor(Qt::yellow);
    } else {
        ui->statusAmount->setToolTip(QString());
        ui->statusAmount->setColor(Qt::green);
    }
}

void germanCreditTransferEdit::purposeChanged()
{
    const QString purpose = ui->transferPurpose->toPlainText();
    QSharedPointer<const germanOnlineTransfer::settings> settings = getOnlineJob().task()->getSettings();
    
    QString tooltip = QString("");
    if (!settings->checkPurposeLineLength( purpose ))
        tooltip = i18np("The maximal line length of %1 character per line is exceeded.", "The maximal line length of %1 characters per line is exceeded.",
                        settings->purposeLineLength())
        .append('\n');
    if (!settings->checkPurposeCharset( purpose ))
        tooltip.append( i18n("The purpose can only contain the letters A-Z, ä,ö,ü, spaces and .,&-+*%/$ or &.") ).append('\n');
    if ( !settings->checkPurposeMaxLines(purpose) )
        tooltip.append( i18np("In the purpose only a single line is allowed.", "The purpose cannot contain more than %1 lines.",
                              settings->purposeMaxLines()) )
        .append('\n');
    
    // Set tooltip and remove the last '\n'
    tooltip.chop(1);
    ui->statusPurpose->setToolTip( tooltip );
    
    if (tooltip.isEmpty())
        ui->statusPurpose->setColor( Qt::green );
    else
        ui->statusPurpose->setColor( Qt::red );
}

onlineJobKnownTask<germanOnlineTransfer> germanCreditTransferEdit::getOnlineJob() const
{
  onlineJobKnownTask<germanOnlineTransfer> job( m_germanCreditTransfer );
  germanAccountIdentifier accountIdent;
  accountIdent.setOwnerName( ui->beneficiaryName->text() );
  accountIdent.setAccountNumber( ui->beneficiaryAccNum->text() );
  accountIdent.setBankCode( ui->beneficiaryBankCode->text().remove(QRegExp("\\s")) );
  job.task()->setOriginAccount( m_originAccount );
  job.task()->setRecipient( accountIdent );
  job.task()->setValue( ui->transferValue->value() );
  job.task()->setPurpose( ui->transferPurpose->toPlainText() );
  return job;
}

void germanCreditTransferEdit::updateEveryStatus()
{
    beneficiaryBankCodeChanged(QString());
    valueChanged();
    purposeChanged();
}

void germanCreditTransferEdit::updateTaskSettings()
{
  QSharedPointer<const germanOnlineTransfer::settings> settings = getOnlineJob().task()->getSettings();
  ui->transferPurpose->setAllowedChars( settings->allowedChars() );
  ui->transferPurpose->setMaxLineLength( settings->purposeLineLength() );
  ui->transferPurpose->setMaxLines( settings->purposeMaxLines() );
  
  ui->beneficiaryName->setValidator( new QRegExpValidator(QRegExp( QString("[%1]*").arg(settings->allowedChars()) ), ui->beneficiaryName) );
  ui->beneficiaryName->setMaxLength( settings->recipientNameLineLength() );
  
  updateEveryStatus();
}
