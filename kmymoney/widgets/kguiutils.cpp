/*
    SPDX-FileCopyrightText: 2006-2010 Tony Bloomfield <tonybloom@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kguiutils.h"

// ----------------------------------------------------------------------------
// QT Includes
// No need for QDateEdit, QSpinBox, etc., since these always return values

#include <QCheckBox>
#include <QPushButton>
#include <QWidget>
#include <QSpinBox>
#include <QApplication>
#include <QListWidget>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>
#include <KLineEdit>
#include <KUrlRequester>

// ----------------------------------------------------------------------------
// Project Includes

#include "amountedit.h"
#include "kmymoneypayeecombo.h"
#include "kmymoneysettings.h"
#include "kmymoneytextedit.h"
#include "onlinetasks/interfaces/ui/ionlinejobedit.h"
#include "widgethintframe.h"

/**************************************************************************
 *                                                                        *
 * The MandatoryFieldGroup code is courtesy of                            *
 * Mark Summerfield in Qt Quarterly                                       *
 * https://doc.qt.io/archives/qq/qq11-mandatoryfields.html                *
 *                                                                        *
 * Enhanced by Thomas Baumgart to support the lineedit field of a         *
 * a KComboBox.                                                           *
 *                                                                        *
 * With further widgets added by Allan Anderson for missing fields.       *
 **************************************************************************/

class KMandatoryFieldGroupPrivate
{
    Q_DISABLE_COPY(KMandatoryFieldGroupPrivate)

public:
    KMandatoryFieldGroupPrivate()
        : m_okButton(nullptr)
        , m_frameCollection(nullptr)
        , m_enabled(true)
        , m_externalMandatoryState(true)
    {
    }

    QList<QWidget*> m_widgets;
    QPushButton* m_okButton;
    WidgetHintFrameCollection* m_frameCollection;
    bool m_enabled;
    bool m_externalMandatoryState;
};

KMandatoryFieldGroup::KMandatoryFieldGroup(QObject *parent) :
    QObject(parent),
    d_ptr(new KMandatoryFieldGroupPrivate)
{
    Q_D(KMandatoryFieldGroup);
    d->m_frameCollection = new WidgetHintFrameCollection(this);
}

KMandatoryFieldGroup::~KMandatoryFieldGroup()
{
    Q_D(KMandatoryFieldGroup);
    delete d;
}

void KMandatoryFieldGroup::add(QWidget *widget)
{
    Q_D(KMandatoryFieldGroup);
    if (!d->m_widgets.contains(widget)) {
        if (qobject_cast<QCheckBox*>(widget))
            connect(qobject_cast<QCheckBox*>(widget),
                    &QCheckBox::clicked,
                    this, &KMandatoryFieldGroup::changed);

        else if (qobject_cast<KComboBox*>(widget)) {
            KComboBox* combo = qobject_cast<KComboBox*>(widget);
            KLineEdit* lineedit = qobject_cast<KLineEdit*>(combo->lineEdit());
            if (lineedit) {
                connect(lineedit, &QLineEdit::textChanged, this, &KMandatoryFieldGroup::changed);
            } else {
                connect(combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::highlighted), this, &KMandatoryFieldGroup::changed);
            }
        }

        else if (qobject_cast<QLineEdit*>(widget)) {
            connect(qobject_cast<QLineEdit*>(widget),
                    &QLineEdit::textChanged,
                    this, &KMandatoryFieldGroup::changed);
        }

        else if (qobject_cast<QSpinBox*>(widget))
            connect(qobject_cast<QSpinBox*>(widget),
                    static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
                    this, &KMandatoryFieldGroup::changed);

        else if (qobject_cast<QListWidget*>(widget))
            connect(qobject_cast<QListWidget*>(widget),
                    &QListWidget::itemSelectionChanged,
                    this, &KMandatoryFieldGroup::changed);

        else if (qobject_cast<KUrlRequester*>(widget))
            connect(qobject_cast<KUrlRequester*>(widget),
                    &KUrlRequester::textChanged,
                    this, &KMandatoryFieldGroup::changed);

        else if (qobject_cast<KMyMoneyTextEdit*>(widget))
            connect(qobject_cast<KMyMoneyTextEdit*>(widget),
                    &KMyMoneyTextEdit::textChanged,
                    this, &KMandatoryFieldGroup::changed);

        else if (qobject_cast<IonlineJobEdit*>(widget)) {
            connect(qobject_cast<IonlineJobEdit*>(widget),
                    &IonlineJobEdit::validityChanged,
                    this, &KMandatoryFieldGroup::changed);

            // Do not use WidgetHintFrame on IonlineJobEdits as they contain subwidgets
            d->m_widgets.append(widget);
            changed();
            return;
        }

        else {
            qWarning("MandatoryFieldGroup: unsupported class %s", (widget->metaObject()->className()));
            return;
        }

        d->m_frameCollection->addFrame(new WidgetHintFrame(widget));
        d->m_widgets.append(widget);
        changed();
    }
}

