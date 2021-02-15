/*
 * SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "knewbankdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QTemporaryFile>
#include <QTimer>
#include <QDesktopServices>
#include <QRegularExpression>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLineEdit>
#include <kguiutils.h>
#include <KLocalizedString>
#include <KIO/Scheduler>
#include <KIO/Job>
#include <KIOGui/KIO/FavIconRequestJob>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewbankdlg.h"

#include "mymoneyinstitution.h"
#include "kmymoneyutils.h"
#include "icons.h"

#include <errno.h>

class KNewBankDlgPrivate
{
  Q_DISABLE_COPY(KNewBankDlgPrivate)

public:
  KNewBankDlgPrivate() :
    ui(new Ui::KNewBankDlg)
  {
    m_iconLoadTimer.setSingleShot(true);
  }

  ~KNewBankDlgPrivate()
  {
    delete ui;
  }

  Ui::KNewBankDlg*                  ui;
  MyMoneyInstitution                m_institution;
  QTimer                            m_iconLoadTimer;
  QPointer<KIO::FavIconRequestJob>  m_favIconJob;
  QIcon                             m_favIcon;
  QString                           m_iconName;
  QUrl                              m_url;
};

KNewBankDlg::KNewBankDlg(MyMoneyInstitution& institution, QWidget *parent) :
  QDialog(parent),
  d_ptr(new KNewBankDlgPrivate)
{
  Q_D(KNewBankDlg);
  d->ui->setupUi(this);
  d->m_institution = institution;
  setModal(true);

  d->ui->nameEdit->setFocus();
  d->ui->nameEdit->setText(institution.name());
  d->ui->cityEdit->setText(institution.city());
  d->ui->streetEdit->setText(institution.street());
  d->ui->postcodeEdit->setText(institution.postcode());
  d->ui->telephoneEdit->setText(institution.telephone());
  d->ui->sortCodeEdit->setText(institution.sortcode());
  d->ui->bicEdit->setText(institution.value(QStringLiteral("bic")));
  d->ui->urlEdit->setText(institution.value(QStringLiteral("url")));

  if (!institution.value(QStringLiteral("icon")).isEmpty()) {
    d->m_favIcon = Icons::loadIconFromApplicationCache(institution.value(QStringLiteral("icon")));
  }
  if (!d->m_favIcon.isNull()) {
    d->ui->iconButton->setEnabled(true);
    d->ui->iconButton->setIcon(d->m_favIcon);
  }

  d->ui->messageWidget->hide();

  connect(d->ui->buttonBox, &QDialogButtonBox::accepted, this, &KNewBankDlg::okClicked);
  connect(d->ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(d->ui->nameEdit, &QLineEdit::textChanged, this, &KNewBankDlg::institutionNameChanged);
  connect(d->ui->urlEdit, &QLineEdit::textChanged, this, &KNewBankDlg::slotUrlChanged);
  connect(&d->m_iconLoadTimer, &QTimer::timeout, this, &KNewBankDlg::slotLoadIcon);
  connect(d->ui->iconButton, &QToolButton::pressed, this,
          [=] {
            QUrl url;
            url.setUrl(QString::fromLatin1("https://%1").arg(d->ui->urlEdit->text()));
            QDesktopServices::openUrl(url);
          });

  institutionNameChanged(d->ui->nameEdit->text());
  slotUrlChanged(d->ui->urlEdit->text());

  auto requiredFields = new KMandatoryFieldGroup(this);
  requiredFields->setOkButton(d->ui->buttonBox->button(QDialogButtonBox::Ok)); // button to be enabled when all fields present
  requiredFields->add(d->ui->nameEdit);
}

void KNewBankDlg::institutionNameChanged(const QString &_text)
{
  Q_D(KNewBankDlg);
  d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!_text.isEmpty());
}

KNewBankDlg::~KNewBankDlg()
{
  Q_D(KNewBankDlg);
  delete d;
}

void KNewBankDlg::okClicked()
{
  Q_D(KNewBankDlg);
  if (d->ui->nameEdit->text().isEmpty()) {
    KMessageBox::information(this, i18n("The institution name field is empty.  Please enter the name."), i18n("Adding New Institution"));
    d->ui->nameEdit->setFocus();
    return;
  }

  d->m_institution.setName(d->ui->nameEdit->text());
  d->m_institution.setTown(d->ui->cityEdit->text());
  d->m_institution.setStreet(d->ui->streetEdit->text());
  d->m_institution.setPostcode(d->ui->postcodeEdit->text());
  d->m_institution.setTelephone(d->ui->telephoneEdit->text());
  d->m_institution.setSortcode(d->ui->sortCodeEdit->text());
  d->m_institution.setValue(QStringLiteral("bic"), d->ui->bicEdit->text());
  d->m_institution.setValue(QStringLiteral("url"), d->ui->urlEdit->text());
  d->m_institution.deletePair(QStringLiteral("icon"));

  if (d->ui->iconButton->isEnabled()) {
    d->m_institution.setValue(QStringLiteral("icon"), d->m_iconName);
    Icons::storeIconInApplicationCache(d->m_iconName, d->m_favIcon);
  }
  accept();
}

const MyMoneyInstitution& KNewBankDlg::institution()
{
  Q_D(KNewBankDlg);
  return d->m_institution;
}

void KNewBankDlg::newInstitution(MyMoneyInstitution& institution)
{
  institution.clearId();
  QPointer<KNewBankDlg> dlg = new KNewBankDlg(institution);
  if (dlg->exec() == QDialog::Accepted && dlg != 0) {
    institution = dlg->institution();
    KMyMoneyUtils::newInstitution(institution);
  }
  delete dlg;
}

void KNewBankDlg::slotUrlChanged(const QString& newUrl)
{
  Q_D(KNewBankDlg);

  // remove a possible leading protocol since we only provide https for now
  QRegularExpression protocol(QStringLiteral("^[a-zA-Z]+://(?<url>.*)"), QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatch matcher = protocol.match(newUrl);
  if (matcher.hasMatch()) {
    d->ui->urlEdit->setText(matcher.captured(QStringLiteral("url")));
    d->ui->messageWidget->setText(i18nc("@info:usagetip", "The protocol part has been removed by KMyMoney because it is fixed to https for security reasons."));
    d->ui->messageWidget->setMessageType(KMessageWidget::Information);
    d->ui->messageWidget->animatedShow();
  }
  d->m_iconLoadTimer.start(200);
}

void KNewBankDlg::slotLoadIcon()
{
  Q_D(KNewBankDlg);

  // if currently a check is running, retry later
  if (d->m_favIconJob) {
    d->m_iconLoadTimer.start(200);
    return;
  }

  const auto path = d->ui->urlEdit->text();
  QRegularExpression urlRe(QStringLiteral("^(.*\\.)?[^\\.]{2,}\\.[a-z]{2,}"), QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatch matcher = urlRe.match(path);
  d->ui->iconButton->setEnabled(false);

  if (matcher.hasMatch()) {
    d->ui->iconButton->setEnabled(true);
    d->m_url = QUrl(QString::fromLatin1("https://%1").arg(path));
    KIO::Scheduler::checkSlaveOnHold(true);
    d->m_favIconJob = new KIO::FavIconRequestJob(d->m_url);
    connect(d->m_favIconJob, &KIO::FavIconRequestJob::result, this, &KNewBankDlg::slotIconLoaded);
    // we force to end the job after 1 second to avoid blocking this mechanism in case the thing fails
    QTimer::singleShot(1000, this, &KNewBankDlg::killIconLoad);
  }
}

void KNewBankDlg::killIconLoad()
{
  Q_D(KNewBankDlg);
  if (d->m_favIconJob) {
    d->m_favIconJob->kill();
    d->m_favIconJob->deleteLater();
  }
}

void KNewBankDlg::slotIconLoaded(KJob* job)
{
  Q_D(KNewBankDlg);

  switch(job->error()) {
    case ECONNREFUSED:
      // There is an answer from the server, but no favicon. In case we
      // already have one, we keep it
      d->ui->iconButton->setEnabled(true);
      d->m_favIcon = Icons::get(Icons::Icon::Bank);
      d->m_iconName = QStringLiteral("enum:Bank");
      break;
    case 0:
      // There is an answer from the server, and the favicon is found
      d->ui->iconButton->setEnabled(true);
      d->m_favIcon = QIcon(dynamic_cast<KIO::FavIconRequestJob*>(job)->iconFile());
      d->m_iconName = QStringLiteral("favicon:%1").arg(d->m_url.host());
      break;
    default:
      // There is problem with the URL from
      qDebug() << "KIO::FavIconRequestJob error" << job->error();
      // intentional fall through

    case EALREADY:    // invalid URL, no server response
      d->ui->iconButton->setEnabled(false);
      d->m_favIcon = QIcon();
      d->m_iconName.clear();
      break;
  }
  d->ui->iconButton->setIcon(d->m_favIcon);
}
