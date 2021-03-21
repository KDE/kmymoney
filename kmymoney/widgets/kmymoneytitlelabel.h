/*
    SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYTITLELABEL_H
#define KMYMONEYTITLELABEL_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QColor>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author ace jones
  */
class KMyMoneyTitleLabelPrivate;
class KMM_BASE_WIDGETS_EXPORT KMyMoneyTitleLabel : public QLabel
{
    Q_OBJECT
    Q_DISABLE_COPY(KMyMoneyTitleLabel)
    Q_PROPERTY(QString leftImageFile READ leftImageFile WRITE setLeftImageFile DESIGNABLE true)
    Q_PROPERTY(QString rightImageFile READ rightImageFile WRITE setRightImageFile DESIGNABLE true)
    Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor DESIGNABLE true)
    Q_PROPERTY(QString text READ text WRITE setText DESIGNABLE true)

public:
    explicit KMyMoneyTitleLabel(QWidget* parent = nullptr);
    ~KMyMoneyTitleLabel();

    void setBgColor(const QColor& _color);
    void setLeftImageFile(const QString& _file);
    void setRightImageFile(const QString& _file);

    QString leftImageFile() const;
    QString rightImageFile() const;
    QColor bgColor() const;
    QString text() const;

public Q_SLOTS:
    virtual void setText(const QString& txt);

protected:
    void updatePixmap();
    void paintEvent(QPaintEvent *) override;

private:
    KMyMoneyTitleLabelPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KMyMoneyTitleLabel)
};

#endif
