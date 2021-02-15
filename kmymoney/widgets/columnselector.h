/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLUMNSELECTOR_H
#define COLUMNSELECTOR_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QTreeView>
#include <QTableView>
#include <QString>
#include <QPoint>
#include <QVector>

class ColumnSelectorPrivate;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
 * This class takes care of the selection of visible columns of a tree view and
 * their sizes and stores the selection in the global application configuration.
 *
 * The @a parent tree view must have a model attached that allows to
 * extract the maximum number of columns. The header names found in
 * the model are displayed in a menu when the user clicks on the
 * header with the right mouse button.
 *
 * @author Thomas Baumgart
 */
class KMM_BASE_WIDGETS_EXPORT ColumnSelector : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(ColumnSelector)

public:
  /**
   * Creates a ColumnSelector object
   *
   * @param view pointer to QTreeView object
   * @param configGroupName name of the configuration group in the rc file
   * @param offset offset to be added/subtracted during load/store operation for certain columns
   * @param columns QVector of column indeces (incl. offset) to which the @a offset should be applied
   *
   * @a offset and @a columns are only used for backward compatability and should not be used
   */
  explicit ColumnSelector(QTreeView* view, const QString& configGroupName = QString(), int offset = 0, const QVector<int>& columns = QVector<int>());

  /**
   * Creates a ColumnSelector object
   *
   * @param view pointer to QTableView object
   * @param configGroupName name of the configuration group in the rc file
   * @param offset offset to be added/subtracted during load/store operation for certain columns
   * @param columns QVector of column indeces (incl. offset) to which the @a offset should be applied
   *
   * @a offset and @a columns are only used for backward compatability and should not be used
   */
  explicit ColumnSelector(QTableView* view, const QString& configGroupName = QString(), int offset = 0, const QVector<int>& columns = QVector<int>());

  ~ColumnSelector();

  void setAlwaysHidden(QVector<int> columns);
  void setAlwaysVisible(QVector<int> columns);
  void setSelectable(QVector<int> columns);
  void setModel(QAbstractItemModel* model);
  const QAbstractItemModel* model() const;
  QVector<int> columns() const;

  /**
   * Set the offset to be subtracted from the actual column number
   * during load and store operations. This is used to maintain
   * backward compatability and should not be used on new code.
   * @a columns contains the indexes to which the @a offset should be applied.
   */
  void setColumnOffsetForStorage();

protected Q_SLOT:
  void slotColumnsMenu(const QPoint);
  void slotUpdateHeaderState();

private:
  ColumnSelectorPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(ColumnSelector)
};

#endif
