/*  This file is part of the KDE project
    Copyright (C) 2009 Laurent Montel <montel@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef TRANSACTIONSORTOPTIONIMPL_H
#define TRANSACTIONSORTOPTIONIMPL_H

#include "ui_transactionsortoption.h"

class TransactionSortOption : public QWidget, public Ui::TransactionSortOption
{
    Q_OBJECT
public:
    TransactionSortOption( QWidget *parent );
public slots:
    void setSettings(const QString& settings);
    void toggleDirection( Q3ListViewItem * item );

protected:
    void addEntry( K3ListView * p, Q3ListViewItem * after, int idx );
protected slots:
    void slotAvailableSelected( QListViewItem * item );
    void slotSelectedSelected( QListViewItem * item );
    void slotAddItem( void );
    void slotRemoveItem( void );
    void slotUpItem( void );
    void slotDownItem( void );
private:
    void init();
};



#endif /* TRANSACTIONSORTOPTIONIMPL_H */

