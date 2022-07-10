/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYVIEWBASE_H
#define KMYMONEYVIEWBASE_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
class QPoint;

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "viewenums.h"

class MyMoneyObject;
class KPageWidgetItem;
class SelectedObjects;
namespace eMenu {
enum class Action;
enum class Menu;
}
namespace KMyMoneyPlugin {
class OnlinePlugin;
}

/**
  * This class is an abstract base class that all specific views
  * should be based on.
  */
class KMyMoneyViewBasePrivate;
class KMM_WIDGETS_EXPORT KMyMoneyViewBase : public QWidget
{
    Q_OBJECT

public:
    virtual ~KMyMoneyViewBase();

    /**
     * Execute the @a action using the @a selections. This base
     * class implementation os a no-op. If a view needs to act upon
     * the execution of @a action you need to override this
     * method in the derived class of the view.
     */
    virtual void executeAction(eMenu::Action action, const SelectedObjects& selections);

    virtual void executeCustomAction(eView::Action) {}

    /**
     * This method is called during a view change on the view
     * that is left. In case you override it, make sure to
     * call the base class method as well.
     */
    virtual void aboutToHide();

    /**
     * This method is called during a view change on the view
     * that is entered. In case you override it, make sure to
     * call the base class method as well. The base class
     * implementation takes care of saving the last selected
     * view and informs the application about the current
     * selected objects in this view by emitting the
     * requestSelectionChange() signal.
     *
     * @sa requestSelectionChange()
     */
    virtual void aboutToShow();

    virtual QHash<eMenu::Action, QAction*> sharedToolbarActions();

    /**
     * Returns @c true if the view has a closable tab/sub-view.
     * Default is to return @c false.
     */
    virtual bool hasClosableView() const;

    /**
     * Closes the current selected closable tab/sub-view in the view.
     * Default is to do nothing.
     */
    virtual void closeCurrentView();

Q_SIGNALS:
    // these signals are send to application logic
    void requestSelectionChange(const SelectedObjects& selection) const;
    void requestCustomContextMenu(eMenu::Menu type, const QPoint& pos) const;
    void requestActionTrigger(eMenu::Action action);

    void viewStateChanged(bool enabled);

    void customActionRequested(View, eView::Action);

    void requestView(QWidget* viewWidget, const QString& accountId, const QString& journalEntryId);

public Q_SLOTS:
    virtual void updateActions(const SelectedObjects& selections) {
        Q_UNUSED(selections)
    }

    virtual void slotSettingsChanged() {}

    /**
     * Inform the view about available online plugins. The default
     * does not do anything
     */
    virtual void setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>* plugins)
    {
        Q_UNUSED(plugins)
    }

    virtual void setDefaultFocus();

protected:
    const QScopedPointer<KMyMoneyViewBasePrivate> d_ptr;

    // we do not allow to create objects of this class
    explicit KMyMoneyViewBase(QWidget* parent = nullptr);
    KMyMoneyViewBase(KMyMoneyViewBasePrivate &dd, QWidget *parent);

private:
    Q_DECLARE_PRIVATE(KMyMoneyViewBase)
};

#endif
