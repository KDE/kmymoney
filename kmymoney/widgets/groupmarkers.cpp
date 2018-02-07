/***************************************************************************
                             groupmarkers.cpp  -  description
                             -------------------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "groupmarkers.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractItemView>
#include <QList>
#include <QRect>
#include <QTableWidget>
#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "groupmarker.h"
#include "groupmarker_p.h"

#include "itemptrvector.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "widgetenums.h"

using namespace KMyMoneyRegister;
using namespace eWidgets;
using namespace eMyMoney;

namespace KMyMoneyRegister
{
  class TypeGroupMarkerPrivate : public GroupMarkerPrivate
  {
  public:
    eRegister::CashFlowDirection m_dir;
  };
}

TypeGroupMarker::TypeGroupMarker(KMyMoneyRegister::Register* parent, eRegister::CashFlowDirection dir, Account::Type accType) :
    GroupMarker(*new TypeGroupMarkerPrivate, parent, QString())
{
  Q_D(TypeGroupMarker);
  d->m_dir = dir;
  switch (dir) {
    case eRegister::CashFlowDirection::Deposit:
      d->m_txt = i18nc("Deposits onto account", "Deposits");
      if (accType == Account::Type::CreditCard) {
        d->m_txt = i18nc("Payments towards credit card", "Payments");
      }
      break;
    case eRegister::CashFlowDirection::Payment:
      d->m_txt = i18nc("Payments made from account", "Payments");
      if (accType == Account::Type::CreditCard) {
        d->m_txt = i18nc("Payments made with credit card", "Charges");
      }
      break;
    default:
      qDebug("Unknown CashFlowDirection %d for TypeGroupMarker constructor", (int)dir);
      break;
  }
}

TypeGroupMarker::~TypeGroupMarker()
{
}

eRegister::CashFlowDirection TypeGroupMarker::sortType() const
{
  Q_D(const TypeGroupMarker);
  return d->m_dir;
}

PayeeGroupMarker::PayeeGroupMarker(KMyMoneyRegister::Register* parent, const QString& name) :
    GroupMarker(parent, name)
{
}

PayeeGroupMarker::~PayeeGroupMarker()
{
}

const QString& PayeeGroupMarker::sortPayee() const
{
  Q_D(const GroupMarker);
  return d->m_txt;
}

CategoryGroupMarker::CategoryGroupMarker(KMyMoneyRegister::Register* parent, const QString& category) :
    GroupMarker(parent, category)
{
}

CategoryGroupMarker::~CategoryGroupMarker()
{
}

const QString& CategoryGroupMarker::sortCategory() const
{
  Q_D(const GroupMarker);
  return d->m_txt;
}
const QString CategoryGroupMarker::sortSecurity() const
{
  Q_D(const GroupMarker);
  return d->m_txt;
}

const char* CategoryGroupMarker::className()
{
  return "CategoryGroupMarker";
}

namespace KMyMoneyRegister
{
  class ReconcileGroupMarkerPrivate : public GroupMarkerPrivate
  {
  public:
    eMyMoney::Split::State m_state;
  };
}

ReconcileGroupMarker::ReconcileGroupMarker(KMyMoneyRegister::Register* parent, eMyMoney::Split::State state) :
    GroupMarker(*new ReconcileGroupMarkerPrivate, parent, QString())
{
  Q_D(ReconcileGroupMarker);
  d->m_state = state;
  switch (state) {
    case eMyMoney::Split::State::NotReconciled:
      d->m_txt = i18nc("Reconcile state 'Not reconciled'", "Not reconciled");
      break;
    case eMyMoney::Split::State::Cleared:
      d->m_txt = i18nc("Reconcile state 'Cleared'", "Cleared");
      break;
    case eMyMoney::Split::State::Reconciled:
      d->m_txt = i18nc("Reconcile state 'Reconciled'", "Reconciled");
      break;
    case eMyMoney::Split::State::Frozen:
      d->m_txt = i18nc("Reconcile state 'Frozen'", "Frozen");
      break;
    default:
      d->m_txt = i18nc("Unknown reconcile state", "Unknown");
      break;
  }
}

ReconcileGroupMarker::~ReconcileGroupMarker()
{
}

eMyMoney::Split::State ReconcileGroupMarker::sortReconcileState() const
{
  Q_D(const ReconcileGroupMarker);
  return d->m_state;
}

namespace KMyMoneyRegister
{
  class RegisterPrivate
  {
  public:
    RegisterPrivate() :
      m_selectAnchor(0),
      m_focusItem(0),
      m_firstItem(0),
      m_lastItem(0),
      m_firstErroneous(0),
      m_lastErroneous(0),
      m_rowHeightHint(0),
      m_ledgerLensForced(false),
      m_selectionMode(QTableWidget::MultiSelection),
      m_needResize(true),
      m_listsDirty(false),
      m_ignoreNextButtonRelease(false),
      m_needInitialColumnResize(false),
      m_usedWithEditor(false),
      m_mouseButton(Qt::MouseButtons(Qt::NoButton)),
      m_modifiers(Qt::KeyboardModifiers(Qt::NoModifier)),
      m_detailsColumnType(eRegister::DetailColumn::PayeeFirst)
    {
    }

    ~RegisterPrivate()
    {
    }

    ItemPtrVector                m_items;
    QVector<RegisterItem*>       m_itemIndex;
    RegisterItem*                m_selectAnchor;
    RegisterItem*                m_focusItem;
    RegisterItem*                m_ensureVisibleItem;
    RegisterItem*                m_firstItem;
    RegisterItem*                m_lastItem;
    RegisterItem*                m_firstErroneous;
    RegisterItem*                m_lastErroneous;

    int                          m_markErroneousTransactions;
    int                          m_rowHeightHint;

    MyMoneyAccount               m_account;

    bool                         m_ledgerLensForced;
    QAbstractItemView::SelectionMode m_selectionMode;

    bool                         m_needResize;
    bool                         m_listsDirty;
    bool                         m_ignoreNextButtonRelease;
    bool                         m_needInitialColumnResize;
    bool                         m_usedWithEditor;
    Qt::MouseButtons             m_mouseButton;
    Qt::KeyboardModifiers        m_modifiers;
    eTransaction::Column                       m_lastCol;
    QList<SortField>  m_sortOrder;
    QRect                        m_lastRepaintRect;
    eRegister::DetailColumn            m_detailsColumnType;

  };
}
