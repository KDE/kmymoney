/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPLITVIEW_H
#define SPLITVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTableView>
#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class MyMoneyAccount;
class MyMoneySecurity;

class SplitView : public QTableView
{
    Q_OBJECT
public:
    explicit SplitView(QWidget* parent = 0);
    virtual ~SplitView();

    /**
     * This method is used to modify the visibility of the
     * empty entry at the end of the ledger. The default
     * for the parameter @a show is @c true.
     */
    void setShowEntryForNewTransaction(bool show = true);

    void setSingleLineDetailRole(eMyMoney::Model::Roles role);

    /**
     * Returns true if the sign of the values displayed has
     * been inverted depending on the account type.
     */
    bool showValuesInverted() const;

    void setColumnsHidden(QVector<int> columns);
    void setColumnsShown(QVector<int> columns);

    void setModel(QAbstractItemModel * model) override;

    void setCommodity(const MyMoneySecurity& commodity);

    void selectMostRecentTransaction();

    void skipStartEditing();
    void blockEditorStart(bool blocked);

    void setTransactionPayeeId(const QString& id);

    void setReadOnlyMode(bool readOnly);

public Q_SLOTS:
    /**
     * This method scrolls the ledger so that the current item is visible
     */
    void ensureCurrentItemIsVisible();

    /**
     * Overridden for internal reasons. No change in base functionality
     */
    void edit(const QModelIndex& index) {
        QTableView::edit(index);
    }

    void slotSettingsChanged();

protected:
    bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) final override;
    void mousePressEvent(QMouseEvent* event) final override;
    void mouseMoveEvent(QMouseEvent* event) final override;
    void mouseDoubleClickEvent(QMouseEvent* event) final override;
    void wheelEvent(QWheelEvent *event) final override;
    void moveEvent(QMoveEvent *event) final override;
    void resizeEvent(QResizeEvent* event) final override;
    void paintEvent(QPaintEvent* event) final override;
    int sizeHintForRow(int row) const final override;
    int sizeHintForColumn(int row) const final override;

protected Q_SLOTS:
    void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) final override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) final override;

    virtual void adjustDetailColumn(int newViewportWidth);

Q_SIGNALS:
    void transactionSelected(const QModelIndex& idx);
    void aboutToStartEdit();
    void aboutToFinishEdit();
    void deleteSelectedSplits();

protected:
    class Private;
    Private * const d;
};
#endif // SPLITVIEW_H

