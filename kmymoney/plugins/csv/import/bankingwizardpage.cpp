/*
    SPDX-FileCopyrightText: 2011-2017 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bankingwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QStandardItemModel>
#include <QTextStream>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "core/csvimportercore.h"
#include "csvwizard.h"
#include "icons.h"
#include "mymoneyqifprofile.h"

#include "ui_bankingwizardpage.h"

// ----------------------------------------------------------------------------

#define connectClearButton(col) \
  connect(ui->col ## Clear, &QToolButton::clicked, this, [&]() { clearComboBox(ui->col ); } );

BankingPage::BankingPage(CSVWizard *dlg, CSVImporterCore *imp)
    : CSVWizardPage(dlg, imp)
    , m_profile(nullptr)
    , ui(new Ui::BankingPage)
{
    ui->setupUi(this);

    m_columnBoxes = QHash<Column, QComboBox*> {
        {Column::Amount, ui->m_amountCol},
        {Column::Debit, ui->m_debitCol},
        {Column::Credit, ui->m_creditCol},
        {Column::Memo, ui->m_memoCol},
        {Column::Number, ui->m_numberCol},
        {Column::Date, ui->m_dateCol},
        {Column::Payee,  ui->m_payeeCol},
        {Column::Category, ui->m_categoryCol},
        {Column::CreditDebitIndicator, ui->m_creditDebitIndicatorCol},
        {Column::Balance, ui->m_balanceCol},
    };

    auto clearComboBox = [&](QComboBox* combobox) {
        combobox->setCurrentIndex(-1);
    };

    connect(ui->m_clear, &QAbstractButton::clicked, this, &BankingPage::clearColumns);
    connect(ui->m_oppositeSigns, &QAbstractButton::clicked, this, [&](bool checked) {
        m_profile->m_oppositeSigns = checked;
    });

    connect(ui->m_amountTabWidget, &QTabWidget::currentChanged, this, [&](int index) {
        if (ui->m_amountTabWidget->widget(index) == ui->amountTab) { // amountTab
            clearComboBox(ui->m_debitCol);
            clearComboBox(ui->m_creditCol);
        } else {          // creditDebitTab
            clearComboBox(ui->m_amountCol);
        }
    });

    void (QComboBox::* signal)(int) = &QComboBox::currentIndexChanged;
    connect(ui->m_amountCol, signal, this, [&](int col) {
        validateSelectedColumn(col, Column::Amount);
    });
    connect(ui->m_debitCol, signal, this, [&](int col) {
        validateSelectedColumn(col, Column::Debit);
    });
    connect(ui->m_creditCol, signal, this, [&](int col) {
        validateSelectedColumn(col, Column::Credit);
    });
    connect(ui->m_numberCol, signal, this, [&](int col) {
        validateSelectedColumn(col, Column::Number);
    });
    connect(ui->m_dateCol, signal, this, [&](int col) {
        validateSelectedColumn(col, Column::Date);
    });
    connect(ui->m_categoryCol, signal, this, [&](int col) {
        validateSelectedColumn(col, Column::Category);
    });

    connect(ui->m_creditIndicator, &QLineEdit::textEdited, [&](const QString& indicator) {
        m_profile->m_creditIndicator = indicator;
        Q_EMIT completeChanged();
    });
    connect(ui->m_debitIndicator, &QLineEdit::textEdited, [&](const QString&  indicator) {
        m_profile->m_debitIndicator = indicator;
        Q_EMIT completeChanged();
    });

    connect(ui->m_memoCol, signal, this, &BankingPage::memoColSelected);
    connect(ui->m_payeeCol, signal, this, &BankingPage::payeeColSelected);
    connect(ui->m_creditDebitIndicatorCol, signal, this, [&](int col) {
        if (validateSelectedColumn(col, Column::CreditDebitIndicator)) {
            ui->m_creditIndicator->setDisabled(col == -1);
            ui->m_debitIndicator->setDisabled(col == -1);
            ui->m_oppositeSigns->setEnabled(col == -1);
            ui->labelBnk_opposite->setEnabled(col == -1);
        }
    });

    connect(ui->m_memoColClear, &QToolButton::clicked, this, &BankingPage::clearMemoColumns);

    // connect the clear buttons with the comboboxes
    connectClearButton(m_numberCol);
    connectClearButton(m_dateCol);
    connectClearButton(m_payeeCol);
    connectClearButton(m_categoryCol);
    connectClearButton(m_balanceCol);
    connectClearButton(m_amountCol);
    connectClearButton(m_creditDebitIndicatorCol);
    connectClearButton(m_debitCol);
    connectClearButton(m_creditCol);

    // setup clear icon on toolbuttons
    const auto toolButtons = findChildren<QToolButton*>();
    for (const auto& button : toolButtons) {
        button->setIcon(Icons::get(Icons::Icon::EditClear));
    }

    // assume debit/credit indicator is not filled
    ui->m_creditIndicator->setDisabled(true);
    ui->m_debitIndicator->setDisabled(true);
}

BankingPage::~BankingPage()
{
    delete ui;
}

void BankingPage::initializePage()
{
    m_profile = dynamic_cast<BankingProfile *>(m_imp->m_profile);
    updateCurrentMemoSelection();

    // fill in column numbers into all comboboxes
    if (ui->m_dateCol->count() != m_imp->m_file->m_columnCount)
        m_dlg->initializeComboBoxes(m_columnBoxes);

    m_dlg->m_colTypeName.clear();
    const auto labels = findChildren<QLabel*>();
    for (auto it = m_columnBoxes.cbegin(); it != m_columnBoxes.cend(); ++it) {
        // m_dlg->m_colTypeName is constructed based on the QLabel::buddy()
        // setup in the UI file pointing to the combobox
        for (const auto& label : labels) {
            if (label->buddy() == it.value()) {
                m_dlg->m_colTypeName.insert(it.key(), label->text());
                break;
            }
        }
        if (!m_dlg->m_colTypeName.contains(it.key())) {
            qWarning() << "No colTypeName in BankingPage for" << it.value()->objectName();
        }
        // skip memo column, we take of it later
        if (it.key() == Column::Memo)
            continue;

        auto index = -1;
        if (m_profile->m_colTypeNum.contains(it.key())) {
            index = m_profile->m_colTypeNum.value(it.key());
        }
        // reset values to undefined in case out of range or unknown
        if ((index == -1) || (index >= it.value()->count())) {
            m_profile->m_colTypeNum[it.key()] = -1;
        }
        it.value()->setCurrentIndex(m_profile->m_colTypeNum.value(it.key()));
    }

    ui->m_oppositeSigns->setChecked(m_profile->m_oppositeSigns);

    ui->m_memoCol->setCurrentIndex(-1);
    for (int i = 0; i < m_profile->m_memoColList.count(); ++i) {
        ui->m_memoCol->setCurrentIndex(m_profile->m_memoColList.value(i));
    }

    if (m_profile->m_colTypeNum.value(Column::Debit) == -1)     // If amount previously selected, setup tab
        ui->m_amountTabWidget->setCurrentWidget(ui->amountTab);
    else                                     // ...else set credit/debit tab
        ui->m_amountTabWidget->setCurrentWidget(ui->debitCreditTab);

    ui->m_creditIndicator->setText(m_profile->m_creditIndicator);
    ui->m_debitIndicator->setText(m_profile->m_debitIndicator);
}

int BankingPage::nextId() const
{
    return CSVWizard::PageFormats;
}

bool BankingPage::isComplete() const
{
    bool rc = (ui->m_dateCol->currentIndex() > -1) &&
              (ui->m_payeeCol->currentIndex() > -1);

    if (ui->m_amountTabWidget->currentIndex() == 0) { // amountTab selected
        rc &= (ui->m_amountCol->currentIndex() > -1);
        if (ui->m_creditDebitIndicatorCol->currentIndex() > -1) {
            // at least one of the indicators must be filled and they both must differ
            rc &= !(ui->m_debitIndicator->text().isEmpty() && ui->m_creditIndicator->text().isEmpty());
            rc &= (ui->m_debitIndicator->text() != ui->m_creditIndicator->text());
        }
    } else {
        // debit and credit must be filled
        rc &= (ui->m_debitCol->currentIndex() > -1) &&
              (ui->m_creditCol->currentIndex() > -1);
    }
    return rc;
}

bool BankingPage::validateMemoComboBox()
{
    if (!m_profile->m_memoColList.isEmpty()) {
        for (int i = 0; i < ui->m_memoCol->count(); ++i)
        {
            const QString txt = ui->m_memoCol->itemText(i);
            if (txt.contains(QLatin1Char('*')))  // check if text containing '*' belongs to valid column types
                if (m_profile->m_colNumType.value(i) != Column::Payee) {
                    ui->m_memoCol->setItemText(i, QString::number(i + 1));
                    m_profile->m_memoColList.removeOne(i);
                    return false;
                }
        }
    }
    return true;
}

void BankingPage::memoColSelected(int col)
{
    if (col != -1) {
        if (m_profile->m_colNumType.value(col) == Column::Payee ) {
            int rc = KMessageBox::PrimaryAction;
            if (isVisible())
                rc = KMessageBox::questionTwoActions(m_dlg,
                                                     i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                                          "<center>If you wish to copy the Payee data to the memo field, click 'Yes'.</center>",
                                                          m_dlg->m_colTypeName.value(m_profile->m_colNumType.value(col))),
                                                     i18nc("@title:window", "Field assignment resolution"),
                                                     KStandardGuiItem::yes(),
                                                     KStandardGuiItem::no());
            if (rc == KMessageBox::PrimaryAction) {
                ui->m_memoCol->setItemText(col, QString::number(col + 1) + QLatin1Char('*'));
                if (!m_profile->m_memoColList.contains(col))
                    m_profile->m_memoColList.append(col);
            } else {
                ui->m_memoCol->setItemText(col, QString::number(col + 1));
                m_profile->m_memoColList.removeOne(col);
            }
            //allow only separate memo field occupy combobox
            QSignalBlocker blocker(ui->m_memoCol);
            if (m_profile->m_colTypeNum.value(Column::Memo) != -1)
                ui->m_memoCol->setCurrentIndex(m_profile->m_colTypeNum.value(Column::Memo));
            else
                ui->m_memoCol->setCurrentIndex(-1);

        } else {
            if (m_profile->m_colTypeNum.value(Column::Memo) != -1)        // check if this memo has any column 'number' assigned...
                m_profile->m_memoColList.removeOne(col);           // ...if true remove it from memo list

            if(validateSelectedColumn(col, Column::Memo)) {
                if (!m_profile->m_memoColList.contains(col)) {
                    m_profile->m_memoColList.append(col);
                    std::sort(m_profile->m_memoColList.begin(), m_profile->m_memoColList.end());
                }
            }
        }
        updateCurrentMemoSelection();
        // always clear the col in the combo box after it is added to the list
        ui->m_memoCol->setCurrentIndex(-1);

    }
}

void BankingPage::updateCurrentMemoSelection()
{
    const auto& list = m_profile->m_memoColList;
    const bool haveSelection = !list.isEmpty();
    QString txt = i18nc("@item:intext No field selection", "None");
    if (haveSelection) {
        txt.clear();
        for (const auto& entry : list) {
            txt += QString("%1, ").arg(entry+1);
        }
        txt = txt.left(txt.length()-2);
    }
    ui->m_currentMemoColumns->setText(i18nc("@label:listbox list of currently selected fields", "<i>Memo columns:</i> %1", txt));

    ui->m_memoColClear->setEnabled(haveSelection);
}

void BankingPage::payeeColSelected(int col)
{
    if (validateSelectedColumn(col, Column::Payee))
        if (!validateMemoComboBox() && col != -1)  // user could have it already in memo so...
            memoColSelected(col);    // ...if true set memo field again
}

void BankingPage::clearColumns()
{
    for (const auto& comboBox : m_columnBoxes) {
        comboBox->setCurrentIndex(-1);
    }
    ui->m_creditIndicator->clear();
    ui->m_debitIndicator->clear();
    clearMemoColumns();
}

void BankingPage::clearMemoColumns()
{
    m_profile->m_colTypeNum[Column::Memo] = -1;

    for (auto it = m_profile->m_colNumType.begin(); it != m_profile->m_colNumType.end(); /* no inc here */) {
        if (it.value() == Column::Memo) {
            it = m_profile->m_colNumType.erase(it);
        } else {
            ++it;
        }
    }
    m_profile->m_memoColList.clear();

    updateCurrentMemoSelection();
}

