/***************************************************************************
                          kmymoneycategory.cpp  -  description
                             -------------------
    begin                : Mon Jul 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneycategory.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPalette>
#include <QTimer>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "kmymoneyaccountcompletion.h"
#include "icons/icons.h"

using namespace Icons;

class KMyMoneyCategory::Private
{
public:
  Private() :
      splitButton(0),
      frame(0),
      recursive(false),
      isSplit(false) {}

  QPushButton*      splitButton;
  QFrame*           frame;
  bool              recursive;
  bool              isSplit;
};

KMyMoneyCategory::KMyMoneyCategory(QWidget* parent, bool splitButton) :
    KMyMoneyCombo(true, parent),
    d(new Private)
{
  if (splitButton) {
    d->frame = new QFrame(0);
    // don't change the following name unless you want to break TransactionEditor::setup()
    d->frame->setObjectName("KMyMoneyCategoryFrame");
    d->frame->setFocusProxy(this);
    QHBoxLayout* layout = new QHBoxLayout(d->frame);
    layout->setContentsMargins(0, 0, 0, 0);

    // make sure not to use our own overridden version of reparent() here
    KMyMoneyCombo::setParent(d->frame, windowFlags()  & ~Qt::WindowType_Mask);
    KMyMoneyCombo::show();
    if (parent) {
      d->frame->setParent(parent);
      d->frame->show();
    }

    // create button
    KGuiItem splitButtonItem(QString(),
                             QIcon::fromTheme(g_Icons[Icon::Split]), QString(), QString());
    d->splitButton = new QPushButton(d->frame);
    d->splitButton->setObjectName("splitButton");
    KGuiItem::assign(d->splitButton, splitButtonItem);

    layout->addWidget(this, 5);
    layout->addWidget(d->splitButton);

    installEventFilter(this);
  }

  m_completion = new kMyMoneyAccountCompletion(this);
  connect(m_completion, SIGNAL(itemSelected(QString)), this, SLOT(slotItemSelected(QString)));
  connect(this, SIGNAL(editTextChanged(QString)), m_completion, SLOT(slotMakeCompletion(QString)));
}

KMyMoneyCategory::~KMyMoneyCategory()
{
  // make sure to wipe out the frame, button and layout
  if (d->frame && !d->frame->parentWidget())
    d->frame->deleteLater();

  delete d;
}

QPushButton* KMyMoneyCategory::splitButton() const
{
  return d->splitButton;
}

void KMyMoneyCategory::setPalette(const QPalette& palette)
{
  if (d->frame)
    d->frame->setPalette(palette);
  KMyMoneyCombo::setPalette(palette);
}

void KMyMoneyCategory::reparent(QWidget *parent, Qt::WindowFlags w, const QPoint&, bool showIt)
{
  if (d->frame) {
    d->frame->setParent(parent, w);
    if (showIt)
      d->frame->show();
  } else {
    KMyMoneyCombo::setParent(parent, w);
    if (showIt)
      KMyMoneyCombo::show();
  }
}

kMyMoneyAccountSelector* KMyMoneyCategory::selector() const
{
  return dynamic_cast<kMyMoneyAccountSelector*>(KMyMoneyCombo::selector());
}

void KMyMoneyCategory::setCurrentTextById(const QString& id)
{
  if (!id.isEmpty()) {
    QString category = MyMoneyFile::instance()->accountToCategory(id);
    setCompletedText(category);
    setEditText(category);
  } else {
    setCompletedText(QString());
    clearEditText();
  }
  setSuppressObjectCreation(false);
}

void KMyMoneyCategory::slotItemSelected(const QString& id)
{
  setCurrentTextById(id);

  m_completion->hide();

  if (m_id != id) {
    m_id = id;
    emit itemSelected(id);
  }
}

void KMyMoneyCategory::focusOutEvent(QFocusEvent *ev)
{
  if (isSplitTransaction()) {
    KComboBox::focusOutEvent(ev);
  } else {
    KMyMoneyCombo::focusOutEvent(ev);
  }
}

void KMyMoneyCategory::focusInEvent(QFocusEvent *ev)
{
  KMyMoneyCombo::focusInEvent(ev);

  // make sure, we get a clean state before we automagically move the focus to
  // some other widget (like for 'split transaction'). We do this by delaying
  // the emission of the focusIn signal until the next run of the event loop.
  QTimer::singleShot(0, this, SIGNAL(focusIn()));
}

void KMyMoneyCategory::setSplitTransaction()
{
  d->isSplit = true;
  setEditText(i18nc("Split transaction (category replacement)", "Split transaction"));
  setSuppressObjectCreation(true);
}

bool KMyMoneyCategory::isSplitTransaction() const
{
  return d->isSplit;
}

bool KMyMoneyCategory::eventFilter(QObject *o, QEvent *ev)
{
  // forward enable/disable state to split button
  if (o == this && ev->type() == QEvent::EnabledChange) {
    if (d->splitButton) {
      d->splitButton->setEnabled(isEnabled());
    }
  }
  return KMyMoneyCombo::eventFilter(o, ev);
}

KMyMoneySecurity::KMyMoneySecurity(QWidget* parent) :
    KMyMoneyCategory(parent, false)
{
}

KMyMoneySecurity::~KMyMoneySecurity()
{
}

void KMyMoneySecurity::setCurrentTextById(const QString& id)
{
  if (!id.isEmpty()) {
    QString security = MyMoneyFile::instance()->account(id).name();
    setCompletedText(security);
    setEditText(security);
  } else {
    setCompletedText(QString());
    clearEditText();
  }
}
