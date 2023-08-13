/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRANSACTIONEDITORBASE_H
#define TRANSACTIONEDITORBASE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"

class MyMoneyTransaction;
class SplitModel;
class QAbstractButton;
class QAbstractItemModel;
class WidgetHintFrameCollection;

class TransactionEditorBase : public QWidget
{
    Q_OBJECT

public:
    explicit TransactionEditorBase(QWidget* parent = 0, const QString& accountId = QString());
    virtual ~TransactionEditorBase();

    virtual bool accepted() const = 0;
    virtual void loadTransaction(const QModelIndex& index) = 0;
    virtual QStringList saveTransaction(const QStringList& selectedJournalEntries) = 0;
    virtual void setAmountPlaceHolderText(const QAbstractItemModel* model);

    /**
     * Inform the editor about the selected journal entries so that
     * it can check if the editor can handle the selection.
     * The default implementation does nothing and returns @c true.
     *
     * In case it cannot handle the selection, the method returns
     * @c false, the errorMessage() should return the reason why it
     * cannot do so.
     */
    virtual bool setSelectedJournalEntryIds(const QStringList& selectedJournalEntryIds);

    /**
     * Returns the reason in case setSelectedJournalEntryIds() returned
     * with @c false. The default implementation returns an empty string.
     */
    virtual QString errorMessage() const;

    virtual void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    QWidget* focusFrame() const;

    /**
     * This method is used to embed the transaction editor in other dialogs
     * e.g. KEditScheduleDlg. If the editor does not have a WidgetHintFrameCollection
     * then @c nullptr is returned. This is the default implementation.
     */
    virtual WidgetHintFrameCollection* widgetHintFrameCollection() const;

    void setVisible(bool visible) override;

public Q_SLOTS:
    virtual void slotSettingsChanged()
    {
    }

protected:
    void keyPressEvent(QKeyEvent* e) override;
    bool focusNextPrevChild(bool next) override;
    void setCancelButton(QAbstractButton* button);
    void setEnterButton(QAbstractButton* button);
    QStringList tabOrder(const QString& name, const QStringList& defaultTabOrder) const;
    void setupTabOrder(const QStringList& tabOrder);
    void storeTabOrder(const QString& name, const QStringList& tabOrder);

protected Q_SLOTS:
    virtual void reject();

Q_SIGNALS:
    void done();
    void editorLayoutChanged();

private:
    class Private;
    QScopedPointer<Private> const d;
};

#endif // TRANSACTIONEDITORBASE_H

