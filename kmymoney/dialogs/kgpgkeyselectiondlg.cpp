/***************************************************************************
                          kgpgkeyselectiondlg.cpp
                             -------------------
    copyright            : (C) 2008 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kgpgkeyselectiondlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KEditListWidget>
#include <KLed>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include <kgpgfile.h>
#include <QDialogButtonBox>
#include <QPushButton>

class KGpgKeySelectionDlgPrivate
{
  Q_DISABLE_COPY(KGpgKeySelectionDlgPrivate)

public:
  KGpgKeySelectionDlgPrivate()
  {
  }

  ~KGpgKeySelectionDlgPrivate()
  {
  }

  KEditListWidget*   m_listWidget;
  KLed*           m_keyLed;
  bool            m_needCheckList;
  bool            m_listOk;
  int             m_checkCount;
};


KGpgKeySelectionDlg::KGpgKeySelectionDlg(QWidget *parent) :
    QDialog(parent),
    d_ptr(new KGpgKeySelectionDlgPrivate)
{
  Q_D(KGpgKeySelectionDlg);
  d->m_needCheckList = true;
  d->m_listOk = false;
  d->m_checkCount = 0;
  // TODO: check port to kf5
  setWindowTitle(i18n("Select additional keys"));
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QWidget *mainWidget = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  mainLayout->addWidget(mainWidget);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
  setModal(true);
  QWidget* page = new QWidget(this);
  mainLayout->addWidget(page);
  mainLayout->addWidget(buttonBox);

  QGroupBox *listBox = new QGroupBox(i18n("User identification"), page);
  QVBoxLayout *verticalLayout = new QVBoxLayout(listBox);
  verticalLayout->setSpacing(6);
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  d->m_listWidget = new KEditListWidget(listBox);
  d->m_listWidget->connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  d->m_listWidget->connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  d->m_listWidget->setWhatsThis(i18n("Enter the id of the key you want to use for data encryption. This can either be an e-mail address or the hexadecimal key id. In case of the key id, do not forget the leading 0x."));
  verticalLayout->addWidget(d->m_listWidget);

  // add a LED for the availability of all keys
  QHBoxLayout* ledBox = new QHBoxLayout();
  ledBox->setContentsMargins(0, 0, 0, 0);
  ledBox->setSpacing(6);
  ledBox->setObjectName("ledBoxLayout");

  d->m_keyLed = new KLed(page);
  mainLayout->addWidget(d->m_keyLed);
  d->m_keyLed->setShape(KLed::Circular);
  d->m_keyLed->setLook(KLed::Sunken);

  ledBox->addWidget(d->m_keyLed);
  ledBox->addWidget(new QLabel(i18n("Keys for all of the above user ids found"), page));
  ledBox->addItem(new QSpacerItem(50, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

  verticalLayout->addLayout(ledBox);

  connect(d->m_listWidget, &KEditListWidget::changed, this, &KGpgKeySelectionDlg::slotIdChanged);
  connect(d->m_listWidget, &KEditListWidget::added, this, &KGpgKeySelectionDlg::slotKeyListChanged);
  connect(d->m_listWidget, &KEditListWidget::removed, this, &KGpgKeySelectionDlg::slotKeyListChanged);
}

KGpgKeySelectionDlg::~KGpgKeySelectionDlg()
{
  Q_D(KGpgKeySelectionDlg);
  delete d;
}

void KGpgKeySelectionDlg::setKeys(const QStringList& list)
{
  Q_D(KGpgKeySelectionDlg);
  d->m_listWidget->clear();
  d->m_listWidget->insertStringList(list);
  slotKeyListChanged();
}

QStringList KGpgKeySelectionDlg::keys() const
{
  Q_D(const KGpgKeySelectionDlg);
  return d->m_listWidget->items();
}

#if 0
void KGpgKeySelectionDlg::slotShowHelp()
{
  QString anchor = m_helpAnchor[m_criteriaTab->currentPage()];
  if (anchor.isEmpty())
    anchor = QString("details.search");

  KHelpClient::invokeHelp(anchor);
}
#endif

void KGpgKeySelectionDlg::slotKeyListChanged()
{
  Q_D(KGpgKeySelectionDlg);
  d->m_needCheckList = true;
  slotIdChanged();
}

void KGpgKeySelectionDlg::slotIdChanged()
{
  Q_D(KGpgKeySelectionDlg);
  // this looks a bit awkward. Here's why: KGPGFile::keyAvailable() starts
  // an external task and processes UI events while it waits for the external
  // process to finish. Thus, the first time we get here, the external process
  // is started and the user may press a second key which calls this routine
  // again.
  //
  // The second invocation is counted, but the check is not started until the
  // first one finishes. Once the external process finishes, we check if we
  // were called in the meantime and restart the check.
  if (++d->m_checkCount == 1) {
    while (1) {
      // first we check the current edit field if filled
      bool keysOk = true;
      if (!d->m_listWidget->currentText().isEmpty()) {
        keysOk = KGPGFile::keyAvailable(d->m_listWidget->currentText());
      }

      // if it is available, then scan the current list if we need to
      if (keysOk) {
        if (d->m_needCheckList) {
          QStringList keys = d->m_listWidget->items();
          QStringList::const_iterator it_s;
          for (it_s = keys.constBegin(); keysOk && it_s != keys.constEnd(); ++it_s) {
            if (!KGPGFile::keyAvailable(*it_s))
              keysOk = false;
          }
          d->m_listOk = keysOk;
          d->m_needCheckList = false;

        } else {
          keysOk = d->m_listOk;
        }
      }

      // did we receive some more requests to check?
      if (d->m_checkCount > 1) {
        d->m_checkCount = 1;
        continue;
      }

      d->m_keyLed->setState(static_cast<KLed::State>(keysOk && (d->m_listWidget->items().count() != 0) ? KLed::On : KLed::Off));
      // TODO: port to kf5
      // okButton->setEnabled((m_listWidget->items().count() == 0) || (m_keyLed->state() == KLed::On));
      break;
    }

    --d->m_checkCount;
  }
}