void KMandatoryFieldGroup::removeAll()
{
    Q_D(KMandatoryFieldGroup);
    while(!d->m_widgets.isEmpty()) {
        const auto widget = d->m_widgets.at(0);
        const auto frame = WidgetHintFrame::frameForWidget(widget);
        delete frame;
        remove(widget);
    }
    changed();
}

void KMandatoryFieldGroup::remove(QWidget *widget)
{
    Q_D(KMandatoryFieldGroup);

    const auto frame = WidgetHintFrame::frameForWidget(widget);
    delete frame;
    d->m_widgets.removeOne(widget);
    changed();
}

void KMandatoryFieldGroup::setOkButton(QPushButton *button)
{
    Q_D(KMandatoryFieldGroup);
    if (d->m_okButton && d->m_okButton != button)
        d->m_okButton->setEnabled(true);
    d->m_okButton = button;
    changed();
}

void KMandatoryFieldGroup::setExternalMandatoryState(bool state)
{
    Q_D(KMandatoryFieldGroup);
    if (d->m_externalMandatoryState != state) {
        d->m_externalMandatoryState = state;
        changed();
    }
}

void KMandatoryFieldGroup::changed()
{
    Q_D(KMandatoryFieldGroup);
    bool enable = d->m_externalMandatoryState;

    for (auto widget : qAsConst(d->m_widgets)) {
        bool widgetEmpty = false;

        auto showFrame = [](QWidget* editWidget, bool visible) -> void {
            (visible) ? WidgetHintFrame::show(editWidget) : WidgetHintFrame::hide(editWidget);
        };

        auto setWidgetEmpty = [&]() -> void {
            widgetEmpty = true;
            enable = false;
        };

        // disabled widgets don't count
        if (!(widget->isEnabled())) {
            continue;
        }

        if (qobject_cast<KMyMoneyPayeeCombo*>(widget)) {
            if ((dynamic_cast<KMyMoneyPayeeCombo*>(widget))->lineEdit()->text().isEmpty()) {
                setWidgetEmpty();
            }

        } else if (qobject_cast<QCheckBox*>(widget)) {
            if ((qobject_cast<QCheckBox*>(widget))->checkState() == Qt::PartiallyChecked) {
                setWidgetEmpty();
            }
        } else if (qobject_cast<KComboBox*>(widget)) {
            if ((qobject_cast<KComboBox*>(widget))->currentText().isEmpty()) {
                setWidgetEmpty();
            }
        } else if (qobject_cast<QLineEdit*>(widget)) {
            if ((qobject_cast<QLineEdit*>(widget))->text().isEmpty()) {
                setWidgetEmpty();
            }
        } else if ((qobject_cast<QListWidget*>(widget))) {
            if ((qobject_cast<QListWidget*>(widget))->selectedItems().isEmpty()) {
                setWidgetEmpty();
            }
        } else if ((qobject_cast<KUrlRequester*>(widget))) {
            if ((qobject_cast<KUrlRequester*>(widget))->text().isEmpty()) {
                setWidgetEmpty();
            }
        } else if ((qobject_cast<AmountEdit*>(widget))) {
            if (!(qobject_cast<AmountEdit*>(widget))->value().isZero()) {
                setWidgetEmpty();
            }
        } else if (qobject_cast<KMyMoneyTextEdit*>(widget)) {
            if (!(qobject_cast<KMyMoneyTextEdit*>(widget))->isValid()) {
                setWidgetEmpty();
            }
        } else if (qobject_cast<IonlineJobEdit*>(widget)) {
            if (!(qobject_cast<IonlineJobEdit*>(widget))->isValid()) {
                setWidgetEmpty();
            }
        }
        showFrame(widget, widgetEmpty);
    }

    if (d->m_okButton) {
        d->m_okButton->setEnabled(enable);
    }
    d->m_enabled = enable;

    Q_EMIT stateChanged();
    Q_EMIT stateChanged(enable);
}

bool KMandatoryFieldGroup::isEnabled() const
{
    Q_D(const KMandatoryFieldGroup);
    return d->m_enabled;
}

void KMandatoryFieldGroup::clear()
{
    Q_D(KMandatoryFieldGroup);

    for (auto widget : qAsConst(d->m_widgets)) {
        const auto frame = WidgetHintFrame::frameForWidget(widget);
        delete frame;
    }

    d->m_widgets.clear();

    if (d->m_okButton) {
        d->m_okButton->setEnabled(true);
        d->m_okButton = nullptr;
        d->m_enabled = true;
    }
}
