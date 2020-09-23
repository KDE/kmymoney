/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef TABBAR_H
#define TABBAR_H

#include "kmm_base_widgets_export.h"

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
  class KMM_BASE_WIDGETS_EXPORT TabBar : public QTabBar
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

  public Q_SLOTS:

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

  protected Q_SLOTS:
    void slotTabCurrentChanged(int id);

  Q_SIGNALS:
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
