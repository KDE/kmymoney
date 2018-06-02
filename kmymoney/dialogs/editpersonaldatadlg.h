/*
 * Copyright 2000-2004  Michael Edwardes <mte@users.sourceforge.net>
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

#ifndef EDITPERSONALDATADLG_H
#define EDITPERSONALDATADLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

struct ContactData;

// This dialog lets the user create/edit a file.
// Use the second constructor to edit a file.
class EditPersonalDataDlgPrivate;
class EditPersonalDataDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(EditPersonalDataDlg)

public:
  explicit EditPersonalDataDlg(QWidget *parent, const QString& title);
  explicit EditPersonalDataDlg(QString userName,
                               QString userStreet,
                               QString userTown,
                               QString userCounty,
                               QString userPostcode,
                               QString userTelephone,
                               QString userEmail,
                               QWidget *parent,
                               const QString& title);
  ~EditPersonalDataDlg();

  QString userName() const;
  QString userStreet() const;
  QString userTown() const;
  QString userCountry() const;
  QString userPostcode() const;
  QString userTelephone() const;
  QString userEmail() const;

protected Q_SLOTS:
  void okClicked();
  void loadFromAddressBook();
  /** Gets the result of searching for the contact details of the current user */
  void slotContactFetched(const ContactData &identity);

private:
  EditPersonalDataDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(EditPersonalDataDlg)
};

#endif
