/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
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

#ifndef IONLINEJOBEDIT_H
#define IONLINEJOBEDIT_H

#include <QWidget>
#include <QVariant>

#include "mymoney/onlinejob.h"

#include "mymoneymoney.h"

class MyMoneyMoney;

/**
 * @brief Interface for widgets editing onlineTasks
 *
 * @since 4.8.0
 */
class IonlineJobEdit : public QWidget
{
  Q_OBJECT

public:
  explicit IonlineJobEdit(QWidget* parent = 0, QVariantList args = QVariantList())
      : QWidget(parent) {
    Q_UNUSED(args);
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
  virtual bool isValid() const = 0;

  /**
   * @brief List of supported onlineTasks
   *
   * Returns a list of all task ids which can be edited with this
   * widget.
   */
  virtual QStringList supportedOnlineTasks() const = 0;

  /**
   * @brief Returns true if this widget is editable
   */
  virtual bool isReadOnly() const = 0;

public Q_SLOTS:
  /**
   * @brief Set an onlineJob to edit
   *
   * If the task is not compatible to the widget, return false. Do not throw
   * exceptions (and catch all of them).
   *
   * @return false if setting was not possible
   */
  virtual bool setOnlineJob(const onlineJob&) = 0;
  virtual void setOriginAccount(const QString&) = 0;
  virtual void showAllErrorMessages(const bool) {}

Q_SIGNALS:
  /**
   * @brief Emitted if a job which transfers money changed it's value
   */
  void transferValueChanged(MyMoneyMoney);

  /**
   * @brief Emitted if a job got valid or invalid
   *
   * @param valid status of onlineJob.isValid()
   */
  void validityChanged(bool valid);

  /**
   * @brief Emitted if widget was set or unset read only
   */
  void readOnlyChanged(bool);

};

Q_DECLARE_INTERFACE(IonlineJobEdit, "org.kmymoney.plugin.ionlinejobedit")

#endif // IONLINEJOBEDIT_H
