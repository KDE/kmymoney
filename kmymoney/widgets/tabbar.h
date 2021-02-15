/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
