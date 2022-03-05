/*
    SPDX-FileCopyrightText: 2016-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef NEWSPLITEDITOR_H
#define NEWSPLITEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QScopedPointer>
#include <QWidget>
class QAbstractItemModel;
class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "tabordereditor.h"

class MyMoneySecurity;

class NewSplitEditor : public QWidget, TabOrderEditorInterface
{
    Q_OBJECT

public:
    /**
     * @a accountId is the current account displayed for the transaction
     */
    explicit NewSplitEditor(QWidget* parent, const MyMoneySecurity& commodity, const QString& accountId = QString());
    virtual ~NewSplitEditor();

    /**
     * This method returns true if the user pressed the enter button.
     * It remains false, in case the user pressed the cancel button.
     */
    virtual bool accepted() const;

    void setShowValuesInverted(bool inverse);
    bool showValuesInverted();

    void setPostDate(const QDate& date);

    void startLoadingSplit();
    void finishLoadingSplit();

    void setAmountPlaceHolderText(const QAbstractItemModel* model);
    void setReadOnly(bool readOnly);

    // Implement TabOrderEditorInterface methods
    void setupUi(QWidget* parent) override;
    void storeTabOrder(const QStringList& tabOrder) override;

protected:
    void keyPressEvent(QKeyEvent* event) final override;
    bool focusNextPrevChild(bool next) override;
    bool eventFilter(QObject* o, QEvent* e) override;

public Q_SLOTS:
    /**
     * Returns the id of the selected account in the category widget
     */
    QString accountId() const;
    void setAccountId(const QString& id);

    /**
     * Returns the contents of the memo widget
     */
    QString memo() const;
    void setMemo(const QString& memo);

    MyMoneyMoney value() const;
    void setValue(const MyMoneyMoney& amount);

    MyMoneyMoney shares() const;
    void setShares(const MyMoneyMoney& amount);

    QString costCenterId() const;
    void setCostCenterId(const QString& id);

    QString number() const;
    void setNumber(const QString& id);

    QString payeeId() const;
    void setPayeeId(const QString& id);

    QList<QString> tagIdList() const;
    void setTagIdList(const QList<QString>& tagIds);

protected Q_SLOTS:
    virtual void reject();
    virtual void acceptEdit();

Q_SIGNALS:
    void done();
    void transactionChanged(const QString&);

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // NEWSPLITEDITOR_H

