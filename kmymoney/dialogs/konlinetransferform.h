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

#include "mymoney/onlinejobtyped.h"
#include "mymoney/germanonlinetransfer.h"
#include "mymoney/sepaonlinetransfer.h"

namespace Ui {
  class kOnlineTransferFormDecl;
}

/**
 * @brief The kOnlineTransferForm class
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
  /** @brief Show form for sepa credit-transfer */
  virtual void activateSepaTransfer( bool active = true );
  /** @brief Show form for german credit-transfer */
  virtual void activateGermanTransfer(bool active = true );

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
  virtual bool setOnlineJob(const onlineJobTyped<onlineTransfer> );

  /**
   * @brief Sets an germanOnlineTransfer to edit
   * @return true if setting was possible (here it is always true)
   */
  virtual bool setOnlineJob(const onlineJobTyped<germanOnlineTransfer>);

  /**
   * @brief Sets an germanOnlineTransfer to edit
   * @return true if setting was possible (here it is always true)
   */
  virtual bool setOnlineJob( const onlineJobTyped<sepaOnlineTransfer> );

private slots:
  /** @brief Slot for account selection box */
  void accountChanged();

  /** @brief Slot for send button */
  void sendJob();

  /** @brief Called when the onlineJob of an sub-widged was changed */
  void jobChanged();

private:
  size_t m_activeTransferType;

  /** @brief returns the active onlineJob */
  onlineJobTyped<onlineTransfer> activeOnlineJob() const;

  Ui::kOnlineTransferFormDecl* ui;

  inline QString originAccount() const;
  inline void setTransferWidget(const size_t &onlineTaskHash );
  
  /**
   * @brief Used to address widgets in QStackedWidgets
   */
  enum widgetPage {
    pageUnsupportedByAccount = 0,
    pageSepaCreditTransfer = 1,
    pageGermanCreditTransfer = 2
  };
};

#endif // KBTRANSFERFORM_H
