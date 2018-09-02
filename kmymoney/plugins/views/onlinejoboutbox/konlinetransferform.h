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


#ifndef KONLINETRANSFERFORM_H
#define KONLINETRANSFERFORM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/onlinejob.h"
#include "mymoney/onlinejobadministration.h"

class IonlineJobEdit;
class KMandatoryFieldGroup;

namespace Ui { class kOnlineTransferForm; }

/**
 * @brief The kOnlineTransferForm class
 *
 * @todo Disable Send/Enque button if no task is shown.
 * @todo If this dialog is shown a second time without setting a onlineJob it, it shows the previous content.
 *       Fix this by creating the IonlineJobEdit widgets on demand and destroying them afterwards.
 */
class kOnlineTransferForm : public QDialog
{
  Q_OBJECT

public:
  explicit kOnlineTransferForm(QWidget *parent = nullptr);
  virtual ~kOnlineTransferForm();

Q_SIGNALS:
  /** @brief The user wants this job to be saved */
  void acceptedForSave(onlineJob);

  /** @brief User wants to send the onlineJob directly */
  void acceptedForSend(onlineJob);

public Q_SLOTS:
  void accept() final override;
  void reject()final override;

  /** @brief sets the current origin account */
  virtual void setCurrentAccount(const QString& accountId);

  /**
   * @brief Sets an onlineTransfer to edit
   *
   * @return true if there is widget which supports editing this onlineJob
   */
  virtual bool setOnlineJob(const onlineJob);

  void duplicateCurrentJob();

private Q_SLOTS:
  /** @brief Slot for account selection box */
  void accountChanged();

  /**
   * @brief Slot to change job type
   * @param index of KComboBox (== index of selected widget in m_onlineJobEditWidgets)
   */
  void convertCurrentJob(const int& index);

  /** @brief Slot for send button */
  void sendJob();

  /**
   * @brief Load a plugin
   */
  void loadOnlineJobEditPlugin(const onlineJobAdministration::onlineJobEditOffer& plugin);

  /** @{ */
  /**
   * @brief Activates the onlineJobEdit widget
   */
  bool showEditWidget(const QString& onlineTaskName);
  void showEditWidget(IonlineJobEdit* widget);

  /** @} */

  /**
   * @brief Shows warning if checkEditWidget() == false
   */
  void checkNotSupportedWidget();

  void setJobReadOnly(const bool&);

private:
  /**
   * @brief returns the currently edited onlineJob
   * Can be a null job
   */
  onlineJob activeOnlineJob() const;

  Ui::kOnlineTransferForm* ui;
  QList<IonlineJobEdit*> m_onlineJobEditWidgets;
  KMandatoryFieldGroup* m_requiredFields;
  QAction* m_duplicateJob;

  /**
   * @brief Checks if widget can edit any task the selected account supports
   */
  bool checkEditWidget(IonlineJobEdit* widget);
  /**
   * @brief Checks current widget
   * @see checkEditWidget( IonlineJobEdit* widget )
   */
  bool checkEditWidget();

  void editWidgetChanged();
};

#endif // KONLINETRANSFERFORM_H
