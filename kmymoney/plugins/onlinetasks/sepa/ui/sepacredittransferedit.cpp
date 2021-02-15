/*
    SPDX-FileCopyrightText: 2013 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sepacredittransferedit.h"
#include "ui_sepacredittransferedit.h"

#include <QCompleter>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include <KDescendantsProxyModel>

#include "kguiutils.h"

#include "mymoney/payeeidentifiermodel.h"
#include "onlinetasks/sepa/sepaonlinetransfer.h"
#include "widgets/payeeidentifier/ibanbic/ibanvalidator.h"
#include "widgets/payeeidentifier/ibanbic/bicvalidator.h"
#include "payeeidentifier/payeeidentifiertyped.h"
#include "misc/charvalidator.h"
#include "payeeidentifier/ibanbic/ibanbic.h"
#include "styleditemdelegateforwarder.h"
#include "widgets/payeeidentifier/ibanbic/ibanbicitemdelegate.h"
#include "onlinejobtyped.h"
#include "mymoneyaccount.h"
#include "widgetenums.h"

class ibanBicCompleterDelegate : public StyledItemDelegateForwarder
{
  Q_OBJECT

public:
  ibanBicCompleterDelegate(QObject *parent)
      : StyledItemDelegateForwarder(parent) {}

protected:
  QAbstractItemDelegate* getItemDelegate(const QModelIndex &index) const final override {
    static QPointer<QAbstractItemDelegate> defaultDelegate;
    static QPointer<QAbstractItemDelegate> ibanBicDelegate;

    const bool ibanBicRequested = index.model()->data(index, payeeIdentifierModel::isPayeeIdentifier).toBool();

    QAbstractItemDelegate* delegate = (ibanBicRequested)
                                      ? ibanBicDelegate
                                      : defaultDelegate;

    if (delegate == 0) {
      if (ibanBicRequested) {
        // Use this->parent() as parent because "this" is const
        ibanBicDelegate = new ibanBicItemDelegate(this->parent());
        delegate = ibanBicDelegate;
      } else {
        // Use this->parent() as parent because "this" is const
        defaultDelegate = new QStyledItemDelegate(this->parent());
        delegate = defaultDelegate;
      }
      connectSignals(delegate, Qt::UniqueConnection);
    }
    Q_CHECK_PTR(delegate);
    return delegate;
  }
};

class payeeIdentifierCompleterPopup : public QTreeView
{
  Q_OBJECT

public:
  payeeIdentifierCompleterPopup(QWidget* parent = 0)
      : QTreeView(parent) {
    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setAnimated(true);
    setHeaderHidden(true);
    setUniformRowHeights(false);
    expandAll();
  }
};

class ibanBicFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  enum roles {
    payeeIban = payeeIdentifierModel::payeeIdentifierUserRole, /**< electornic IBAN of payee */
  };

  ibanBicFilterProxyModel(QObject* parent = 0)
      : QSortFilterProxyModel(parent) {}

  QVariant data(const QModelIndex &index, int role) const final override {
    if (role == payeeIban) {
      if (!index.isValid())
        return QVariant();

      try {
        payeeIdentifierTyped<payeeIdentifiers::ibanBic> iban = payeeIdentifierTyped<payeeIdentifiers::ibanBic>(
              index.model()->data(index, payeeIdentifierModel::payeeIdentifier).value<payeeIdentifier>()
            );
        return iban->electronicIban();
      } catch (const payeeIdentifier::empty &) {
        return QVariant();
      } catch (const payeeIdentifier::badCast &) {
        return QVariant();
      }
    }

    return QSortFilterProxyModel::data(index, role);
  }

  bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const final override {
    if (!source_parent.isValid())
      return true;

    QModelIndex index = source_parent.model()->index(source_row, 0, source_parent);
    return (source_parent.model()->data(index, payeeIdentifierModel::payeeIdentifierType).toString() == payeeIdentifiers::ibanBic::staticPayeeIdentifierIid());
  }
};

class ibanBicCompleter : public QCompleter
{
  Q_OBJECT

public:
  ibanBicCompleter(QObject* parent = 0);

Q_SIGNALS:
  void activatedName(const QString& name) const;
  void highlightedName(const QString& name) const;

  void activatedBic(const QString& bic) const;
  void highlightedBic(const QString& bic) const;

