/***************************************************************************
                          ktagreassigndlg.cpp
                             -------------------
    copyright            : (C) 2012 by Alessandro Russo <axela74@yahoo.it>
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

#ifndef KTAGREASSIGNDLG_H
#define KTAGREASSIGNDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KTagReassignDlg; }

/**
 *  Implementation of the dialog that lets the user select a tag in order
 *  to re-assign transactions (for instance, if tags are deleted).
 */

class MyMoneyTag;

class KTagReassignDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KTagReassignDlg)

public:
  explicit KTagReassignDlg(QWidget* parent = nullptr);
  ~KTagReassignDlg();

  /**
    * This function sets up the dialog, lets the user select a tag and returns
    * the id of the selected tag in the tagslist.
    *
    * @param tagslist reference to QList of MyMoneyTag objects to be contained in the list
    *
    * @return Returns the id of the selected tag in the list or QString() if
    *         the dialog was aborted. QString() is also returned if the tagslist is empty.
    */
  QString show(const QList<MyMoneyTag>& tagslist);

protected:
  void accept() override;

private:
  Ui::KTagReassignDlg *ui;
};

#endif // KTAGREASSIGNDLG_H
