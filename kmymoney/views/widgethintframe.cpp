/*
 * Copyright 2015-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#include "widgethintframe.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QEvent>
#include <QMoveEvent>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class WidgetHintFrameCollection::Private
{
public:
  QList<QWidget*>           widgetList;
  QList<WidgetHintFrame*>   frameList;
};

WidgetHintFrameCollection::WidgetHintFrameCollection(QObject* parent)
  : QObject(parent)
  , d(new Private)
{
}

WidgetHintFrameCollection::~WidgetHintFrameCollection()
{
  delete d;
}

void WidgetHintFrameCollection::addFrame(WidgetHintFrame* frame)
{
  if(!d->frameList.contains(frame)) {
    connect(frame, &QObject::destroyed, this, &WidgetHintFrameCollection::frameDestroyed);
    connect(frame, &WidgetHintFrame::changed, this, [=] { QMetaObject::invokeMethod(this, "updateWidgets", Qt::QueuedConnection); });
    d->frameList.append(frame);
  }
}

void WidgetHintFrameCollection::addWidget(QWidget* w)
{
  if(!d->widgetList.contains(w)) {
    d->widgetList.append(w);
    updateWidgets();
  }
}

void WidgetHintFrameCollection::removeWidget(QWidget* w)
{
  d->widgetList.removeAll(w);
  w->setEnabled(true);
}

void WidgetHintFrameCollection::frameDestroyed(QObject* o)
{
  WidgetHintFrame* frame = qobject_cast< WidgetHintFrame* >(o);
  if(frame) {
    d->frameList.removeAll(frame);
  }
}

void WidgetHintFrameCollection::updateWidgets()
{
  bool enabled = true;
  Q_FOREACH(WidgetHintFrame* frame, d->frameList) {
    enabled &= !frame->isErroneous();
    if(!enabled) {
      break;
    }
  }

  Q_FOREACH(QWidget* w, d->widgetList) {
    w->setEnabled(enabled);
  }
}







class WidgetHintFrame::Private
{
public:
  QWidget*    editWidget;
  bool        status;
  FrameStyle  style;
};

WidgetHintFrame::WidgetHintFrame(QWidget* editWidget, FrameStyle style, Qt::WindowFlags f)
  : QFrame(editWidget->parentWidget(), f)
  , d(new Private)
{
  d->editWidget = 0;
  d->status = false;
  d->style = style;
  switch(style) {
    case Error:
      setStyleSheet("QFrame { background-color: none; padding: 1px; border: 2px solid red; border-radius: 4px; }");
      break;
    case Warning:
    case Info:
      setStyleSheet("QFrame { background-color: none; padding: 1px; border: 2px dashed red; border-radius: 4px; }");
      break;
  }
  attachToWidget(editWidget);
}

WidgetHintFrame::~WidgetHintFrame()
{
  delete d;
}

bool WidgetHintFrame::isErroneous() const
{
  return (d->style == Error) && (d->status == true);
}

static WidgetHintFrame* frame(QWidget* editWidget)
{
  QList<WidgetHintFrame*> allErrorFrames = editWidget->parentWidget()->findChildren<WidgetHintFrame*>();
  QList<WidgetHintFrame*>::const_iterator it;
  foreach(WidgetHintFrame* f, allErrorFrames) {
    if(f->editWidget() == editWidget) {
      return f;
    }
  }
  return 0;
}


void WidgetHintFrame::show(QWidget* editWidget, const QString& tooltip)
{
  WidgetHintFrame* f = frame(editWidget);
  if(f) {
    f->QWidget::show();
    f->d->status = true;
    emit f->changed();
  }
  if(!tooltip.isNull())
    editWidget->setToolTip(tooltip);
}

void WidgetHintFrame::hide(QWidget* editWidget, const QString& tooltip)
{
  WidgetHintFrame* f = frame(editWidget);
  if(f) {
    f->QWidget::hide();
    f->d->status = false;
    emit f->changed();
  }
  if(!tooltip.isNull())
    editWidget->setToolTip(tooltip);
}

QWidget* WidgetHintFrame::editWidget() const
{
  return d->editWidget;
}

void WidgetHintFrame::detachFromWidget()
{
  if(d->editWidget) {
    d->editWidget->removeEventFilter(this);
    d->editWidget = 0;
  }
}

void WidgetHintFrame::attachToWidget(QWidget* w)
{
  // detach first
  detachFromWidget();
  if(w) {
    d->editWidget = w;
    // make sure we receive changes in position and size
    w->installEventFilter(this);
    // place frame around widget
    move(w->pos() - QPoint(2, 2));
    resize(w->width()+4, w->height()+4);
    // make sure widget is on top of frame
    w->raise();
    // and hide frame for now
    QWidget::hide();
  }
}

bool WidgetHintFrame::eventFilter(QObject* o, QEvent* e)
{
  if(o == d->editWidget) {
    QMoveEvent* mev = 0;
    QResizeEvent* sev = 0;
    switch(e->type()) {
      case QEvent::EnabledChange:
      case QEvent::Hide:
      case QEvent::Show:
        /**
         * @todo think about what to do when widget is enabled/disabled
         * hidden or shown
         */
        break;

      case QEvent::Move:
        mev = static_cast<QMoveEvent*>(e);
        move(mev->pos() - QPoint(2, 2));
        break;

      case QEvent::Resize:
        sev = static_cast<QResizeEvent*>(e);
        resize(sev->size().width()+4, sev->size().height()+4);
        break;
      default:
        break;
    }
  }
  return QObject::eventFilter(o, e);
}
