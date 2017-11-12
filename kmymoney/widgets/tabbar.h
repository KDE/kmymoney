/***************************************************************************
                             tabbar.h
                             ----------
    begin                : Sun May 14 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef TABBAR_H
#define TABBAR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTabBar>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace eWidgets { namespace eTabBar { enum class SignalEmission; } }

namespace KMyMoneyTransactionForm
{
  /**
  * @author Thomas Baumgart
  */
  class TabBarPrivate;
  class TabBar : public QTabBar
  {
    Q_OBJECT
    Q_DISABLE_COPY(TabBar)

  public:
    explicit TabBar(QWidget* parent = nullptr);
    ~TabBar();

    eWidgets::eTabBar::SignalEmission setSignalEmission(eWidgets::eTabBar::SignalEmission type);

    void copyTabs(const TabBar* otabbar);

    void insertTab(int id, const QString& title);
    void insertTab(int id);

    void setIdentifier(QWidget* tab, int newId);

    void setTabEnabled(int id, bool enabled);

    int currentIndex() const;

  public slots:

    /**
    * overridden for internal reasons, API not changed
    */
    virtual void setCurrentIndex(int id);

    /**
    * overridden for internal reasons, API not changed
    */
    void showEvent(QShowEvent* event) override;

  protected:
    void mousePressEvent(QMouseEvent* event) override;

  protected slots:
    void slotTabCurrentChanged(int id);

  signals:
    void tabCurrentChanged(int id);

  private:
    /**
    * returns the Qt index of tab at pos @a p or -1
    * Derived from QTabBarPrivate
    */
    int indexAtPos(const QPoint& p) const;

  private:
    TabBarPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(TabBar)
  };

} // namespace

#endif
