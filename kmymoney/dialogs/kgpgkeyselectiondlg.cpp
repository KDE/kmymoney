/***************************************************************************
                          kgpgkeyselectiondlg.cpp
                             -------------------
    copyright            : (C) 2008 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

KGpgKeySelectionDlg::KGpgKeySelectionDlg(QWidget *parent) :
    QDialog(parent),
    m_needCheckList(true),
    m_listOk(false),
    m_checkCount(0)
{
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
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
  setModal(true);
  QWidget* page = new QWidget(this);
  mainLayout->addWidget(page);
  mainLayout->addWidget(buttonBox);

  QGroupBox *listBox = new QGroupBox(i18n("User identification"), page);
  QVBoxLayout *verticalLayout = new QVBoxLayout(listBox);
  verticalLayout->setSpacing(6);
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  m_listWidget = new KEditListWidget(listBox);
  m_listWidget->connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  m_listWidget->connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  m_listWidget->setWhatsThis(i18n("Enter the id of the key you want to use for data encryption. This can either be an e-mail address or the hexadecimal key id. In case of the key id, do not forget the leading 0x."));
  verticalLayout->addWidget(m_listWidget);

  // add a LED for the availability of all keys
  QHBoxLayout* ledBox = new QHBoxLayout();
  ledBox->setContentsMargins(0, 0, 0, 0);
  ledBox->setSpacing(6);
  ledBox->setObjectName("ledBoxLayout");

  m_keyLed = new KLed(page);
  mainLayout->addWidget(m_keyLed);
  m_keyLed->setShape(KLed::Circular);
  m_keyLed->setLook(KLed::Sunken);

  ledBox->addWidget(m_keyLed);
  ledBox->addWidget(new QLabel(i18n("Keys for all of the above user ids found"), page));
  ledBox->addItem(new QSpacerItem(50, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

  verticalLayout->addLayout(ledBox);

  connect(m_listWidget, SIGNAL(changed()), this, SLOT(slotIdChanged()));
  connect(m_listWidget, SIGNAL(added(QString)), this, SLOT(slotKeyListChanged()));
  connect(m_listWidget, SIGNAL(removed(QString)), this, SLOT(slotKeyListChanged()));
}

void KGpgKeySelectionDlg::setKeys(const QStringList& list)
{
  m_listWidget->clear();
  m_listWidget->insertStringList(list);
  slotKeyListChanged();
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
  m_needCheckList = true;
  slotIdChanged();
}

void KGpgKeySelectionDlg::slotIdChanged()
{
  // this looks a bit awkward. Here's why: KGPGFile::keyAvailable() starts
  // an external task and processes UI events while it waits for the external
  // process to finish. Thus, the first time we get here, the external process
  // is started and the user may press a second key which calls this routine
  // again.
  //
  // The second invocation is counted, but the check is not started until the
  // first one finishes. Once the external process finishes, we check if we
  // were called in the meantime and restart the check.
  if (++m_checkCount == 1) {
    while (1) {
      // first we check the current edit field if filled
      bool keysOk = true;
      if (!m_listWidget->currentText().isEmpty()) {
        keysOk = KGPGFile::keyAvailable(m_listWidget->currentText());
      }

      // if it is available, then scan the current list if we need to
      if (keysOk) {
        if (m_needCheckList) {
          QStringList keys = m_listWidget->items();
          QStringList::const_iterator it_s;
          for (it_s = keys.constBegin(); keysOk && it_s != keys.constEnd(); ++it_s) {
            if (!KGPGFile::keyAvailable(*it_s))
              keysOk = false;
          }
          m_listOk = keysOk;
          m_needCheckList = false;

        } else {
          keysOk = m_listOk;
        }
      }

      // did we receive some more requests to check?
      if (m_checkCount > 1) {
        m_checkCount = 1;
        continue;
      }

      m_keyLed->setState(static_cast<KLed::State>(keysOk && (m_listWidget->items().count() != 0) ? KLed::On : KLed::Off));
      // TODO: port to kf5
      // okButton->setEnabled((m_listWidget->items().count() == 0) || (m_keyLed->state() == KLed::On));
      break;
    }

    --m_checkCount;
  }
}
