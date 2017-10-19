/***************************************************************************
                          splitdelegate.h
                             -------------------
    begin                : Wed Apr 6 2016
    copyright            : (C) 2016 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SPLITDELEGATE_H
#define SPLITDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class SplitDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit SplitDelegate(QObject* parent = 0);
  virtual ~SplitDelegate();

  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;
  virtual void setEditorData(QWidget* editWidget, const QModelIndex& index) const;

  virtual void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /**
   * This method returns the row that currently has an editor
   * or -1 if no editor is open
   */
  virtual int editorRow() const;

  void setShowValuesInverted(bool inverse);
  bool showValuesInverted();

  static void setErroneousColor(const QColor& color);
  static void setImportedColor(const QColor& color);

  static QColor erroneousColor();

protected:
  bool eventFilter(QObject* o, QEvent* event);

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