  void activatedIban(const QString& iban) const;
  void highlightedIban(const QString& iban) const;

private Q_SLOTS:
  void slotActivated(const QModelIndex& index) const;
  void slotHighlighted(const QModelIndex& index) const;
};

ibanBicCompleter::ibanBicCompleter(QObject *parent)
    : QCompleter(parent)
{
  connect(this, SIGNAL(activated(QModelIndex)), SLOT(slotActivated(QModelIndex)));
  connect(this, SIGNAL(highlighted(QModelIndex)), SLOT(slotHighlighted(QModelIndex)));
}

void ibanBicCompleter::slotActivated(const QModelIndex &index) const
{
  if (!index.isValid())
    return;

  emit activatedName(index.model()->data(index, payeeIdentifierModel::payeeName).toString());
  try {
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> iban = payeeIdentifierTyped<payeeIdentifiers::ibanBic>(
          index.model()->data(index, payeeIdentifierModel::payeeIdentifier).value<payeeIdentifier>()
        );
    emit activatedIban(iban->electronicIban());
    emit activatedBic(iban->storedBic());
  } catch (const payeeIdentifier::empty &) {
  } catch (const payeeIdentifier::badCast &) {
  }
}

void ibanBicCompleter::slotHighlighted(const QModelIndex &index) const
{
  if (!index.isValid())
    return;

  emit highlightedName(index.model()->data(index, payeeIdentifierModel::payeeName).toString());
  try {
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> iban = payeeIdentifierTyped<payeeIdentifiers::ibanBic>(
          index.model()->data(index, payeeIdentifierModel::payeeIdentifier).value<payeeIdentifier>()
        );
    emit highlightedIban(iban->electronicIban());
    emit highlightedBic(iban->storedBic());
  } catch (const payeeIdentifier::empty &) {
  } catch (const payeeIdentifier::badCast &) {
  }
}

