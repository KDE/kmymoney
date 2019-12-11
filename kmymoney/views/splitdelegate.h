/*
 * Copyright 2016-2019  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef SPLITDELEGATE_H
#define SPLITDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySecurity;

class SplitDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit SplitDelegate(QObject* parent = 0);
  virtual ~SplitDelegate();

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const final override;
  void setEditorData(QWidget* editWidget, const QModelIndex& index) const final override;

  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

  /**
   * This method returns the row that currently has an editor
   * or -1 if no editor is open
   */
  virtual int editorRow() const;

  void setShowValuesInverted(bool inverse);
  bool showValuesInverted();

  void setCommodity(const MyMoneySecurity& commodity);

  static void setErroneousColor(const QColor& color);
  static void setImportedColor(const QColor& color);

  static QColor erroneousColor();

protected:
  bool eventFilter(QObject* o, QEvent* event) final override;

protected Q_SLOTS:
  void endEdit();

Q_SIGNALS:
  void sizeHintChanged(const QModelIndex&) const;

private:
  class Private;
  Private * const d;

  static QColor m_erroneousColor;
  static QColor m_importedColor;
};

#endif // SPLITDELEGATE_H

