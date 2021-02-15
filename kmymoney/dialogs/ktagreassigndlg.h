/*
 * SPDX-FileCopyrightText: 2011-2012 Alessandro Russo <axela74@yahoo.it>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KTAGREASSIGNDLG_H
#define KTAGREASSIGNDLG_H

#include "kmm_base_dialogs_export.h"

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

class KMM_BASE_DIALOGS_EXPORT KTagReassignDlg : public QDialog
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
    * @param tagslist reference to QList of tag ids that are not available
    *                 for re-assignment
    *
    * @return Returns the id of the selected tag in the list or QString() if
    *         the dialog was aborted.
    */
  QString show(const QList<QString>& tagslist);

private:
  Ui::KTagReassignDlg *ui;
};

#endif // KTAGREASSIGNDLG_H