void BankingPage::resetComboBox(Column comboBox, int index)
{
    if (m_columnBoxes.contains(comboBox)) {
        m_columnBoxes.value(comboBox)->setCurrentIndex(index);
    } else {
        KMessageBox::error(m_dlg, i18n("<center>Field name not recognised.</center> <center>'<b>%1</b>'</center> Please re-enter your column selections."
                                       , static_cast<int>(comboBox)), i18n("CSV import"));
    }
}

bool BankingPage::validateSelectedColumn(const int col, const Column type)
{
    if (m_profile->m_colTypeNum.value(type) != -1)        // check if this 'type' has any column 'number' assigned...
        m_profile->m_colNumType.remove(m_profile->m_colTypeNum[type]); // ...if true remove 'type' assigned to this column 'number'

    bool ret = true;
    if (col == -1) { // user only wanted to reset his column so allow him
        m_profile->m_colNumType.remove(m_profile->m_colTypeNum[type]);
        m_profile->m_colTypeNum[type] = col;  // assign new column 'number' to this 'type'

    } else if (col == m_profile->m_colTypeNum[type]) {
        // nothing to do since it is the same value

    } else if (m_profile->m_colNumType.contains(col)) { // if this column 'number' has already 'type' assigned
        KMessageBox::information(m_dlg, i18n("Column <b>%1</b> cannot be selected because it is already used in '<b>%2</b>'.",
                                             col+1, m_dlg->m_colTypeName.value(m_profile->m_colNumType.value(col))));
        resetComboBox(type, m_profile->m_colTypeNum[type]);
        ret = false;

    } else {
        m_profile->m_colTypeNum[type] = col; // assign new column 'number' to this 'type'
        m_profile->m_colNumType[col] = type; // assign new 'type' to this column 'number'
    }
    Q_EMIT completeChanged();
    return ret;
}

