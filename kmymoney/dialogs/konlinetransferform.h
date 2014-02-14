/*
  This file is part of KMyMoney, A Personal Finance Manager for KDE
  Copyright (C) 2013 Christian Dávid <christian-david@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef KBTRANSFERFORM_H
#define KBTRANSFERFORM_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QtGui/QDialog>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/onlinejob.h"

class IonlineJobEdit;
class kMandatoryFieldGroup;

namespace Ui {
  class kOnlineTransferFormDecl;
}

/**
 * @brief The kOnlineTransferForm class
 * 
 * @todo Disable Send/Enque button if no task is shown.
 */
class kOnlineTransferForm : public QDialog
{
  Q_OBJECT

public:
  kOnlineTransferForm(QWidget *parent = 0);
  virtual ~kOnlineTransferForm();  

signals:
  /** @brief The user wants this job to be saved */
  void acceptedForSave( onlineJob );

  /** @brief User wants to send the onlineJob directly */
  void acceptedForSend( onlineJob );
  
public slots:  
  virtual void accept();
  virtual void reject();
    
  /** @brief sets the current origin account */
  virtual void setCurrentAccount( const QString& accountId );

  /**
   * @brief Sets an onlineTransfer to edit
   *
   * Convenient function if you do not know the type of
   * onlineTransfer.
   *
   * If possible you should use one of the other setOnlineJob()s.
   * @return true if setting was possible
   */
  virtual bool setOnlineJob( const onlineJob );

private slots:
  /** @brief Slot for account selection box */
  void accountChanged();

  /**
   * @brief Slot for changes of transfer type
   * @param index of KComboBox
   */
  void convertCurrentJob( const int& index );

  /** @brief Slot for send button */
  void sendJob();

  /**
   * @brief add a widget
   * 
   * Caller gives away ownership.
   */
  void addOnlineJobEditWidget( IonlineJobEdit* widget );

  /** @{ */
  /**
   * @brief Activates the onlineJobEdit widget
   */
  bool showEditWidget( const QString& onlineTaskName );
  void showEditWidget( IonlineJobEdit* widget );

  /** @} */
   
  /**
   * @brief Shows warning if checkEditWidget() == false
   */
  void checkNotSupportedWidget();

private:
  /**
   * @brief returns the currently edited onlineJob
   * Can be a null job
   */
  onlineJob activeOnlineJob() const;

  Ui::kOnlineTransferFormDecl* ui;
  QList<IonlineJobEdit*> m_onlineJobEditWidgets;
  kMandatoryFieldGroup* m_requiredFields;
  
  /**
   * @brief Checks if widget can edit any task the selected account supports
   */
  bool checkEditWidget( IonlineJobEdit* widget );
  /**
   * @brief Checks current widget
   * @see checkEditWidget( IonlineJobEdit* widget )
   */
  bool checkEditWidget();
  
  void editWidgetChanged();
};

#endif // KBTRANSFERFORM_H
