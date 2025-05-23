/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JOURNALDELEGATE_H
#define JOURNALDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <kmmstyleditemdelegate.h>
class QColor;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"

class LedgerView;
class MyMoneyMoney;

class JournalDelegate : public KMMStyledItemDelegate
{
    Q_OBJECT
public:
    explicit JournalDelegate(LedgerView* parent = nullptr);
    virtual ~JournalDelegate();

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const final override;
    void setEditorData(QWidget* editWidget, const QModelIndex& index) const final override;

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

    /**
     * This method returns the row that currently has an editor
     * or -1 if no editor is open
     */
    virtual int editorRow() const;

    void setOnlineBalance(const QDate& date, const MyMoneyMoney& amount, int fraction = 0);

    /**
     * Which data (@a role) shall be displayed in the detail column
     * when only a single line is shown. The default is the payee.
     */
    void setSingleLineRole(eMyMoney::Model::Roles role);

    /**
     * If @a show is @c true, the payee name is shown in the
     * detail column if no payee column is present. During the
     * creation of a JournalDelegate this is set to @c true.
     */
    void setShowPayeeInDetailColumn(bool show);

    void setAccountType(eMyMoney::Account::Type accountType);

    static void setErroneousColor(const QColor& color);
    static void setImportedColor(const QColor& color);

    static QColor erroneousColor();

    /**
     * The maximum number of icons that can show up in parallel on
     * a transaction.
     */
    static int maxIcons()
    {
        return 3;
    }

    /**
     * This will force the next call to sizeHint to re-calculate
     * the size of a single line. Should be called when the font
     * characteristics change.
     */
    void resetLineHeight();

protected:
    bool eventFilter(QObject* o, QEvent* event) final override;

protected Q_SLOTS:
    void endEdit();

private:
    class Private;
    Private * const d;

    static QColor m_erroneousColor;
    static QColor m_importedColor;
    static QColor m_separatorColor;
};

#endif // JOURNALDELEGATE_H

