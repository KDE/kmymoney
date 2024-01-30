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

#include "mymoneyenums.h"
#include "mymoneysplit.h"

class CreditDebitEdit;
class KMyMoneyAccountCombo;
class MyMoneyTransaction;
class QAbstractButton;
class QAbstractItemModel;
class QComboBox;
class KTagContainer;
class SplitModel;
class WidgetHintFrameCollection;

class TransactionEditorBase : public QWidget
{
    Q_OBJECT

public:
    explicit TransactionEditorBase(QWidget* parent = 0, const QString& accountId = QString());
    virtual ~TransactionEditorBase();

    /**
     * This method returns true if the user pressed the enter button.
     * It remains false, in case the user pressed the cancel button.
     */
    virtual bool accepted() const;
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
    virtual bool isTransactionDataValid() const = 0;

    /**
     * Check a category with the name entered into the
     * lineedit of @a comboBox needs to be created
     */
    bool needCreateCategory(KMyMoneyAccountCombo* comboBox) const;

    /**
     * Create a category/account based on the name provided
     * in @a comboBox and the @a type.
     *
     * @note This starts the creation editor and returns immediately
     */
    void createCategory(KMyMoneyAccountCombo* comboBox, eMyMoney::Account::Type type);

    /**
     * Return type depending on the amount entered into the @a valueWidget
     */
    eMyMoney::Account::Type defaultCategoryType(CreditDebitEdit* valueWidget) const;

    /**
     * Check if a payee with the name entered into the
     * lineedit of @a comboBox needs to be created
     *
     * @note As a side effect: sets combobox's completer case
     *       sensitivity to @c Qt::CaseSensitive
     */
    bool needCreatePayee(QComboBox* comboBox) const;

    /**
     * Create a payee based on the name provided
     * in @a comboBox.
     *
     * @note This starts the creation editor and returns immediately
     */
    void createPayee(QComboBox* comboBox);

    /**
     * Check if a tag with the name entered into the
     * lineedit of @a comboBox needs to be created
     *
     * @note As a side effect: sets combobox's completer case
     *       sensitivity to @c Qt::CaseSensitive
     */
    bool needCreateTag(QComboBox* comboBox) const;

    /**
     * Create a tag based on the name provided
     * in the combobox of @a tagContainer.
     *
     * @note This starts the creation editor and returns immediately
     */
    void createTag(KTagContainer* tagContainer);

protected Q_SLOTS:
    virtual void reject();
    virtual void acceptEdit();

Q_SIGNALS:
    void done();
    void editorLayoutChanged();

private:
    class Private;
    QScopedPointer<Private> const d;
};

#endif // TRANSACTIONEDITORBASE_H

