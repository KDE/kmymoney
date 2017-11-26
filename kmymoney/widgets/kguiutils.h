/***************************************************************************
                         kguiutils.h  -  description
                            -------------------
   begin                : Fri Jan 27 2006
   copyright            : (C) 2006 Tony Bloomfield
   email                : Tony Bloomfield <tonybloom@users.sourceforge.net>
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

#ifndef KGUIUTILS_H
#define KGUIUTILS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_widgets_export.h"

class QWidget;
class QPushButton;

/**
  * @author Tony Bloomfield
  */
class KMandatoryFieldGroupPrivate;
class KMM_WIDGETS_EXPORT KMandatoryFieldGroup : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(KMandatoryFieldGroup)

public:
  explicit KMandatoryFieldGroup(QObject *parent);
  ~KMandatoryFieldGroup();

  /**
    * This method adds a widget to the list of mandatory fields for the current dialog
    *
    * @param widget pointer to the widget to be added
    */
  void add(QWidget *widget);

  /**
    * This method removes a widget form the list of mandatory fields for the current dialog
    *
    * @param widget pointer to the widget to be removed
    */
  void remove(QWidget *widget);

  /**
   * This method removes all widgets from the list of mandatory fields for the current dialog
   */
  void removeAll();

  /**
    * This method designates the button to be enabled when all mandatory fields
    * have been completed
    *
    * @param button pointer to the 'ok' button
    */
  void setOkButton(QPushButton *button);

  /**
    * This method returns if all requirements for the mandatory group
    * have been fulfilled (@p true) or not (@p false).
    */
  bool isEnabled() const;

public Q_SLOTS:
  void clear();

  /**
    * Force update of ok button
    */
  void changed();

Q_SIGNALS:
  void stateChanged();
  void stateChanged(bool state);

private:
  KMandatoryFieldGroupPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KMandatoryFieldGroup)
};

#endif // KGUIUTILS_H