sepaCreditTransferEdit::sepaCreditTransferEdit(QWidget *parent, QVariantList args) :
    IonlineJobEdit(parent, args),
    ui(new Ui::sepaCreditTransferEdit),
    m_onlineJob(onlineJobTyped<sepaOnlineTransfer>()),
    m_requiredFields(new KMandatoryFieldGroup(this)),
    m_readOnly(false),
    m_showAllErrors(false)
{
  ui->setupUi(this);

  m_requiredFields->add(ui->beneficiaryIban);
  m_requiredFields->add(ui->value);
  // Other required fields are set in updateSettings()

  connect(m_requiredFields, SIGNAL(stateChanged(bool)), this, SLOT(requiredFieldsCompleted(bool)));

  connect(ui->beneficiaryName, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryNameChanged(QString)));
  connect(ui->beneficiaryIban, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryIbanChanged(QString)));
  connect(ui->beneficiaryBankCode, SIGNAL(textChanged(QString)), this, SLOT(beneficiaryBicChanged(QString)));
  connect(ui->value, SIGNAL(valueChanged(QString)), this, SLOT(valueChanged()));
  connect(ui->sepaReference, SIGNAL(textChanged(QString)), this, SLOT(endToEndReferenceChanged(QString)));
  connect(ui->purpose, SIGNAL(textChanged()), this, SLOT(purposeChanged()));

  connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(updateEveryStatus()));

  connect(ui->beneficiaryName, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
  connect(ui->beneficiaryIban, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
  connect(ui->beneficiaryBankCode, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
  connect(ui->value, SIGNAL(valueChanged(QString)), this, SIGNAL(onlineJobChanged()));
  connect(ui->sepaReference, SIGNAL(textChanged(QString)), this, SIGNAL(onlineJobChanged()));
  connect(ui->purpose, SIGNAL(textChanged()), this, SIGNAL(onlineJobChanged()));

  // Connect signals for read only
  connect(this, SIGNAL(readOnlyChanged(bool)), ui->beneficiaryName, SLOT(setReadOnly(bool)));
  connect(this, SIGNAL(readOnlyChanged(bool)), ui->beneficiaryIban, SLOT(setReadOnly(bool)));
  connect(this, SIGNAL(readOnlyChanged(bool)), ui->beneficiaryBankCode, SLOT(setReadOnly(bool)));
  connect(this, SIGNAL(readOnlyChanged(bool)), ui->value, SLOT(setReadOnly(bool)));
  connect(this, SIGNAL(readOnlyChanged(bool)), ui->sepaReference, SLOT(setReadOnly(bool)));
  connect(this, SIGNAL(readOnlyChanged(bool)), ui->purpose, SLOT(setReadOnly(bool)));

  // Create models for completers
  payeeIdentifierModel* identModel = new payeeIdentifierModel(this);
  identModel->setTypeFilter(payeeIdentifiers::ibanBic::staticPayeeIdentifierIid());

  ibanBicFilterProxyModel* filterModel = new ibanBicFilterProxyModel(this);
  filterModel->setSourceModel(identModel);

  KDescendantsProxyModel* descendantsModel = new KDescendantsProxyModel(this);
  descendantsModel->setSourceModel(filterModel);

  // Set completers popup and bind them to the corresponding fields
  {
    // Beneficiary name field
    ibanBicCompleter* completer = new ibanBicCompleter(this);
    completer->setModel(descendantsModel);
    completer->setCompletionRole(payeeIdentifierModel::payeeName);
    completer->setCaseSensitivity(Qt::CaseInsensitive);

    connect(completer, SIGNAL(activatedIban(QString)), ui->beneficiaryIban, SLOT(setText(QString)));
    connect(completer, SIGNAL(activatedBic(QString)), ui->beneficiaryBankCode, SLOT(setText(QString)));

    ui->beneficiaryName->setCompleter(completer);

    QAbstractItemView *itemView = new payeeIdentifierCompleterPopup();
    completer->setPopup(itemView);

    // setPopup() resets the delegate
    itemView->setItemDelegate(new ibanBicCompleterDelegate(this));
  }
  {
    // IBAN field
    ibanBicCompleter* ibanCompleter = new ibanBicCompleter(this);
    ibanCompleter->setModel(descendantsModel);
    ibanCompleter->setCompletionRole(ibanBicFilterProxyModel::payeeIban);
    ibanCompleter->setCaseSensitivity(Qt::CaseInsensitive);

    connect(ibanCompleter, SIGNAL(activatedName(QString)), ui->beneficiaryName, SLOT(setText(QString)));
    connect(ibanCompleter, SIGNAL(activatedBic(QString)), ui->beneficiaryBankCode, SLOT(setText(QString)));

    ui->beneficiaryIban->setCompleter(ibanCompleter);

    QAbstractItemView *itemView = new payeeIdentifierCompleterPopup();
    ibanCompleter->setPopup(itemView);

    // setPopup() resets the delegate
    itemView->setItemDelegate(new ibanBicCompleterDelegate(this));
  }
}

sepaCreditTransferEdit::~sepaCreditTransferEdit()
{
  delete ui;
}

void sepaCreditTransferEdit::showEvent(QShowEvent* event)
{
  updateEveryStatus();
  QWidget::showEvent(event);
}

void sepaCreditTransferEdit::showAllErrorMessages(const bool state)
{
  if (m_showAllErrors != state) {
    m_showAllErrors = state;
    updateEveryStatus();
  }
}

onlineJobTyped<sepaOnlineTransfer> sepaCreditTransferEdit::getOnlineJobTyped() const
{
  onlineJobTyped<sepaOnlineTransfer> sepaJob(m_onlineJob);

  sepaJob.task()->setValue(ui->value->value());
  sepaJob.task()->setPurpose(ui->purpose->toPlainText());
  sepaJob.task()->setEndToEndReference(ui->sepaReference->text());

  payeeIdentifiers::ibanBic accIdent;
  accIdent.setOwnerName(ui->beneficiaryName->text());
  accIdent.setIban(ui->beneficiaryIban->text());
  accIdent.setBic(ui->beneficiaryBankCode->text());
  sepaJob.task()->setBeneficiary(accIdent);

  return sepaJob;
}

void sepaCreditTransferEdit::setOnlineJob(const onlineJobTyped<sepaOnlineTransfer>& job)
{
  m_onlineJob = job;
  updateSettings();
  setReadOnly(!job.isEditable());

  ui->purpose->setText(job.task()->purpose());
  ui->sepaReference->setText(job.task()->endToEndReference());
  ui->value->setValue(job.task()->value());
  ui->beneficiaryName->setText(job.task()->beneficiaryTyped().ownerName());
  ui->beneficiaryIban->setText(job.task()->beneficiaryTyped().paperformatIban());
  ui->beneficiaryBankCode->setText(job.task()->beneficiaryTyped().storedBic());
}

bool sepaCreditTransferEdit::setOnlineJob(const onlineJob& job)
{
  if (!job.isNull() && job.task()->taskName() == sepaOnlineTransfer::name()) {
    setOnlineJob(onlineJobTyped<sepaOnlineTransfer>(job));
    return true;
  }
  return false;
}

void sepaCreditTransferEdit::setOriginAccount(const QString& accountId)
{
  m_onlineJob.task()->setOriginAccount(accountId);
  updateSettings();
}

void sepaCreditTransferEdit::updateEveryStatus()
{
  beneficiaryNameChanged(ui->beneficiaryName->text());
  beneficiaryIbanChanged(ui->beneficiaryIban->text());
  beneficiaryBicChanged(ui->beneficiaryBankCode->text());
  purposeChanged();
  valueChanged();
  endToEndReferenceChanged(ui->sepaReference->text());
}

void sepaCreditTransferEdit::setReadOnly(const bool& readOnly)
{
  // Only set writeable if it changes something and if it is possible
  if (readOnly != m_readOnly && (readOnly == true || getOnlineJobTyped().isEditable())) {
    m_readOnly = readOnly;
    emit readOnlyChanged(m_readOnly);
  }
}

void sepaCreditTransferEdit::updateSettings()
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
  // Reference
  ui->sepaReference->setMaxLength(settings->endToEndReferenceLength());
  if (settings->endToEndReferenceLength() == 0)
    ui->sepaReference->setEnabled(false);
  else
    ui->sepaReference->setEnabled(true);

  // Purpose
  ui->purpose->setAllowedChars(settings->allowedChars());
  ui->purpose->setMaxLineLength(settings->purposeLineLength());
  ui->purpose->setMaxLines(settings->purposeMaxLines());
  if (settings->purposeMinLength())
    m_requiredFields->add(ui->purpose);
  else
    m_requiredFields->remove(ui->purpose);

  // Beneficiary Name
  ui->beneficiaryName->setValidator(new charValidator(ui->beneficiaryName, settings->allowedChars()));
  ui->beneficiaryName->setMaxLength(settings->recipientNameLineLength());

  if (settings->recipientNameMinLength() != 0)
    m_requiredFields->add(ui->beneficiaryName);
  else
    m_requiredFields->remove(ui->beneficiaryName);

  updateEveryStatus();
}

void sepaCreditTransferEdit::beneficiaryIbanChanged(const QString& iban)
{
  // Check IBAN
  QPair<eWidgets::ValidationFeedback::MessageType, QString> answer = ibanValidator::validateWithMessage(iban);
  if (m_showAllErrors || iban.length() > 5 || (!ui->beneficiaryIban->hasFocus() && !iban.isEmpty()))
    ui->feedbackIban->setFeedback(answer.first, answer.second);
  else
    ui->feedbackIban->removeFeedback();

  // Check if BIC is mandatory
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();

  QString payeeIban;
  try {
    payeeIdentifier ident = getOnlineJobTyped().task()->originAccountIdentifier();
    payeeIban = ident.data<payeeIdentifiers::ibanBic>()->electronicIban();
  } catch (const payeeIdentifier::empty &) {
  } catch (const payeeIdentifier::badCast &) {
  }

  if (settings->isBicMandatory(payeeIban, iban)) {
    m_requiredFields->add(ui->beneficiaryBankCode);
    beneficiaryBicChanged(ui->beneficiaryBankCode->text());
  } else {
    m_requiredFields->remove(ui->beneficiaryBankCode);
    beneficiaryBicChanged(ui->beneficiaryBankCode->text());
  }
}

void sepaCreditTransferEdit::beneficiaryBicChanged(const QString& bic)
{
  if (bic.isEmpty() && !ui->beneficiaryIban->text().isEmpty()) {
    QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();

    const payeeIdentifier payee = getOnlineJobTyped().task()->originAccountIdentifier();
    QString iban;
    try {
      iban = payee.data<payeeIdentifiers::ibanBic>()->electronicIban();
    } catch (const payeeIdentifier::badCast &) {
    }

    if (settings->isBicMandatory(iban , ui->beneficiaryIban->text())) {
      ui->feedbackBic->setFeedback(eWidgets::ValidationFeedback::MessageType::Error, i18n("For this beneficiary's country the BIC is mandatory."));
      return;
    }
  }

  QPair<eWidgets::ValidationFeedback::MessageType, QString> answer = bicValidator::validateWithMessage(bic);
  if (m_showAllErrors || bic.length() >= 8 || (!ui->beneficiaryBankCode->hasFocus() && !bic.isEmpty()))
    ui->feedbackBic->setFeedback(answer.first, answer.second);
  else
    ui->feedbackBic->removeFeedback();
}

void sepaCreditTransferEdit::beneficiaryNameChanged(const QString& name)
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
  if (name.length() < settings->recipientNameMinLength() && (m_showAllErrors || (!ui->beneficiaryName->hasFocus() && !name.isEmpty()))) {
    ui->feedbackName->setFeedback(eWidgets::ValidationFeedback::MessageType::Error, i18np("A beneficiary name is needed.", "The beneficiary name must be at least %1 characters long",
                                  settings->recipientNameMinLength()
                                                                          ));
  } else {
    ui->feedbackName->removeFeedback();
  }
}

