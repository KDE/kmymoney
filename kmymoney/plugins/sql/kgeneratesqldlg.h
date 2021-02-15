/***************************************************************************
                          kgeneratesql.h
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield <tonybloom@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KGENERATESQLDLG_H
#define KGENERATESQLDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KGenerateSqlDlgPrivate;
class KGenerateSqlDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KGenerateSqlDlg)

public:
  explicit KGenerateSqlDlg(QWidget *parent = nullptr);
  ~KGenerateSqlDlg();
  /**
   * execute the generation
   */
  int exec() override;

public Q_SLOTS:
  void slotHelp();
  void slotdriverSelected();
  void slotcreateTables();
  void slotsaveSQL();

private:
  KGenerateSqlDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KGenerateSqlDlg)

};

#endif
