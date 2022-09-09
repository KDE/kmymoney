/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "payeeidentifierselectiondelegate.h"

#include <KLocalizedString>

#include "models/payeeidentifiercontainermodel.h"
#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"


payeeIdentifierTypeSelectionWidget::payeeIdentifierTypeSelectionWidget(QWidget* parent)
    : QComboBox(parent)
{
    connect(this, QOverload<int>::of(&payeeIdentifierTypeSelectionWidget::activated), this, &payeeIdentifierTypeSelectionWidget::itemSelected);
}

void payeeIdentifierTypeSelectionWidget::itemSelected(int index)
{
    if (index != 0) {
        Q_EMIT commitData(this);
        setCurrentIndex(0);
    }
}

payeeIdentifierSelectionDelegate::payeeIdentifierSelectionDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* payeeIdentifierSelectionDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    payeeIdentifierTypeSelectionWidget* comboBox = new payeeIdentifierTypeSelectionWidget(parent);
    comboBox->setFrame(false);
    connect(comboBox, &payeeIdentifierTypeSelectionWidget::commitData, this, &payeeIdentifierSelectionDelegate::commitData);
    comboBox->addItem(i18n("Please select the account number type"));

    const QMap<QString, QString> availableDelegates {
        {payeeIdentifiers::ibanBic::staticPayeeIdentifierIid(),         i18n("IBAN and BIC")},
        {payeeIdentifiers::nationalAccount::staticPayeeIdentifierIid(), i18n("National Account Number")}
    };

    for (auto delegate = availableDelegates.cbegin(); delegate != availableDelegates.cend(); ++delegate )
        comboBox->addItem(delegate.value(), delegate.key());

    return comboBox;
}

void payeeIdentifierSelectionDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox *const comboBox = qobject_cast<QComboBox*>(editor);
    const QString selectedPidType = comboBox->model()->data(comboBox->model()->index(comboBox->currentIndex(), 0), Qt::UserRole).toString();
    payeeIdentifier orig = model->data(index, payeeIdentifierContainerModel::payeeIdentifier).value<payeeIdentifier>();

    payeeIdentifier ident;
    if (selectedPidType == payeeIdentifiers::ibanBic::staticPayeeIdentifierIid())
        ident = payeeIdentifier(orig.id(), new payeeIdentifiers::ibanBic());
    else if (selectedPidType == payeeIdentifiers::nationalAccount::staticPayeeIdentifierIid())
        ident = payeeIdentifier(orig.id(), new payeeIdentifiers::nationalAccount());

    model->setData(index, QVariant::fromValue<payeeIdentifier>(ident), payeeIdentifierContainerModel::payeeIdentifier);
}

void payeeIdentifierSelectionDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
{
    editor->setGeometry(option.rect);
}

QSize payeeIdentifierSelectionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
