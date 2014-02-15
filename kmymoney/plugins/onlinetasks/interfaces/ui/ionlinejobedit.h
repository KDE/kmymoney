/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2014  Christian David <c.david@christian-david.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef IONLINEJOBEDIT_H
#define IONLINEJOBEDIT_H

#include <QWidget>

#include "mymoney/onlinejob.h"

/**
 * @brief Interface for widgets editing onlineTasks
 * 
 */
class IonlineJobEdit : public QWidget
{
  Q_OBJECT

public:
  explicit IonlineJobEdit(QWidget* parent = 0, Qt::WindowFlags f = 0)
    : QWidget(parent, f)
  {
    
  }
  
  /**
   * @brief Reads interface and creates an onlineJob
   * 
   * An null onlineJob can be returned.
   */
  virtual onlineJob getOnlineJob() const = 0;
  
  /**
   * @brief Checks if the user input would generate a valid onlineJob
   */
  virtual bool isValid() const { return getOnlineJob().isValid(); }
  
  /**
   * @brief List of supported onlineTasks
   * 
   * Returns a list of all onlineTask::name()s which can be edited with this
   * widget.
   */
  virtual QStringList supportedOnlineTasks() const = 0;
  
  /**
   * @brief a localized string presented to select this widget
   */
  virtual QString label() const = 0;
  
public slots:
  /**
   * @brief Set an onlineJob to edit
   * 
   * If the task is not compatible to the widget, return false. Do not throw
   * exceptions (and catch all of them).
   * 
   * @return false if setting was not possible
   */
  virtual bool setOnlineJob( const onlineJob& ) = 0;
  virtual void setOriginAccount( const QString& ) = 0;

signals:
  /**
   * @brief Emitted if a job which transfers money changed it's value
   * 
   */
  void transferValueChanged( MyMoneyMoney );
  
  /**
   * @brief Emitted if a job got valid or invalid
   * 
   * @param valid status of onlineJob.isValid()
   */
  void validityChanged( bool valid );
  
protected slots:
  /**
   * @brief Convenient slot to emit validityChanged()
   * 
   * A default implementation to emit validityChanged() based on getOnlineJob().isValid().
   * This is useful if you use @a kMandatoryFieldsGroup in your widget. Just connect kMandatoryFieldsGroup::stateChanged(bool)
   * to this slot.
   * 
   * @param status if false, validityChanged(false) is emitted without further checks.
   */
  void requiredFieldsCompleted( const bool& status = true )
  { 
    if ( status ) {
      emit validityChanged( getOnlineJob().isValid() );
    } else {
      emit validityChanged( false );
    }
  }

};

#endif // IONLINEJOBEDIT_H
