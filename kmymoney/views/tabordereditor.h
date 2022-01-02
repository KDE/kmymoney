/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TABORDEREDITOR_H
#define TABORDEREDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

class QPaintEvent;
#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class TabOrderEditorInterface
{
public:
    virtual ~TabOrderEditorInterface() = default;
    virtual void setupUi(QWidget* parent) = 0;
    virtual void storeTabOrder(const QStringList& tabOrder) = 0;
};

class TabOrderDialogPrivate;
class TabOrderEditorPrivate;
class TabOrderDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TabOrderDialog)
    Q_DISABLE_COPY(TabOrderDialog)
    friend class TabOrderEditor;
    friend class TabOrderEditorPrivate;

public:
    explicit TabOrderDialog(QWidget* parent);
    ~TabOrderDialog();

    /**
     * The widget that contains the edit widgets that
     * we want to set the tab order for.
     *
     * @param targetWidget Pointer to the widget containing the edit widgets
     *                     for which the tab order needs to be set.
     *
     * @note The widgets themselves are identified by the property
     *        @c kmm_taborder being @c true. Clears the default tab order.
     */
    void setTarget(TabOrderEditorInterface* targetWidget);

    /**
     * Presets the tab order according to the names
     * of the widgets contained in @a widgetNames.
     * If called prior to setTarget() it returns immediately.
     *
     * @param widgetNames QStringList of widget names
     *
     * @note If @a widgetNames contains names for which no widget
     *       is found in @c targetWidget or the property @c kmm_taborder is
     *       not set or false then the entry will be dropped
     *       without notice. It will be missing when calling tabOrder().
     *
     * @sa tabOrder(), setTarget()
     */
    void setTabOrder(const QStringList& widgetNames);

    /**
     * Loads the default tab order of the widget. Is used if
     * the user wishes to return to the default.
     *
     * @param widgetNames QStringList of widget names
     *
     * @note If @a widgetNames contains names for which no widget
     *       is found in @c targetWidget or the property @c kmm_taborder is
     *       not set or false then the entry will be dropped
     *       without notice. It will be missing when calling tabOrder().
     *
     * @sa tabOrder(), setTarget()
     */
    void setDefaultTabOrder(const QStringList& widgetNames);

    /**
     * Returns a list of widget names sorted in the tab order.
     * If called prior to setTarget() it returns an empty list.
     * If called prior to setDefaultTabOrder() or setTabOrder()
     * the order is random.
     *
     * @returns QStringList of widget names.
     */
    QStringList tabOrder() const;

    int exec() override;

private:
    TabOrderDialogPrivate* d_ptr;
};

class TabOrderEditor : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TabOrderEditor)
    Q_DISABLE_COPY(TabOrderEditor)
    friend class TabOrderDialog;
    friend class TabOrderDialogPrivate;

public:
    explicit TabOrderEditor(TabOrderDialog* parent);
    ~TabOrderEditor();

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* e) override;
    bool eventFilter(QObject* o, QEvent* e) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

    const QFontMetrics& indicatorFontMetrics() const;

Q_SIGNALS:
    void geometryUpdated();

private:
    TabOrderEditorPrivate* d_ptr;
};

#endif // TABORDEREDITOR_H