void sepaCreditTransferEdit::valueChanged()
{
  if ((!ui->value->isValid() && (m_showAllErrors || (!ui->value->hasFocus() && ui->value->value().toDouble() != 0))) || (!ui->value->value().isPositive() && ui->value->value().toDouble() != 0)) {
    ui->feedbackAmount->setFeedback(eWidgets::ValidationFeedback::MessageType::Error, i18n("A positive amount to transfer is needed."));
    return;
  }

  if (!ui->value->isValid())
    return;

  const MyMoneyAccount account = getOnlineJob().responsibleMyMoneyAccount();
  const MyMoneyMoney expectedBalance = account.balance() - ui->value->value();

  if (expectedBalance < MyMoneyMoney(account.value("maxCreditAbsolute"))) {
    ui->feedbackAmount->setFeedback(eWidgets::ValidationFeedback::MessageType::Warning, i18n("After this credit transfer the account's balance will be below your credit limit."));
  } else if (expectedBalance < MyMoneyMoney(account.value("minBalanceAbsolute"))) {
    ui->feedbackAmount->setFeedback(eWidgets::ValidationFeedback::MessageType::Information, i18n("After this credit transfer the account's balance will be below the minimal balance."));
  } else {
    ui->feedbackAmount->removeFeedback();
  }
}

