/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
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

    void executeCustomAction(eView::Action action) override;
    void refresh();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject* o, QEvent* e) override;

public Q_SLOTS:
    /**
      * Print the current view
      */
    void slotPrintView();

private:
    Q_DECLARE_PRIVATE(KHomeView)

private Q_SLOTS:
    void slotOpenUrl(const QUrl &url);
    void slotAdjustScrollPos();
};

#endif
