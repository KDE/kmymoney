/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYPAYEEIDENTIFIERCONTAINER_H
#define MYMONEYPAYEEIDENTIFIERCONTAINER_H

#include "kmm_mymoney_export.h"

#include <QList>

#include "payeeidentifier/payeeidentifier.h"

/**
 *
 *
 * @internal payeeIdentifiers should get their own id. So they can be created as all other MyMoneyObjects.
 * But adding a MyMoneyObject to MyMoneyFile and its storage backends is so time-consuming,
 * I won't do that - sorry. So all payeeIdentifiers have to be created when a MyMoneyPayeeIdentifierContainer
 * is loaded. Optimal would be if they are only created if needed (which won't be often).
 */
template <class T> class payeeIdentifierTyped;
class KMM_MYMONEY_EXPORT MyMoneyPayeeIdentifierContainer
{
public:
    MyMoneyPayeeIdentifierContainer();

    unsigned int payeeIdentifierCount() const;
    ::payeeIdentifier payeeIdentifier(unsigned int) const;
    QList< ::payeeIdentifier > payeeIdentifiers() const;

    template< class type >
    QList< ::payeeIdentifierTyped<type> > payeeIdentifiersByType() const;

    void addPayeeIdentifier(const ::payeeIdentifier& ident);
    void addPayeeIdentifier(const unsigned int position, const ::payeeIdentifier& ident);

    void removePayeeIdentifier(const ::payeeIdentifier& ident);
    void removePayeeIdentifier(const int index);

    void modifyPayeeIdentifier(const ::payeeIdentifier& ident);
    void modifyPayeeIdentifier(const int index, const ::payeeIdentifier& ident);

    void resetPayeeIdentifiers(const QList< ::payeeIdentifier >& list = QList< ::payeeIdentifier >());

    void loadXML(QDomElement node);
    void writeXML(QDomDocument document, QDomElement parent) const;

protected:
    QList< ::payeeIdentifier > m_payeeIdentifiers;
};

template< class type >
QList< payeeIdentifierTyped<type> > MyMoneyPayeeIdentifierContainer::payeeIdentifiersByType() const
{
    QList< payeeIdentifierTyped<type> > typedList;
    return typedList;
}

#endif // MYMONEYPAYEEIDENTIFIERCONTAINER_H
