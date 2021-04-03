/*
    SPDX-FileCopyrightText: 2002 Thomas Baumgart <thb@net-bembel.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYQIFPROFILEEDITOR_H
#define MYMONEYQIFPROFILEEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
#include <QValidator>
class QTreeWidgetItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_mymoneyqifprofileeditor.h"
#include "mymoneyqifprofile.h"

/**
  * @author Thomas Baumgart
  */

class MyMoneyQifProfileNameValidator : public QValidator
{
    Q_OBJECT

public:
    explicit MyMoneyQifProfileNameValidator(QObject* o);
    virtual ~MyMoneyQifProfileNameValidator();

    QValidator::State validate(QString&, int&) const final override;
};


class MyMoneyQifProfileEditor : public QWidget, public Ui::MyMoneyQifProfileEditor
{
    Q_OBJECT

public:
    explicit MyMoneyQifProfileEditor(const bool edit = false, QWidget *parent = 0);
    virtual ~MyMoneyQifProfileEditor();

    /**
      * This method returns the currently selected profile in the list box.
      */
    const QString selectedProfile() const;

protected Q_SLOTS:
    void slotLoadProfileFromConfig(const QString& name);
    void slotReset();
    void slotRename();
    void slotDelete();
    void slotNew();
    void slotAmountTypeSelected();
    void slotDecimalChanged(const QString& val);
    void slotThousandsChanged(const QString& val);
    void slotHelp();

private:
    void loadProfileListFromConfig();
    void loadWidgets();
    void showProfile();
    void addProfile(const QString& name);
    void deleteProfile(const QString& name);
    const QString enterName(bool& ok);

private:
    bool                m_inEdit;
    MyMoneyQifProfile   m_profile;
    bool                m_isDirty;
    bool                m_isAccepted;
    QTreeWidgetItem*    m_selectedAmountType;
};

#endif
