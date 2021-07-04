/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KPAYEEIDENTIFIERVIEW_H
#define KPAYEEIDENTIFIERVIEW_H

#include <QWidget>

#include "mymoneypayeeidentifiercontainer.h"
#include "widgets/styleditemdelegateforwarder.h"

class payeeIdentifier;

namespace Ui {
class KPayeeIdentifierView;
}

class KPayeeIdentifierView : public QWidget
{
    Q_OBJECT

public:
    explicit KPayeeIdentifierView(QWidget* parent);
    ~KPayeeIdentifierView();
    QList<payeeIdentifier> identifiers() const;

    void closeSource();

Q_SIGNALS:
    void dataChanged();

public Q_SLOTS:
    void setSource(MyMoneyPayeeIdentifierContainer data);

private Q_SLOTS:
    void removeSelected();

private:
    Ui::KPayeeIdentifierView* ui;
};

class payeeIdentifierDelegate : public StyledItemDelegateForwarder
{
    Q_OBJECT
public:
    explicit payeeIdentifierDelegate(QObject* parent = 0);
    virtual QAbstractItemDelegate* getItemDelegate(const QModelIndex& index) const override;
};

#endif // KPAYEEIDENTIFIERVIEW_H
