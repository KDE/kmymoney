/*
 * Copyright 2008-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kgpgkeyselectiondlg.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QPushButton>
#include <QDialogButtonBox>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kgpgfile.h>
#include <ui_kgpgkeyselectiondlg.h>

class KGpgKeySelectionDlgPrivate
{
  Q_DISABLE_COPY(KGpgKeySelectionDlgPrivate)

public:
  KGpgKeySelectionDlgPrivate()
  : ui(new Ui::KGpgKeySelectionDlg)
  , needCheckList(true)
  , listOk(false)
  , checkCount(0)
  {
  }

  ~KGpgKeySelectionDlgPrivate()
  {
    delete ui;
  }

  Ui::KGpgKeySelectionDlg*  ui;
  bool                      needCheckList;
  bool                      listOk;
  int                       checkCount;
};


KGpgKeySelectionDlg::KGpgKeySelectionDlg(QWidget *parent) :
    QDialog(parent),
    d_ptr(new KGpgKeySelectionDlgPrivate)
{
  Q_D(KGpgKeySelectionDlg);
  d->ui->setupUi(this);
  connect(d->ui->m_secretKey, SIGNAL(currentIndexChanged(int)), this, SLOT(slotIdChanged()));
  connect(d->ui->m_listWidget, &KEditListWidget::changed, this, &KGpgKeySelectionDlg::slotIdChanged);
  connect(d->ui->m_listWidget, &KEditListWidget::added, this, &KGpgKeySelectionDlg::slotKeyListChanged);
  connect(d->ui->m_listWidget, &KEditListWidget::removed, this, &KGpgKeySelectionDlg::slotKeyListChanged);
}

KGpgKeySelectionDlg::~KGpgKeySelectionDlg()
{
  Q_D(KGpgKeySelectionDlg);
  delete d;
}

void KGpgKeySelectionDlg::setSecretKeys(const QStringList& keyList, const QString& defaultKey)
{
  static constexpr char recoveryKeyId[] = "59B0F826D2B08440";

  Q_D(KGpgKeySelectionDlg);
  d->ui->m_secretKey->addItem(i18n("No encryption"));

  foreach(auto key, keyList) {
    QStringList fields = key.split(':', QString::SkipEmptyParts);
    if (fields[0] != recoveryKeyId) {
      // replace parenthesis in name field with brackets
      auto name = fields[1];
      name.replace('(', "[");
      name.replace(')', "]");
      name = QString("%1 (0x%2)").arg(name).arg(fields[0]);
      d->ui->m_secretKey->addItem(name);
      if (name.contains(defaultKey)) {
        d->ui->m_secretKey->setCurrentText(name);
      }
    }
  }
}

QString KGpgKeySelectionDlg::secretKey() const
{
  Q_D(const KGpgKeySelectionDlg);
  const bool enabled = (d->ui->m_secretKey->currentIndex() != 0);
  QString key;
  if (enabled) {
    key = d->ui->m_secretKey->currentText();
  }
  return key;
}

void KGpgKeySelectionDlg::setAdditionalKeys(const QStringList& list)
{
  Q_D(KGpgKeySelectionDlg);
  d->ui->m_listWidget->clear();
  d->ui->m_listWidget->insertStringList(list);
  slotKeyListChanged();
}

QStringList KGpgKeySelectionDlg::additionalKeys() const
{
  Q_D(const KGpgKeySelectionDlg);
  return d->ui->m_listWidget->items();
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
  d->needCheckList = true;
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
  if (++d->checkCount == 1) {
    const bool enabled = (d->ui->m_secretKey->currentIndex() != 0);
    d->ui->m_listWidget->setEnabled(enabled);
    d->ui->m_keyLed->setState(enabled ? KLed::On : KLed::Off);
    while (enabled) {
      // first we check the current edit field if filled
      bool keysOk = true;
      if (!d->ui->m_listWidget->currentText().isEmpty()) {
        keysOk = KGPGFile::keyAvailable(d->ui->m_listWidget->currentText());
      }

      // if it is available, then scan the current list if we need to
      if (keysOk) {
        if (d->needCheckList) {
          QStringList keys = d->ui->m_listWidget->items();
          QStringList::const_iterator it_s;
          for (it_s = keys.constBegin(); keysOk && it_s != keys.constEnd(); ++it_s) {
            if (!KGPGFile::keyAvailable(*it_s))
              keysOk = false;
          }
          d->listOk = keysOk;
          d->needCheckList = false;

        } else {
          keysOk = d->listOk;
        }
      }

      // did we receive some more requests to check?
      if (d->checkCount > 1) {
        d->checkCount = 1;
        continue;
      }

      if (!d->ui->m_listWidget->items().isEmpty())  {
        d->ui->m_keyLed->setState(static_cast<KLed::State>(keysOk ? KLed::On : KLed::Off));
      } else {
        d->ui->m_keyLed->setState(KLed::On);
      }
      break;
    }

    --d->checkCount;
    d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!enabled || (d->ui->m_keyLed->state() == KLed::On));
  }
}
