/*
    SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IBANBICITEMEDIT_H
#define IBANBICITEMEDIT_H

#include <QWidget>
#include <payeeidentifier/payeeidentifier.h>

namespace Ui
{
class ibanBicItemEdit;
}

class ibanBicItemEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(payeeIdentifier identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged STORED true)
    Q_PROPERTY(QString iban READ iban WRITE setIban NOTIFY ibanChanged STORED false DESIGNABLE true)
    Q_PROPERTY(QString bic READ bic WRITE setBic NOTIFY bicChanged STORED false DESIGNABLE true)

public:
    explicit ibanBicItemEdit(QWidget* parent = 0);

    payeeIdentifier identifier() const;
    QString iban() const;
    QString bic() const;

public Q_SLOTS:
    void setIdentifier(const payeeIdentifier&);
    void setIban(const QString&);
    void setBic(const QString&);

Q_SIGNALS:
    void commitData(QWidget*);
    void closeEditor(QWidget* editor);
    void identifierChanged(payeeIdentifier);
    void ibanChanged(QString);
    void bicChanged(QString);

private Q_SLOTS:
    void updateIdentifier();

    /** @brief emits commitData(this) and closeEditor(this) */
    void editFinished();

private:
    struct Private;
    Private* d;
};

#endif // IBANBICITEMEDIT_H
