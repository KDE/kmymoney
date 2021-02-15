/***************************************************************************
                          kgncpricesourcedlg.h
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield <tonybloom@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KGNCPRICESOURCEDLG_H
#define KGNCPRICESOURCEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KGncPriceSourceDlgPrivate;
class KGncPriceSourceDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KGncPriceSourceDlg)

public:
  explicit KGncPriceSourceDlg(const QString &stockName, const QString &gncSource , QWidget * parent = nullptr);
  ~KGncPriceSourceDlg();

  QString selectedSource() const;
  bool alwaysUse() const;

public Q_SLOTS:
  void buttonPressed(int);
  void slotHelp();

private:
  KGncPriceSourceDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KGncPriceSourceDlg)
};

#endif
