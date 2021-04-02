/*
    SPDX-FileCopyrightText: 2009-2016 Cristian One ț <onet.cristian@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2010-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYGENERALCOMBO_H
#define KMYMONEYGENERALCOMBO_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMM_WIDGETS_EXPORT KMyMoneyGeneralCombo : public KComboBox
{
    Q_OBJECT
    Q_DISABLE_COPY(KMyMoneyGeneralCombo)
    Q_PROPERTY(int currentItem READ currentItem WRITE setCurrentItem STORED false)

public:
    explicit KMyMoneyGeneralCombo(QWidget* parent = nullptr);
    virtual ~KMyMoneyGeneralCombo();

    void insertItem(const QString& txt, int id, int idx = -1);

    void setCurrentItem(int id);
    int currentItem() const;

    void removeItem(int id);

public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void itemSelected(int id);

protected:
    // prevent the caller to use the standard KComboBox insertItem function with a default idx
    void insertItem(const QString&);

protected Q_SLOTS:
    void slotChangeItem(int idx);

};

#endif
