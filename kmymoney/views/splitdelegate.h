/*
    SPDX-FileCopyrightText: 2016-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPLITDELEGATE_H
#define SPLITDELEGATE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneySecurity;

class SplitDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SplitDelegate(QObject* parent = 0);
    virtual ~SplitDelegate();

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

    void setShowValuesInverted(bool inverse);
    bool showValuesInverted();

    void setCommodity(const MyMoneySecurity& commodity);
    void setTransactionPayeeId(const QString& id);

    static void setErroneousColor(const QColor& color);
    static void setImportedColor(const QColor& color);

    static QColor erroneousColor();

protected:
    bool eventFilter(QObject* o, QEvent* event) final override;

protected Q_SLOTS:
    void endEdit();

Q_SIGNALS:
    void sizeHintChanged(const QModelIndex&) const;

private:
    class Private;
    Private * const d;

    static QColor m_erroneousColor;
    static QColor m_importedColor;
};

#endif // SPLITDELEGATE_H

