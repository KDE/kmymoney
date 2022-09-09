/*
    SPDX-FileCopyrightText: 2006-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneycategory.h"
#include "kmymoneycombo_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPalette>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QIcon>
#include <QEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountselector.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "kmymoneyaccountcompletion.h"
#include "icons/icons.h"

using namespace Icons;

class KMyMoneyCategoryPrivate : public KMyMoneyComboPrivate
{
public:
    KMyMoneyCategoryPrivate() :
        splitButton(0),
        frame(0),
        recursive(false),
        isSplit(false)
    {
    }

    QPushButton*      splitButton;
    QFrame*           frame;
    bool              recursive;
    bool              isSplit;
};

KMyMoneyCategory::KMyMoneyCategory(bool splitButton, QWidget* parent) :
    KMyMoneyCombo(*new KMyMoneyCategoryPrivate, true, parent)
{
    Q_D(KMyMoneyCategory);
    if (splitButton) {
        d->frame = new QFrame(nullptr);
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
                                 Icons::get(Icon::Split), QString(), QString());
        d->splitButton = new QPushButton(d->frame);
        d->splitButton->setObjectName("splitButton");
        KGuiItem::assign(d->splitButton, splitButtonItem);

        layout->addWidget(this, 5);
        layout->addWidget(d->splitButton);

        installEventFilter(this);
    }

    d->m_completion = new KMyMoneyAccountCompletion(this);
    connect(d->m_completion, &KMyMoneyCompletion::itemSelected, this, &KMyMoneyCategory::slotItemSelected);
    connect(this, &QComboBox::editTextChanged, d->m_completion, &KMyMoneyCompletion::slotMakeCompletion);
}

KMyMoneyCategory::~KMyMoneyCategory()
{
    Q_D(KMyMoneyCategory);
    // make sure to wipe out the frame, button and layout
    if (d->frame && !d->frame->parentWidget())
        d->frame->deleteLater();
}

QPushButton* KMyMoneyCategory::splitButton() const
{
    Q_D(const KMyMoneyCategory);
    return d->splitButton;
}

void KMyMoneyCategory::setPalette(const QPalette& palette)
{
    Q_D(KMyMoneyCategory);
    if (d->frame)
        d->frame->setPalette(palette);
    KMyMoneyCombo::setPalette(palette);
}

void KMyMoneyCategory::reparent(QWidget *parent, Qt::WindowFlags w, const QPoint&, bool showIt)
{
    Q_D(KMyMoneyCategory);
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

KMyMoneyAccountSelector* KMyMoneyCategory::selector() const
{
    return dynamic_cast<KMyMoneyAccountSelector*>(KMyMoneyCombo::selector());
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
    Q_D(KMyMoneyCategory);
    setCurrentTextById(id);

    d->m_completion->hide();

    if (d->m_id != id) {
        d->m_id = id;
        Q_EMIT itemSelected(id);
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
    QMetaObject::invokeMethod(this, "focusIn", Qt::QueuedConnection);
}

void KMyMoneyCategory::setSplitTransaction()
{
    Q_D(KMyMoneyCategory);
    d->isSplit = true;
    setEditText(i18nc("Split transaction (category replacement)", "Split transaction"));
    setSuppressObjectCreation(true);
}

bool KMyMoneyCategory::isSplitTransaction() const
{
    Q_D(const KMyMoneyCategory);
    return d->isSplit;
}

void KMyMoneyCategory::setCurrentText(const QString& txt)
{
    KMyMoneyCombo::setCurrentText(txt);
}

void KMyMoneyCategory::setCurrentText()
{
    KMyMoneyCombo::setCurrentText(QString());
}

bool KMyMoneyCategory::eventFilter(QObject *o, QEvent *ev)
{
    Q_D(KMyMoneyCategory);
    // forward enable/disable state to split button
    if (o == this && ev->type() == QEvent::EnabledChange) {
        if (d->splitButton) {
            d->splitButton->setEnabled(isEnabled());
        }
    }
    return KMyMoneyCombo::eventFilter(o, ev);
}

KMyMoneySecurity::KMyMoneySecurity(QWidget* parent) :
    KMyMoneyCategory(false, parent)
{
}

KMyMoneySecurity::~KMyMoneySecurity()
{
}

void KMyMoneySecurity::setCurrentText(const QString& txt)
{
    KMyMoneyCategory::setCurrentText(txt);
}

void KMyMoneySecurity::setCurrentText()
{
    KMyMoneyCategory::setCurrentText(QString());
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
