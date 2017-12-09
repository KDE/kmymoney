/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 * (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "konlinetransferform.h"
#include "ui_konlinetransferform.h"

#include <memory>

#include <QList>
#include <QDebug>
#include <QPluginLoader>
#include <QDateTime>

#include <KStandardAction>
#include <KLocalizedString>
#include <KPluginFactory>

#include "kguiutils.h"
#include "onlinetasks/interfaces/ui/ionlinejobedit.h"

#include "mymoneyfile.h"
#include "mymoney/onlinejobadministration.h"
#include "onlinejob.h"
#include "tasks/onlinetask.h"
#include "accountsmodel.h"

#include "models/models.h"
#include "icons/icons.h"
using namespace Icons;

kOnlineTransferForm::kOnlineTransferForm(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::kOnlineTransferForm),
    m_onlineJobEditWidgets(QList<IonlineJobEdit*>()),
    m_requiredFields(new KMandatoryFieldGroup(this))
{
  ui->setupUi(this);
  ui->unsupportedIcon->setPixmap(Icons::get(Icon::DialogInformation).pixmap(style()->pixelMetric(QStyle::PM_MessageBoxIconSize)));
  // The ui designer fills the QScrollArea with a QWidget. Remove it so we can simply check for .widget() == nullptr
  // if it contains a valid widget
  delete ui->creditTransferEdit->takeWidget();

  OnlineBankingAccountNamesFilterProxyModel* accountsModel = new OnlineBankingAccountNamesFilterProxyModel(this);
  auto const model = Models::instance()->accountsModel();
  accountsModel->setSourceModel(model);
  ui->originAccount->setModel(accountsModel);

  ui->convertMessage->hide();
  ui->convertMessage->setWordWrap(true);

  auto edits = onlineJobAdministration::instance()->onlineJobEdits();
  std::for_each(edits.constBegin(), edits.constEnd(), [this](onlineJobAdministration::onlineJobEditOffer in) {this->loadOnlineJobEditPlugin(in);});

  // Message Widget for read only jobs
  m_duplicateJob = KStandardAction::copy(this);
  connect(m_duplicateJob, &QAction::triggered, this, &kOnlineTransferForm::duplicateCurrentJob);

  ui->headMessage->hide();
  ui->headMessage->setWordWrap(true);
  ui->headMessage->setCloseButtonVisible(false);
  ui->headMessage->addAction(m_duplicateJob);

  connect(ui->transferTypeSelection, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &kOnlineTransferForm::convertCurrentJob);

  connect(ui->buttonAbort, &QAbstractButton::clicked, this, &kOnlineTransferForm::reject);
  connect(ui->buttonSend, &QAbstractButton::clicked, this, &kOnlineTransferForm::sendJob);
  connect(ui->buttonEnque, &QAbstractButton::clicked, this, &kOnlineTransferForm::accept);
  connect(m_requiredFields, static_cast<void (KMandatoryFieldGroup::*)(bool)>(&KMandatoryFieldGroup::stateChanged), ui->buttonEnque, &QPushButton::setEnabled);

  connect(ui->originAccount, &KMyMoneyAccountCombo::accountSelected, this, &kOnlineTransferForm::accountChanged);

  accountChanged();
  setJobReadOnly(false);
  m_requiredFields->add(ui->originAccount);
  m_requiredFields->setOkButton(ui->buttonSend);
}

void kOnlineTransferForm::loadOnlineJobEditPlugin(const onlineJobAdministration::onlineJobEditOffer& pluginDesc)
{
  try {
    std::unique_ptr<QPluginLoader> loader{new QPluginLoader(pluginDesc.fileName, this)};
    QObject* plugin = loader->instance();
    if (!plugin) {
      qWarning() << "Could not load plugin for online job editor from file \"" << pluginDesc.fileName << "\".";
      return;
    }

    // Cast to KPluginFactory
    KPluginFactory* pluginFactory = qobject_cast< KPluginFactory* >(plugin);
    if (!pluginFactory) {
      qWarning() << "Could not create plugin factory for online job editor in file \"" << pluginDesc.fileName << "\".";
      return;
    }

    IonlineJobEdit* widget = pluginFactory->create<IonlineJobEdit>(pluginDesc.pluginKeyword, this);
    if (!widget) {
      qWarning() << "Could not create online job editor in file \"" << pluginDesc.fileName << "\".";
      return;
    }

    // directly load the first widget into QScrollArea
    bool showWidget = true;
    if (!m_onlineJobEditWidgets.isEmpty()) {
      widget->setEnabled(false);
      showWidget = false;
    }

    m_onlineJobEditWidgets.append(widget);
    ui->transferTypeSelection->addItem(pluginDesc.name);
    m_requiredFields->add(widget);

    if (showWidget)
      showEditWidget(widget);
  } catch (MyMoneyException& e) {
    qWarning("Error while loading a plugin (IonlineJobEdit).");
  }
}

void kOnlineTransferForm::convertCurrentJob(const int& index)
{
  Q_ASSERT(index < m_onlineJobEditWidgets.count());

  IonlineJobEdit* widget = m_onlineJobEditWidgets.at(index);

  // Vars set by onlineJobAdministration::convertBest
  onlineTaskConverter::convertType convertType;
  QString userMessage;

  widget->setOnlineJob(onlineJobAdministration::instance()->convertBest(activeOnlineJob(), widget->supportedOnlineTasks(), convertType, userMessage));

  if (convertType == onlineTaskConverter::convertImpossible && userMessage.isEmpty())
    userMessage = i18n("During the change of the order your previous entries could not be converted.");

  if (!userMessage.isEmpty()) {
    switch (convertType) {
      case onlineTaskConverter::convertionLossyMajor:
        ui->convertMessage->setMessageType(KMessageWidget::Warning);
        break;
      case onlineTaskConverter::convertImpossible:
      case onlineTaskConverter::convertionLossyMinor:
        ui->convertMessage->setMessageType(KMessageWidget::Information);
        break;
      case onlineTaskConverter::convertionLoseless: break;
    }

    ui->convertMessage->setText(userMessage);
    ui->convertMessage->animatedShow();
  }

  showEditWidget(widget);
}

void kOnlineTransferForm::duplicateCurrentJob()
{
  IonlineJobEdit* widget = qobject_cast< IonlineJobEdit* >(ui->creditTransferEdit->widget());
  if (widget == 0)
    return;

  onlineJob duplicate(QString(), activeOnlineJob());
  widget->setOnlineJob(duplicate);
}

void kOnlineTransferForm::accept()
{
  emit acceptedForSave(activeOnlineJob());
  QDialog::accept();
}

void kOnlineTransferForm::sendJob()
{
  emit acceptedForSend(activeOnlineJob());
    QDialog::accept();
}

void kOnlineTransferForm::reject()
{
  QDialog::reject();
}

bool kOnlineTransferForm::setOnlineJob(const onlineJob job)
{
  QString name;
  try {
    name = job.task()->taskName();
  } catch (const onlineJob::emptyTask&) {
    return false;
  }

  setCurrentAccount(job.responsibleAccount());
  if (showEditWidget(name)) {
    IonlineJobEdit* widget = qobject_cast<IonlineJobEdit*>(ui->creditTransferEdit->widget());
    if (widget != 0) { // This can happen if there are no widgets
      const bool ret = widget->setOnlineJob(job);
      setJobReadOnly(!job.isEditable());
      return ret;
    }
  }
  return false;
}

void kOnlineTransferForm::accountChanged()
{
  const QString accountId = ui->originAccount->getSelected();
  try {
    ui->orderAccountBalance->setValue(MyMoneyFile::instance()->balance(accountId));
  } catch (const MyMoneyException&) {
    // @todo this can happen until the selection allows to select correct accounts only
    ui->orderAccountBalance->setText("");
  }

  foreach (IonlineJobEdit* widget, m_onlineJobEditWidgets)
  widget->setOriginAccount(accountId);

  checkNotSupportedWidget();
}

bool kOnlineTransferForm::checkEditWidget()
{
  return checkEditWidget(qobject_cast<IonlineJobEdit*>(ui->creditTransferEdit->widget()));
}

bool kOnlineTransferForm::checkEditWidget(IonlineJobEdit* widget)
{
  if (widget != 0 && onlineJobAdministration::instance()->isJobSupported(ui->originAccount->getSelected(), widget->supportedOnlineTasks())) {
    return true;
  }
  return false;
}

/** @todo auto set another widget if a loseless convert is possible */
void kOnlineTransferForm::checkNotSupportedWidget()
{
  if (!checkEditWidget()) {
    ui->displayStack->setCurrentIndex(0);
  } else {
    ui->displayStack->setCurrentIndex(1);
  }
}

void kOnlineTransferForm::setCurrentAccount(const QString& accountId)
{
  ui->originAccount->setSelected(accountId);
}

onlineJob kOnlineTransferForm::activeOnlineJob() const
{
  IonlineJobEdit* widget = qobject_cast<IonlineJobEdit*>(ui->creditTransferEdit->widget());
  if (widget == 0)
    return onlineJob();

  return widget->getOnlineJob();
}

void kOnlineTransferForm::setJobReadOnly(const bool& readOnly)
{
  ui->originAccount->setDisabled(readOnly);
  ui->transferTypeSelection->setDisabled(readOnly);

  if (readOnly) {
    ui->headMessage->setMessageType(KMessageWidget::Information);
    if (activeOnlineJob().sendDate().isValid())
      ui->headMessage->setText(i18n("This credit-transfer was sent to your bank at %1 therefore cannot be edited anymore. You may create a copy for editing.", activeOnlineJob().sendDate().toString(Qt::DefaultLocaleShortDate)));
    else
      ui->headMessage->setText(i18n("This credit-transfer is not editable. You may create a copy for editing."));

    if (this->isHidden())
      ui->headMessage->show();
    else
      ui->headMessage->animatedShow();
  } else {
    ui->headMessage->animatedHide();
  }
}

bool kOnlineTransferForm::showEditWidget(const QString& onlineTaskName)
{
  int index = 0;
  foreach (IonlineJobEdit* widget, m_onlineJobEditWidgets) {
    if (widget->supportedOnlineTasks().contains(onlineTaskName)) {
      ui->transferTypeSelection->setCurrentIndex(index);
      showEditWidget(widget);
      return true;
    }
    ++index;
  }
  return false;
}

void kOnlineTransferForm::showEditWidget(IonlineJobEdit* widget)
{
  Q_CHECK_PTR(widget);

  QWidget* oldWidget = ui->creditTransferEdit->takeWidget();
  if (oldWidget != 0) { // This is true at the first call of showEditWidget() and if there are no widgets.
    oldWidget->setEnabled(false);
    disconnect(qobject_cast<IonlineJobEdit*>(oldWidget), &IonlineJobEdit::readOnlyChanged, this, &kOnlineTransferForm::setJobReadOnly);
  }

  widget->setEnabled(true);
  ui->creditTransferEdit->setWidget(widget);
  setJobReadOnly(widget->isReadOnly());
  widget->show();

  connect(widget, &IonlineJobEdit::readOnlyChanged, this, &kOnlineTransferForm::setJobReadOnly);
  checkNotSupportedWidget();
  m_requiredFields->changed();
}

kOnlineTransferForm::~kOnlineTransferForm()
{
  ui->creditTransferEdit->takeWidget();
  qDeleteAll(m_onlineJobEditWidgets);

  delete ui;
  delete m_duplicateJob;
}
