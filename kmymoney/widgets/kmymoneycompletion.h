/*
    SPDX-FileCopyrightText: 2004-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYCOMPLETION_H
#define KMYMONEYCOMPLETION_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QTreeWidgetItem;
class QTreeWidget;

class KMyMoneySelector;

/**
  * @author Thomas Baumgart
  */

class KMyMoneyCompletionPrivate;
class KMM_BASE_WIDGETS_EXPORT KMyMoneyCompletion : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KMyMoneyCompletion)

public:

    explicit  KMyMoneyCompletion(QWidget* parent = nullptr);
    virtual ~KMyMoneyCompletion();

    /**
      * Re-implemented for internal reasons.  API is unaffected.
      */
    virtual void hide();

    /**
      * This method sets the current account with id @p id as
      * the current selection.
      *
      * @param id id of account to be selected
      */
    void setSelected(const QString& id);

    KMyMoneySelector* selector() const;

public Q_SLOTS:
    void slotMakeCompletion(const QString& txt);

    void slotItemSelected(QTreeWidgetItem *item, int col);

protected:
    /**
      * Reimplemented from KMyMoneyAccountSelector to get events from the viewport (to hide
      * this widget on mouse-click, Escape-presses, etc.
      */
    bool eventFilter(QObject *, QEvent *) override;

    /**
      * Re-implemented for internal reasons.  API is unaffected.
      */
    void showEvent(QShowEvent*) override;

    /**
      * This method resizes the widget to show a maximum of @p count
      * or @a MAX_ITEMS items.
      *
      * @param count maximum number to be shown if < MAX_ITEMS
      */
    void adjustSize(const int count);

    /**
      * This method counts the number of items currently visible and
      * calls adjustSize(count).
      */
    void adjustSize();

    void connectSignals(QWidget *widget, QTreeWidget* lv);

    void show(bool presetSelected);

Q_SIGNALS:
    void itemSelected(const QString& id);

protected:
    KMyMoneyCompletionPrivate * const d_ptr;
    KMyMoneyCompletion(KMyMoneyCompletionPrivate &dd, QWidget* parent = nullptr);
    Q_DECLARE_PRIVATE(KMyMoneyCompletion)
};

#endif