bool BankingPage::validateCreditDebit()
{
    for (int row = m_profile->m_startLine; row <= m_profile->m_endLine; ++row) {
        // process credit/debit field
        if (m_profile->m_colTypeNum.value(Column::Credit) != -1 &&
                m_profile->m_colTypeNum.value(Column::Debit) != -1) {
            QString credit = m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(Column::Credit))->text();
            QString debit = m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(Column::Debit))->text();
            m_imp->processCreditDebit(credit, debit);
            if (!credit.isEmpty() && !debit.isEmpty()) {
                int ret = KMessageBox::questionTwoActionsCancel(m_dlg,
                                                                i18n("<center>The %1 field contains '%2'</center>"
                                                                     "<center>and the %3 field contains '%4'.</center>"
                                                                     "<center>Please choose which you wish to accept.</center>",
                                                                     m_dlg->m_colTypeName.value(Column::Debit),
                                                                     debit,
                                                                     m_dlg->m_colTypeName.value(Column::Credit),
                                                                     credit),
                                                                i18n("CSV invalid field values"),
                                                                KGuiItem(i18n("Accept %1", m_dlg->m_colTypeName.value(Column::Debit))),
                                                                KGuiItem(i18n("Accept %1", m_dlg->m_colTypeName.value(Column::Credit))),
                                                                KGuiItem(i18n("Cancel")));
                switch(ret) {
                case KMessageBox::Cancel:
                    return false;
                case KMessageBox::PrimaryAction:
                    m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(Column::Credit))->setText(QString());
                    break;
                case KMessageBox::SecondaryAction:
                    m_imp->m_file->m_model->item(row, m_profile->m_colTypeNum.value(Column::Debit))->setText(QString());
                    break;
                }
            }
        }
    }
    return true;
}

