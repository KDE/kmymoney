#include "germancredittransferedit.h"
#include "germancredittransferedit_p.h"
#include "ui_germancredittransferedit.h"

germanCreditTransferEdit::germanCreditTransferEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::germanCreditTransferEdit),
    m_germanCreditTransfer( onlineJobKnownTask<germanOnlineTransfer>(new germanOnlineTransfer) )
{
    ui->setupUi(this);
    updateTaskSettings();

    ui->beneficiaryBankCode->setValidator( new QRegExpValidator(QRegExp("\\s*(\\d\\s*){8}"), ui->beneficiaryBankCode) );
    ui->beneficiaryAccNum->setValidator( new QRegExpValidator(QRegExp("\\s*(\\d\\s*){1,10}"), ui->beneficiaryAccNum) );

    connect(ui->beneficiaryBankCode, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryBankCodeChanged(QString)));
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
    m_originAccount = job.task()->responsibleAccount();
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

void germanCreditTransferEdit::beneficiaryNameChanged( QString name )
{
  Q_UNUSED(name);
}

void germanCreditTransferEdit::beneficiaryBankCodeChanged( QString bankCode )
{
  germanAccountIdentifier accountIdent;
  accountIdent.setBankCode( bankCode.remove(QRegExp("\\s")) );
  ui->beneficiaryBankName->setText( accountIdent.getBankName() );
}

void germanCreditTransferEdit::valueChanged( MyMoneyMoney value )
{
  Q_UNUSED(value);
}

void germanCreditTransferEdit::purposeChanged( QString purpose )
{
  Q_UNUSED(purpose);
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

void germanCreditTransferEdit::updateTaskSettings()
{
  QSharedPointer<const germanOnlineTransfer::settings> settings = getOnlineJob().task()->getSettings();
  ui->beneficiaryName->setMaxLength( settings->recipientNameLineLength() );
  ui->transferPurpose->setLineWrapColumnOrWidth( settings->purposeLineLength() );
}