void sepaCreditTransferEdit::endToEndReferenceChanged(const QString& reference)
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();
  if (settings->checkEndToEndReferenceLength(reference) == validators::tooLong) {
    ui->feedbackReference->setFeedback(eWidgets::ValidationFeedback::MessageType::Error, i18np("The end-to-end reference cannot contain more than one character.",
                                       "The end-to-end reference cannot contain more than %1 characters.",
                                       settings->endToEndReferenceLength()
                                                                               ));
  } else {
    ui->feedbackReference->removeFeedback();
  }
}

void sepaCreditTransferEdit::purposeChanged()
{
  const QString purpose = ui->purpose->toPlainText();
  QSharedPointer<const sepaOnlineTransfer::settings> settings = taskSettings();

  QString message;
  if (!settings->checkPurposeLineLength(purpose))
    message = i18np("The maximal line length of %1 character per line is exceeded.", "The maximal line length of %1 characters per line is exceeded.",
                    settings->purposeLineLength())
              .append('\n');
  if (!settings->checkPurposeCharset(purpose))
    message.append(i18n("The purpose can only contain the letters A-Z, spaces and ':?.,-()+ and /")).append('\n');
  if (!settings->checkPurposeMaxLines(purpose)) {
    message.append(i18np("In the purpose only a single line is allowed.", "The purpose cannot contain more than %1 lines.",
                         settings->purposeMaxLines()))
    .append('\n');
  } else if (settings->checkPurposeLength(purpose) == validators::tooShort) {
    message.append(i18np("A purpose is needed.", "The purpose must be at least %1 characters long.", settings->purposeMinLength()))
    .append('\n');
  }

  // Remove the last '\n'
  message.chop(1);

  if (!message.isEmpty()) {
    ui->feedbackPurpose->setFeedback(eWidgets::ValidationFeedback::MessageType::Error, message);
  } else {
    ui->feedbackPurpose->removeFeedback();
  }
}

QSharedPointer< const sepaOnlineTransfer::settings > sepaCreditTransferEdit::taskSettings()
{
  return getOnlineJobTyped().constTask()->getSettings();
}

#include "sepacredittransferedit.moc"
