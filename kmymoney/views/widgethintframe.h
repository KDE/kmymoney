/*
    SPDX-FileCopyrightText: 2015-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WIDGETHINTFRAME_H
#define WIDGETHINTFRAME_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class WidgetHintFrame : public QFrame
{
  Q_OBJECT

public:
  enum FrameStyle {
    Error = 0,
    Warning,
    Info
  };
  Q_ENUM(FrameStyle)

  explicit WidgetHintFrame(QWidget* editWidget, FrameStyle style = Error, Qt::WindowFlags f = 0);
  ~WidgetHintFrame();

  void attachToWidget(QWidget* w);
  void detachFromWidget();

  bool isErroneous() const;

  QWidget* editWidget() const;

  /**
   * Shows the info frame around @a editWidget and in case @a tooltip
   * is not null (@sa QString::isNull()) the respective message will
   * be loaded into the @a editWidget's tooltip. In case @a tooltip is null
   * (the default) the @a editWidget's tooltip will not be changed.
   */
  static void show(QWidget* editWidget, const QString& tooltip = QString());

  /**
   * Hides the info frame around @a editWidget and in case @a tooltip
   * is not null (@sa QString::isNull()) the respective message will
   * be loaded into the @a editWidget's tooltip. In case @a tooltip is null
   * (the default) the @a editWidget's tooltip will not be changed.
   */
  static void hide(QWidget* editWidget, const QString& tooltip = QString());

protected:
  bool eventFilter(QObject* o, QEvent* e) final override;

Q_SIGNALS:
  void changed();

private:
  class Private;
  Private * const d;
};

class WidgetHintFrameCollection : public QObject
{
  Q_OBJECT
public:
  explicit WidgetHintFrameCollection(QObject* parent = 0);
  ~WidgetHintFrameCollection();

  void addFrame(WidgetHintFrame* frame);
  void addWidget(QWidget* w);
  void removeWidget(QWidget* w);

protected Q_SLOTS:
  virtual void frameDestroyed(QObject* o);
  virtual void updateWidgets();

Q_SIGNALS:
  void inputIsValid(bool valid);

private:
  class Private;
  Private * const d;
};

#endif // WIDGETHINTFRAME_H

