/*
 *    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TAGCREATOR_H
#define TAGCREATOR_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
class QAbstractButton;
class KTagContainer;

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

/**
 * This class implements the creation of a new tag
 * based on the data provided through a KTagContainer.
 * It can be called within a focusOut event handler to check
 * if the text contained in the combobox of the KTagContainer
 * refers to an existing item or needs to create a new one.
 *
 * If the tag is created, it will be added to the KTagContainer
 * and the focus is moved to the next widget.
 * If it is not created, the text will be removed in the
 * lineedit of the combobox and the focus remains in the
 * combobox.
 *
 * Since any editor buttons (enter, cancel, etc.) also issue
 * a focusOut event on the combobox, they need to be added
 * to this object using the @c addButton() method so that the
 * creation does not trigger when one of the buttons is
 * pressed.
 *
 * Use @c setComboBox() to identify the combobox to be used.
 *
 * Calling @c createTag() will start the creation process
 * delayed by at least 150ms and returns immediately. This period
 * is extended in case one of the buttons added using @c addButton()
 * is still pressed. In case the object is destroyed before the
 * waiting period elapses, the creation will not be triggered.
 * Once the creation is processed, the object destroys itself.
 */
class KMM_BASE_DIALOGS_EXPORT TagCreator : public QObject
{
    Q_OBJECT
public:
    explicit TagCreator(QObject* parent);

    void addButton(QAbstractButton* button);
    void setTagContainer(KTagContainer* tagContainer);

public Q_SLOTS:
    void createTag();

private:
    QList<QAbstractButton*> m_buttons;
    KTagContainer* m_tagContainer;
    QString m_name;
};

#endif // TAGCREATOR_H
