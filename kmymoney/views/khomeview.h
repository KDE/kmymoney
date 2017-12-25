/***************************************************************************
                          khomeview.h  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
#ifndef KHOMEVIEW_H
#define KHOMEVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

/**
  * Displays a 'home page' for the user.  Similar to concepts used in
  * quicken and m$-money.
  *
  * @author Michael Edwardes
  *
  * @short A view containing the home page for kmymoney.
**/

class KHomeViewPrivate;
class KHomeView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KHomeView(QWidget *parent = nullptr);
  ~KHomeView() override;

  void refresh() override;

protected:
  void showEvent(QShowEvent* event) override;
  void wheelEvent(QWheelEvent *event) override;

public Q_SLOTS:
  /**
    * Print the current view
    */
  void slotPrintView();

Q_SIGNALS:
  void ledgerSelected(const QString& id, const QString& transaction);
  void objectSelected(const MyMoneyObject& obj);
  void openObjectRequested(const MyMoneyObject& obj);

private:
  Q_DECLARE_PRIVATE(KHomeView)

private Q_SLOTS:
  void slotOpenUrl(const QUrl &url);
};

#endif
