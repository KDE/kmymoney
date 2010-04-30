/***************************************************************************
                          registersearchline.cpp
                             -------------------
    copyright            : (C) 2006 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "registersearchline.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QLabel>
#include <QToolButton>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktoolbar.h>
#include <kcombobox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <KIcon>

// ----------------------------------------------------------------------------
// Project Includes

using namespace KMyMoneyRegister;

class RegisterSearchLine::RegisterSearchLinePrivate
{
public:
  RegisterSearchLinePrivate() :
      reg(0),
      combo(0),
      queuedSearches(0),
      status(0) {}

  Register* reg;
  KComboBox* combo;
  QString search;
  int queuedSearches;
  int status;
};

RegisterSearchLine::RegisterSearchLine(QWidget* parent, Register* reg) :
    KLineEdit(parent),
    d(new RegisterSearchLinePrivate)
{
  setClearButtonShown(true);
  init(reg);
}

RegisterSearchLine::RegisterSearchLine(QWidget* parent) :
    KLineEdit(parent),
    d(new RegisterSearchLinePrivate)
{
  setClearButtonShown(true);
  init(0);
}

void RegisterSearchLine::init(Register *reg)
{
  d->reg = reg;
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(queueSearch(const QString&)));

  QLabel* label = new QLabel(i18nc("label for status combo", "Stat&us"), parentWidget());
  d->combo = new KComboBox(parentWidget());
  // don't change the order of the following lines unless updating
  // the case labels in RegisterSearchLine::itemMatches() at the same time
  d->combo->addItem(SmallIcon("system-run"), i18n("Any status"));
  d->combo->addItem(SmallIcon("document-import"), i18n("Imported"));
  d->combo->addItem(SmallIcon("process-stop"), i18n("Matched"));
  d->combo->addItem(SmallIcon("attention"), i18n("Erroneous"));
  d->combo->addItem(i18n("Not marked"));
  d->combo->addItem(i18n("Not reconciled"));
  d->combo->addItem(i18nc("Reconciliation state 'Cleared'", "Cleared"));
  d->combo->setCurrentIndex(0);
  connect(d->combo, SIGNAL(activated(int)), this, SLOT(slotStatusChanged(int)));
  connect(this, SIGNAL(clearButtonClicked()), this, SLOT(reset()));

  label->setBuddy(d->combo);

  if (reg) {
    connect(reg, SIGNAL(destroyed()), this, SLOT(registerDestroyed()));
    connect(reg, SIGNAL(itemAdded(RegisterItem*)), this, SLOT(itemAdded(RegisterItem*)));
  } else {
    setEnabled(false);
  }
}

RegisterSearchLine::~RegisterSearchLine()
{
  delete d;
}

void RegisterSearchLine::setRegister(Register* reg)
{
  if (d->reg) {
    disconnect(d->reg, SIGNAL(destroyed()), this, SLOT(registerDestroyed()));
    disconnect(d->reg, SIGNAL(itemAdded(RegisterItem*)), this, SLOT(itemAdded(RegisterItem*)));
  }

  d->reg = reg;

  if (reg) {
    connect(reg, SIGNAL(destroyed()), this, SLOT(registerDestroyed()));
    connect(reg, SIGNAL(itemAdded(RegisterItem*)), this, SLOT(itemAdded(RegisterItem*)));
  }

  setEnabled(reg != 0);
}

void RegisterSearchLine::slotStatusChanged(int status)
{
  d->status = status;
  updateSearch();
}

void RegisterSearchLine::queueSearch(const QString& search)
{
  d->queuedSearches++;
  d->search = search;
  QTimer::singleShot(200, this, SLOT(activateSearch()));
}

void RegisterSearchLine::activateSearch(void)
{
  --(d->queuedSearches);
  if (d->queuedSearches == 0)
    updateSearch(d->search);
}

void RegisterSearchLine::updateSearch(const QString& s)
{
  if (!d->reg)
    return;

  d->search = s.isNull() ? text() : s;

  // keep track of the current focus item
  RegisterItem* focusItem = d->reg->focusItem();

  bool enabled = d->reg->updatesEnabled();
  d->reg->setUpdatesEnabled(false);

  bool scrollBarVisible = d->reg->verticalScrollBar()->isVisible();

  RegisterItem* p = d->reg->firstItem();
  for (; p; p = p->nextItem()) {
    p->setVisible(itemMatches(p, d->search));
  }
  d->reg->suppressAdjacentMarkers();
  d->reg->updateAlternate();
  d->reg->setUpdatesEnabled(enabled);

  // if focus item is still visible, then make sure we have
  // it on screen
  if (focusItem && focusItem->isVisible()) {
    d->reg->update();
    d->reg->ensureItemVisible(focusItem);
  }
  // if the scrollbar's visibility changed, we need to resize the contents
  if (scrollBarVisible != d->reg->verticalScrollBar()->isVisible()) {
    d->reg->resize(DetailColumn);
  }
}

bool RegisterSearchLine::itemMatches(const RegisterItem* item, const QString& s) const
{
  const Transaction* t = dynamic_cast<const Transaction*>(item);
  if (t && !t->transaction().id().isEmpty()) {
    // Keep the case list of the following switch statement
    // in sync with the logic to fill the combo box in
    // RegisterSearchLine::init()
    switch (d->status) {
    default:
      break;
    case 1:    // Imported
      if (!t->transaction().isImported())
        return false;
      break;
    case 2:    // Matched
      if (!t->split().isMatched())
        return false;
      break;
    case 3:    // Erroneous
      if (t->transaction().splitSum().isZero())
        return false;
      break;
    case 4:    // Not marked
      if (t->split().reconcileFlag() != MyMoneySplit::NotReconciled)
        return false;
      break;
    case 5:    // Not reconciled
      if (t->split().reconcileFlag() != MyMoneySplit::NotReconciled
          && t->split().reconcileFlag() != MyMoneySplit::Cleared)
        return false;
      break;
    case 6:    // Cleared
      if (t->split().reconcileFlag() != MyMoneySplit::Cleared)
        return false;
      break;
    }
  }

  return item->matches(s);
}

void RegisterSearchLine::reset(void)
{
  clear();
  d->combo->setCurrentIndex(0);
  slotStatusChanged(0);
}

void RegisterSearchLine::itemAdded(RegisterItem* item) const
{
  item->setVisible(itemMatches(item, text()));
}

void RegisterSearchLine::registerDestroyed(void)
{
  d->reg = 0;
  setEnabled(false);
}


class RegisterSearchLineWidget::RegisterSearchLineWidgetPrivate
{
public:
  RegisterSearchLineWidgetPrivate() :
      reg(0),
      searchLine(0) {}

  Register* reg;
  RegisterSearchLine* searchLine;
};


RegisterSearchLineWidget::RegisterSearchLineWidget(Register* reg, QWidget* parent) :
    KHBox(parent),
    d(new RegisterSearchLineWidgetPrivate)
{
  d->reg = reg;
  setSpacing(5);
  QTimer::singleShot(0, this, SLOT(createWidgets()));
}

RegisterSearchLineWidget::~RegisterSearchLineWidget()
{
  delete d;
}

RegisterSearchLine* RegisterSearchLineWidget::createSearchLine(Register* reg)
{
  if (!d->searchLine)
    d->searchLine = new RegisterSearchLine(this, reg);
  return d->searchLine;
}

void RegisterSearchLineWidget::createWidgets(void)
{
  QLabel *label = new QLabel(i18nc("Search widget label", "S&earch:"), this);

  d->searchLine = createSearchLine(d->reg);
  d->searchLine->show();

  label->setBuddy(d->searchLine);
  label->show();
}


RegisterSearchLine* RegisterSearchLineWidget::searchLine(void) const
{
  return d->searchLine;
}

#include "registersearchline.moc"
