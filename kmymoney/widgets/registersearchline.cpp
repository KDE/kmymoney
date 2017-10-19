
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

#include <QLabel>
#include <QTimer>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QHash>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "register.h"
#include "registeritem.h"
#include "transaction.h"
#include "icons/icons.h"

using namespace KMyMoneyRegister;
using namespace Icons;

class RegisterSearchLine::RegisterSearchLinePrivate
{
public:
  RegisterSearchLinePrivate() :
      reg(0),
      combo(0),
      queuedSearches(0),
      status(RegisterFilter::Any) {}

  Register* reg;
  KComboBox* combo;
  QString search;
  int queuedSearches;
  RegisterFilter::ItemState status;
};

RegisterSearchLine::RegisterSearchLine(QWidget* parent, Register* reg) :
    KLineEdit(parent),
    d(new RegisterSearchLinePrivate)
{
  setClearButtonEnabled(true);
  init(reg);
}

void RegisterSearchLine::init(Register *reg)
{
  if (!parentWidget()->layout())
    parentWidget()->setLayout(new QHBoxLayout);
  parentWidget()->layout()->addWidget(this);
  d->reg = reg;
  connect(this, SIGNAL(textChanged(QString)), this, SLOT(queueSearch(QString)));

  QLabel* label = new QLabel(i18nc("label for status combo", "Stat&us"), parentWidget());
  parentWidget()->layout()->addWidget(label);
  d->combo = new KComboBox(parentWidget());
  parentWidget()->layout()->addWidget(d->combo);
  // don't change the order of the following lines unless updating
  // the case labels in RegisterSearchLine::itemMatches() at the same time
  d->combo->insertItem(RegisterFilter::Any, QIcon::fromTheme(g_Icons[Icon::SystemRun]), i18n("Any status"));
  d->combo->insertItem(RegisterFilter::Imported, QIcon::fromTheme(g_Icons[Icon::DocumentImport]), i18n("Imported"));
  d->combo->insertItem(RegisterFilter::Matched, KMyMoneyUtils::overlayIcon(g_Icons[Icon::ViewFinancialTransfer], g_Icons[Icon::DocumentImport]), i18n("Matched"));
  d->combo->insertItem(RegisterFilter::Erroneous, QIcon::fromTheme(g_Icons[Icon::TaskAttention]), i18n("Erroneous"));
  d->combo->insertItem(RegisterFilter::NotMarked, i18n("Not marked"));
  d->combo->insertItem(RegisterFilter::NotReconciled, i18n("Not reconciled"));
  d->combo->insertItem(RegisterFilter::Cleared, i18nc("Reconciliation state 'Cleared'", "Cleared"));
  d->combo->setCurrentIndex(RegisterFilter::Any);
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
  d->status = static_cast<RegisterFilter::ItemState>(status);
  updateSearch();
}

void RegisterSearchLine::queueSearch(const QString& search)
{
  d->queuedSearches++;
  d->search = search;
  QTimer::singleShot(200, this, SLOT(activateSearch()));
}

void RegisterSearchLine::activateSearch()
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

  RegisterFilter filter(d->search, d->status);
  RegisterItem* p = d->reg->firstItem();
  for (; p; p = p->nextItem()) {
    p->setVisible(p->matches(filter));
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

void RegisterSearchLine::reset()
{
  clear();
}

void RegisterSearchLine::itemAdded(RegisterItem* item) const
{
  item->setVisible(item->matches(RegisterFilter(text(), d->status)));
}

void RegisterSearchLine::registerDestroyed()
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
    QWidget(parent),
    d(new RegisterSearchLineWidgetPrivate)
{
  d->reg = reg;
  createWidgets();
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

void RegisterSearchLineWidget::createWidgets()
{
  QHBoxLayout *searchLineLayout = new QHBoxLayout(this);
  searchLineLayout->setSpacing(0);
  searchLineLayout->setContentsMargins(0, 0, 0, 0);
  QLabel *label = new QLabel(i18nc("Filter widget label", "Fi&lter:"), this);
  searchLineLayout->addWidget(label);

  d->searchLine = createSearchLine(d->reg);
  d->searchLine->show();

  label->setBuddy(d->searchLine);
  label->show();
}


RegisterSearchLine* RegisterSearchLineWidget::searchLine() const
{
  return d->searchLine;
}
