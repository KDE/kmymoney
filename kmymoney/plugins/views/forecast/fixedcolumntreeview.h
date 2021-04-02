/*
    SPDX-FileCopyrightText: 2014 Cristian One ț <onet.cristian@gmail.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

#ifndef FIXEDCOLUMNTREEVIEW_H
#define FIXEDCOLUMNTREEVIEW_H

#include <QTreeView>

/**
  * This tree view should be used together with a source
  * tree view to obtain a view that has the first column
  * fixed just like in the "Frozen Column" Qt example.
  *
  * @usage Just create this view by passing the source view
  *        as a parameter. If you intend to change the model
  *        of the source view after this view was attached call
  *        sourceModelUpdated().
  */
class FixedColumnTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit FixedColumnTreeView(QTreeView *parent);
    ~FixedColumnTreeView();

public Q_SLOTS:
    /**
      * Call this if the model of the source view is changed
      * after the fixed column view was attached.
      */
    void sourceModelUpdated();

protected:
    bool viewportEvent(QEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

protected Q_SLOTS:
    void onExpanded(const QModelIndex& index);
    void onCollapsed(const QModelIndex& index);
    void updateSectionWidth(int logicalIndex, int, int newSize);
    void updateSortIndicator(int logicalIndex, Qt::SortOrder order);

private:
    struct Private;
    Private * const d;
};

#endif // FIXEDCOLUMNTREEVIEW_H
