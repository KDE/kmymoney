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

// ----------------------------------------------------------------------------
// QT Includes

#include <qrect.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qlayout.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3Frame>
#include <QFocusEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycategory.h"
#include <kmymoney/mymoneyfile.h>
#include "kmymoneyaccountcompletion.h"

class KMyMoneyCategory::Private
{
public:
  Private() :
    splitButton(0),
    frame(0),
    recursive(false) {}

  KPushButton*      splitButton;
  Q3Frame*           frame;
  bool              recursive;
};

KMyMoneyCategory::KMyMoneyCategory(QWidget* parent, const char * name, bool splitButton) :
  KMyMoneyCombo(true, parent, name),
  d(new Private)
{
  if(splitButton) {
    d->frame = new Q3Frame(0);
    d->frame->setFocusProxy(this);
    Q3HBoxLayout* layout = new Q3HBoxLayout(d->frame);
    // make sure not to use our own overridden version of reparent() here
    KMyMoneyCombo::reparent(d->frame, getWFlags() & ~WType_Mask, QPoint(0, 0), true);
    if(parent)
      d->frame->reparent(parent, QPoint(0, 0), true);

    // create button
    KGuiItem splitButtonItem("",
        QIcon(KIconLoader::global()->loadIcon("split_transaction", KIcon::Small,
        KIconLoader::SizeSmall)), "", "");
    d->splitButton = new KPushButton( splitButtonItem, d->frame );
    d->splitButton->setObjectName( "splitButton" );

    layout->addWidget(this, 5);
    layout->addWidget(d->splitButton);
  }

  m_completion = new kMyMoneyAccountCompletion(this, 0);
  connect(m_completion, SIGNAL(itemSelected(const QString&)), this, SLOT(slotItemSelected(const QString&)));
  connect(this, SIGNAL(textChanged(const QString&)), m_completion, SLOT(slotMakeCompletion(const QString&)));
}

KMyMoneyCategory::~KMyMoneyCategory()
{
  // make sure to wipe out the frame, button and layout
  if(d->frame && !d->frame->parentWidget())
    d->frame->deleteLater();

  delete d;
}

KPushButton* KMyMoneyCategory::splitButton(void) const
{
  return d->splitButton;
}

void KMyMoneyCategory::setPalette(const QPalette& palette)
{
  if(d->frame)
    d->frame->setPalette(palette);
  KMyMoneyCombo::setPalette(palette);
}

void KMyMoneyCategory::reparent(QWidget *parent, Qt::WFlags w, const QPoint& pos, bool showIt)
{
  if(d->frame)
    d->frame->reparent(parent, w, pos, showIt);
  else
    KMyMoneyCombo::reparent(parent, w, pos, showIt);
}

kMyMoneyAccountSelector* KMyMoneyCategory::selector(void) const
{
  return dynamic_cast<kMyMoneyAccountSelector*>(KMyMoneyCombo::selector());
}

void KMyMoneyCategory::setCurrentTextById(const QString& id)
{
  if(!id.isEmpty())
    setCurrentText(MyMoneyFile::instance()->accountToCategory(id));
  else
    setCurrentText();
  setSuppressObjectCreation(false);
}

void KMyMoneyCategory::slotItemSelected(const QString& id)
{
  setCurrentTextById(id);

  m_completion->hide();

  if(m_id != id) {
    m_id = id;
    emit itemSelected(id);
  }
}

void KMyMoneyCategory::focusOutEvent(QFocusEvent *ev)
{
  if(isSplitTransaction()) {
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

void KMyMoneyCategory::setSplitTransaction(void)
{
  setCurrentText(i18n("Split transaction (category replacement)", "Split transaction"));
  setSuppressObjectCreation(true);
}

bool KMyMoneyCategory::isSplitTransaction(void) const
{
  return currentText() == i18n("Split transaction (category replacement)", "Split transaction");
}

void KMyMoneyCategory::setEnabled(bool enable)
{
  if(d->recursive || !d->frame) {
    KMyMoneyCombo::setEnabled(enable);

  } else if(d->frame) {
    d->recursive = true;
    d->frame->setEnabled(enable);
    d->recursive = false;
  }
}

void KMyMoneyCategory::setDisabled(bool disable)
{
  setEnabled(!disable);
}

KMyMoneySecurity::KMyMoneySecurity(QWidget* parent, const char * name) :
  KMyMoneyCategory(parent, name, false)
{
}

KMyMoneySecurity::~KMyMoneySecurity()
{
}

void KMyMoneySecurity::setCurrentTextById(const QString& id)
{
  if(!id.isEmpty())
    KMyMoneyCategory::setCurrentText(MyMoneyFile::instance()->account(id).name());
  else
    KMyMoneyCategory::setCurrentText();
}

#include "kmymoneycategory.moc"
