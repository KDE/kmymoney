/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMSEARCHWIDGET_H
#define KMMSEARCHWIDGET_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QComboBox>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMMSearchWidgetPrivate;
class KMM_BASE_WIDGETS_EXPORT KMMSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KMMSearchWidget(QWidget* parent);

    QLineEdit* lineEdit() const;
    QToolButton* closeButton() const;
    QComboBox* comboBox() const;

    void close();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;

Q_SIGNALS:
    void closed();

private:
    Q_DECLARE_PRIVATE(KMMSearchWidget);
    KMMSearchWidgetPrivate* d_ptr;
};
#endif // KMMSEARCHWIDGET_H