void BankingPage::makeQIF(const MyMoneyStatement& st, const QString& outFileName, const MyMoneyQifProfile& qifProfile)
{
    QFile oFile(outFileName);
    oFile.open(QIODevice::WriteOnly);
    QTextStream out(&oFile);

    QString buffer;
    QString strEType;

    switch (st.m_eType) {
    case eMyMoney::Statement::Type::CreditCard:
        strEType = QStringLiteral("CCard");
        break;
    case eMyMoney::Statement::Type::Savings:
    case eMyMoney::Statement::Type::Checkings:
    default:
        strEType = QStringLiteral("Bank");
    }

    const auto eol(QLatin1Char('\n'));

    if (!st.m_strAccountName.isEmpty()) {
        buffer.append(QStringLiteral("!Account\n"));
        buffer.append(QLatin1Char('N') + st.m_strAccountName + eol);
        buffer.append(QLatin1Char('T') + strEType + eol);
        buffer.append(QStringLiteral("^\n"));
    }

    buffer.append(QStringLiteral("!Type:") + strEType + eol);

    for (QList<MyMoneyStatement::Transaction>::const_iterator it = st.m_listTransactions.constBegin(); it != st.m_listTransactions.constEnd(); ++it) {
        buffer.append(QLatin1Char('D') + qifProfile.date(it->m_datePosted) + eol);
        buffer.append(QLatin1Char('T') + qifProfile.value(QLatin1Char('T'), it->m_amount) + eol);
        buffer.append(QLatin1Char('P') + it->m_strPayee + eol);
        if (!it->m_listSplits.isEmpty()) {
            buffer.append(QLatin1Char('L') + it->m_listSplits.first().m_strCategoryName + eol);
        }
        if (!it->m_strNumber.isEmpty()) {
            buffer.append(QLatin1Char('N') + it->m_strNumber + eol);
        }
        if (!it->m_strMemo.isEmpty()) {
            buffer.append(QLatin1Char('M') + it->m_strMemo + eol);
        }
        buffer.append(QStringLiteral("^\n"));
        out << buffer;// output qif file
        buffer.clear();
    }
    oFile.close();
}
