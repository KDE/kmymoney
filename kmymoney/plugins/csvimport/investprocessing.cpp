/*******************************************************************************
*                              investprocessing.cpp
*                              --------------------
* begin                       : Sat Jan 01 2010
* copyright                   : (C) 2010 by Allan Anderson
* email                       : agander93@gmail.com
* copyright                   : (C) 2016 by Łukasz Wojniłowicz
* email                       : lukasz.wojnilowicz@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#include "investprocessing.h"

// ----------------------------------------------------------------------------
// QT Headers

#include <QScrollBar>
#include <QDesktopWidget>
#include <QCloseEvent>
#include <QLineEdit>

#include <QtCore/QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <QUrl>
#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Headers

#include <KSharedConfig>
#include <kmessagebox.h>
#include <KAboutData>
#include <KAboutApplicationDialog>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QHBoxLayout>
#include <KLocalizedString>
#include <KConfigGroup>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyfile.h"
#include "kmymoney.h"

#include "convdate.h"
#include "csvutil.h"
#include "csvwizard.h"

#include "mymoneystatement.h"
#include "mymoneystatementreader.h"
#include "mymoneymoney.h"
#include "redefinedlg.h"

#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_bankingwizardpage.h"
#include "ui_lines-datewizardpage.h"
#include "ui_completionwizardpage.h"
#include "ui_investmentwizardpage.h"
#include "ui_csvwizard.h"
#include "symboltabledlg.h"

InvestProcessing::InvestProcessing()
{
  m_completer = 0;
  m_securityName.clear();

  m_colTypeName.insert(ColumnType, i18n("Type"));
  m_colTypeName.insert(ColumnPrice, i18n("Price"));
  m_colTypeName.insert(ColumnQuantity, i18n("Quantity"));
  m_colTypeName.insert(ColumnFee, i18n("Fee"));
  m_colTypeName.insert(ColumnDate, i18n("Date"));
  m_colTypeName.insert(ColumnAmount, i18n("Amount"));
  m_colTypeName.insert(ColumnSymbol, i18n("Symbol"));
  m_colTypeName.insert(ColumnName, i18n("Name"));
  m_colTypeName.insert(ColumnMemo, i18n("Memo"));

  m_redefine = new RedefineDlg;
  connect(m_redefine, SIGNAL(changedType(QString)), this, SLOT(changedType(QString)));
}

InvestProcessing::~InvestProcessing()
{
  delete m_symbolTableDlg;
  delete m_completer;
  delete m_redefine;
}

void InvestProcessing::init()
{
  m_symbolTableDlg  = new SymbolTableDlg;
  m_symbolTableDlg->m_investProcessing = this;

  m_securityName = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();

  QLineEdit* securityLineEdit = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->lineEdit();//krazy:exclude=<qclasses>

  m_completer = new QCompleter(m_securityList, this);
  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  securityLineEdit->setCompleter(m_completer);
  connect(securityLineEdit, SIGNAL(editingFinished()), this, SLOT(securityNameEdited()));
  connect(this, SIGNAL(statementReady(MyMoneyStatement&)), m_wiz->m_plugin, SLOT(slotGetStatement(MyMoneyStatement&)));

  //  The following string list strings are descriptions of possible investment
  //  activity types.  Each of the lists may also contain alternative descriptions,
  //  added by the user to the resource file, to suit his needs.

  QString text = "Type of operation as in financial statement";
  m_buyList += i18nc("%1", "buy", "%1", text);  //                       some basic entries in case rc file missing
  m_sellList << i18nc("%1", "sell", text)
             << i18nc("%1", "repurchase", text);
  m_divXList += i18nc("%1", "dividend", text);
  m_intIncList << i18nc("%1", "interest", text)
               << i18nc("%1", "income", text);
  m_reinvdivList << i18nc("%1", "reinvest", text)
                 << i18nc("%1", "reinv", text)
                 << i18nc("%1", "re-inv", text);
  m_shrsinList << i18nc("%1", "add", text)
               << i18nc("%1", "stock dividend", text)
               << i18nc("%1", "divd reinv", text)
               << i18nc("%1", "transfer in", text)
               << i18nc("%1", "re-registration in", text)
               << i18nc("%1", "journal entry", text);
  m_shrsoutList += i18nc("%1", "remove", text);
  text = "Brokerage type";
  m_brokerageList << i18nc("%1", "check", text)
                  << i18nc("%1", "payment", text)
                  << i18nc("%1", "bill payment", text)
                  << i18nc("%1", "dividend", text)
                  << i18nc("%1", "interest", text)
                  << i18nc("%1", "qualified div", text)
                  << i18nc("%1", "foreign tax paid", text)
                  << i18nc("%1", "adr mgmt fee", text);
}

void InvestProcessing::saveSettings()
{
  KConfigGroup profileNamesGroup(m_wiz->m_config, "ProfileNames");
  profileNamesGroup.writeEntry("Invest", m_wiz->m_profileList);
  profileNamesGroup.writeEntry("PriorInvest", m_wiz->m_profileList.indexOf(m_wiz->m_profileName));
  profileNamesGroup.config()->sync();

  KConfigGroup profilesGroup(m_wiz->m_config, "Invest-" + m_wiz->m_profileName);
  profilesGroup.writeEntry("DateFormat", m_wiz->m_pageLinesDate->ui->comboBox_dateFormat->currentIndex());
  profilesGroup.writeEntry("FieldDelimiter", m_wiz->m_pageSeparator->ui->comboBox_fieldDelimiter->currentIndex());
  profilesGroup.writeEntry("DecimalSymbol", m_wiz->m_pageCompletion->ui->comboBox_decimalSymbol->currentIndex());
  profilesGroup.writeEntry("PriceFraction", m_wiz->m_pageInvestment->ui->comboBoxInv_priceFraction->currentIndex());
  profilesGroup.writeEntry("StartLine", m_wiz->m_pageLinesDate->ui->spinBox_skip->value() - 1);
  profilesGroup.writeEntry("SecurityName", m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentIndex());
  profilesGroup.writeEntry("TrailerLines", m_wiz->m_pageLinesDate->m_trailerLines);
  //    The strings in these resource file lists may be edited,
  //    or expanded in the file by the user, to suit his needs.

  profilesGroup.writeEntry("ShrsinParam", m_shrsinList);
  profilesGroup.writeEntry("DivXParam", m_divXList);
  profilesGroup.writeEntry("IntIncParam", m_intIncList);
  profilesGroup.writeEntry("BrokerageParam", m_brokerageList);
  profilesGroup.writeEntry("ReinvdivParam", m_reinvdivList);
  profilesGroup.writeEntry("BuyParam", m_buyList);
  profilesGroup.writeEntry("SellParam", m_sellList);
  profilesGroup.writeEntry("RemoveParam", m_shrsoutList);

  QString str = m_wiz->m_pageInvestment->ui->lineEdit_filter->text();
  if (str.endsWith(' ')) {
    str.append('#');  //  Terminate trailing blank
  }
  profilesGroup.writeEntry("Filter", str);
  if (m_wiz->m_inFileName.startsWith("/home/")) // replace /home/user with ~/ for brevity
  {
    QFileInfo fileInfo = QFileInfo(m_wiz->m_inFileName);
    if (fileInfo.isFile())
      m_wiz->m_inFileName = fileInfo.absolutePath();
    m_wiz->m_inFileName = "~/" + m_wiz->m_inFileName.section('/',3);
  }

  profilesGroup.writeEntry("Directory", m_wiz->m_inFileName);
  profilesGroup.writeEntry("Encoding", m_wiz->m_encodeIndex);
  profilesGroup.writeEntry("DateCol", m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->currentIndex());
  profilesGroup.writeEntry("PayeeCol", m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->currentIndex());

  QList<int> list = m_wiz->m_memoColList;
  int posn = 0;
  if ((posn = list.indexOf(-1)) > -1) {
    list.removeOne(-1);
  }
  profilesGroup.writeEntry("MemoCol", list);
  profilesGroup.writeEntry("QuantityCol", m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->currentIndex());
  profilesGroup.writeEntry("AmountCol", m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->currentIndex());
  profilesGroup.writeEntry("PriceCol", m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->currentIndex());
  profilesGroup.writeEntry("FeeCol", m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->currentIndex());
  profilesGroup.writeEntry("SymbolCol", m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->currentIndex());
  profilesGroup.writeEntry("NameCol", m_wiz->m_pageInvestment->ui->comboBoxInv_nameCol->currentIndex());
  profilesGroup.writeEntry("FeeIsPercentage", int(m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->isChecked()));
  profilesGroup.writeEntry("FeeRate", m_wiz->m_pageInvestment->ui->lineEdit_feeRate->text());
  profilesGroup.writeEntry("MinFee", m_wiz->m_pageInvestment->ui->lineEdit_minFee->text());
  profilesGroup.config()->sync();

  KConfigGroup securitiesGroup(m_wiz->m_config, "Securities");
  securitiesGroup.writeEntry("SecurityNameList", securityList());
  securitiesGroup.config()->sync();
}

void InvestProcessing::clearFeesSelected()
{
  int i;
  if (!m_feeRate.isEmpty() && m_colTypeNum.value(ColumnFee) >= m_wiz->m_endColumn) { //delete fee colum, but only if it was generated
    m_wiz->m_maxColumnCount--;
    m_wiz->ui->tableWidget->setColumnCount(m_wiz->m_maxColumnCount);
    i = m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->currentIndex();
    m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
    m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->removeItem(i);
  }
  m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setEnabled(true);
  m_wiz->m_pageInvestment->ui->lineEdit_feeRate->setEnabled(true);
  m_wiz->m_pageInvestment->ui->lineEdit_minFee->setEnabled(true);
  m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setEnabled(true);
  m_wiz->m_pageInvestment->ui->buttonInv_calculateFee->setEnabled(false);
  m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->lineEdit_feeRate->clear();
  m_wiz->m_pageInvestment->ui->lineEdit_minFee->clear();
  m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setChecked(false);
  m_wiz->updateWindowSize();
}

void InvestProcessing::clearColumnsSelected()
{
  clearColumnNumbers();
  clearComboBoxText();
  m_wiz->m_memoColList.clear();
  m_colNumType.clear();
}

void InvestProcessing::clearColumnNumbers()
{
  m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_nameCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);
}

void InvestProcessing::clearComboBoxText()
{
  for (int i = 0; i < m_wiz->m_maxColumnCount; i++)
    m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(i, QString().setNum(i + 1));
}



void InvestProcessing::feeInputsChanged()
{
  m_wiz->m_pageInvestment->ui->buttonInv_calculateFee->setEnabled(false);
  m_feeRate = m_wiz->m_pageInvestment->ui->lineEdit_feeRate->text();
  if(m_feeRate.isEmpty()) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setEnabled(true);
    m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setEnabled(true);
    m_wiz->m_pageInvestment->ui->lineEdit_minFee->setEnabled(false);
  } else {
    m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setEnabled(false);
    m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setEnabled(false);
    m_wiz->m_pageInvestment->ui->checkBoxInv_feeIsPercentage->setChecked(true);
    m_wiz->m_pageInvestment->ui->lineEdit_minFee->setEnabled(true);
    if (m_colTypeNum.value(ColumnAmount) != -1)
      m_wiz->m_pageInvestment->ui->buttonInv_calculateFee->setEnabled(true);
  }
}

void InvestProcessing::fractionColumnChanged(int col)
{
  m_priceFraction = col;
  m_priceFractionValue = m_wiz->m_pageInvestment->ui->comboBoxInv_priceFraction->itemText(col);
}

bool InvestProcessing::validateMemoComboBox()
{
  if (m_wiz->m_memoColList.count() == 0)
    return true;
  for (int i = 0; i < m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->count(); i++)
  {
    QString txt = m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->itemText(i);
    if (txt.contains('*'))  // check if text containing '*' belongs to valid column types
      if (m_colNumType.value(i) != ColumnName &&
          m_colNumType.value(i) != ColumnType) {
        m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(i, QString().setNum(i + 1));
        m_wiz->m_memoColList.removeOne(i);
        return false;
      }
  }
  return true;
}

bool InvestProcessing::validateSelectedColumn(int col, columnTypeE type)
{
  if (m_colTypeNum.value(type) != -1)        // check if this 'type' has any column 'number' assigned...
    m_colNumType.remove(m_colTypeNum[type]); // ...if true remove 'type' assigned to this column 'number'

  if (col == -1) { // user only wanted to reset his column so allow him
    m_colTypeNum[type] = col;  // assign new column 'number' to this 'type'
    return true;
  }

  if (m_colNumType.contains(col)) { // if this column 'number' has already 'type' assigned
    KMessageBox::information(m_wiz, i18n("The '<b>%1</b>' field already has this column selected. <center>Please reselect both entries as necessary.</center>",
                                     m_colTypeName.value(m_colNumType[col])));
    resetComboBox(m_colNumType[col]);
    resetComboBox(type);
    return false;
  }

  m_colTypeNum[type] = col; // assign new column 'number' to this 'type'
  m_colNumType[col] = type; // assign new 'type' to this column 'number'
  return true;
}

void InvestProcessing::memoColumnSelected(int col)
{
  if (m_colNumType.value(col) == ColumnType || m_colNumType.value(col) == ColumnName) {
    int rc = KMessageBox::Yes;
    if (m_wiz->m_pageInvestment->isVisible())
      rc = KMessageBox::questionYesNo(0, i18n("<center>The '<b>%1</b>' field already has this column selected.</center>"
                                              "<center>If you wish to copy that data to the memo field, click 'Yes'.</center>",
                                              m_colTypeName.value(m_colNumType[col])));
    if (rc == KMessageBox::Yes) {
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1) + '*');
      if (!m_wiz->m_memoColList.contains(col))
        m_wiz->m_memoColList.append(col);
    } else {
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setItemText(col, QString().setNum(col + 1));
      m_wiz->m_memoColList.removeOne(col);
    }
    //allow only separate memo field occupy combobox
    m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->blockSignals(true);
    if (m_colTypeNum.value(ColumnMemo) != -1)
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(m_colTypeNum.value(ColumnMemo));
    else
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
    m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->blockSignals(false);
    return;
  }

  if (m_colTypeNum.value(ColumnMemo) != -1)        // check if this memo has any column 'number' assigned...
    m_wiz->m_memoColList.removeOne(col);           // ...if true remove it from memo list

  if(validateSelectedColumn(col, ColumnMemo))
    if (col != - 1 && !m_wiz->m_memoColList.contains(col))
      m_wiz->m_memoColList.append(col);
}

void InvestProcessing::dateColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnDate);
}

void InvestProcessing::feeColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnFee);
  feeInputsChanged();
}

void InvestProcessing::typeColumnSelected(int col)
{
  if (validateSelectedColumn(col, ColumnType))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      memoColumnSelected(col);    // ...if true set memo field again
}

void InvestProcessing::quantityColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnQuantity);
}

void InvestProcessing::priceColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnPrice);
}

void InvestProcessing::amountColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnAmount);
  feeInputsChanged();
}

void InvestProcessing::symbolColumnSelected(int col)
{
  validateSelectedColumn(col, ColumnSymbol);
}

void InvestProcessing::nameColumnSelected(int col)
{
  if (validateSelectedColumn(col, ColumnName))
    if (!validateMemoComboBox())  // user could have it already in memo so...
      memoColumnSelected(col);    // ...if true set memo field again
}

void InvestProcessing::feeIsPercentageCheckBoxClicked(bool checked)
{
  m_feeIsPercentage = checked;
}

void InvestProcessing::createStatement()
{
  m_wiz->st = MyMoneyStatement();
  m_wiz->st.m_eType = MyMoneyStatement::etInvestment;
  if (m_colTypeNum.value(ColumnFee) >= m_wiz->m_endColumn) // fee column has not been calculated so do it now
    calculateFee();

  for (int line = m_wiz->m_startLine - 1; line < m_wiz->m_endLine; line++) {
    if (!processInvestLine(m_wiz->m_lineList[line], m_wiz->st)) { // parse fields
      m_wiz->m_importNow = false;
      m_wiz->m_wizard->back();  //               have another try at the import
      break;
    }
  }  
  if (!m_wiz->m_importNow)
    return;

  QList<MyMoneyStatement::Security>::const_iterator it_s = m_listSecurities.constBegin();
  while (it_s != m_listSecurities.constEnd()) {
    m_wiz->st.m_listSecurities << (*it_s);
    ++it_s;
  }

  emit statementReady(m_wiz->st);  // investment statement ready
  m_wiz->m_importNow = false;
}

bool InvestProcessing::processInvestLine(const QString &line, MyMoneyStatement &st)
{
  MyMoneyStatement::Transaction tr;
  m_columnList = m_wiz->m_parse->parseLine(line); // split line into fields
  if (m_columnList.count() < m_wiz->m_endColumn) {
    if (!m_wiz->m_accept) {
      QString row = QString::number(m_wiz->m_row);
      int ret = KMessageBox::questionYesNoCancel(0, i18n("<center>Row number %1 does not have the expected number of columns.</center>"
                "<center>This might not be a problem, but it may be a header line.</center>"
                "<center>You may accept all similar items, or just this one, or cancel.</center>",
                row), i18n("CSV import"),
                KGuiItem(i18n("Accept All")),
                KGuiItem(i18n("Accept This")),
                KGuiItem(i18n("Cancel")));
      if (ret == KMessageBox::Cancel)
        return false;
      if (ret == KMessageBox::Yes)
        m_wiz->m_accept = true;
    }
  }

  int neededFieldsCount = 0;
  QString memo;
  QString txt;

  for (int i = 0; i < m_columnList.count(); i++) {
    m_columnList[i].trimmed().remove(m_wiz->m_textDelimiterCharacter);
  }

    // process date field
    if (m_colTypeNum.value(ColumnDate) != -1) {
      ++neededFieldsCount;
      txt = m_columnList[m_colTypeNum[ColumnDate]];
      tr.m_datePosted = m_wiz->m_convertDate->convertDate(txt);      //  Date column
      if (tr.m_datePosted == QDate()) {
        KMessageBox::sorry(m_wiz, i18n("<center>An invalid date has been detected during import.</center>"
                                       "<center><b>'%1'</b></center>"
                                       "Please check that you have set the correct date format,\n"
                                       "<center>and start and end lines.</center>"
                                       , txt), i18n("CSV import"));
        m_wiz->m_importError = true;
        return false;
      }
    }

    // process quantity field
    if (m_colTypeNum.value(ColumnQuantity) != -1) {
      ++neededFieldsCount;
      txt = m_columnList[m_colTypeNum[ColumnQuantity]];
      if (txt.isEmpty())
        txt = "0" + m_wiz->m_decimalSymbol + "00";

      txt.remove(QRegularExpression("+-")); // remove unwanted sings in quantity
      tr.m_shares = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(txt));
    }

    // process price field
    if (m_colTypeNum.value(ColumnPrice) != -1) {
      ++neededFieldsCount;
      txt = m_columnList[m_colTypeNum[ColumnPrice]];
      if (txt.isEmpty())
        txt = "0" + m_wiz->m_decimalSymbol + "00";

      MyMoneyMoney price = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(txt));
      price *= MyMoneyMoney(m_priceFractionValue);
      tr.m_price = price;
    }

    // process amount field
    if (m_colTypeNum.value(ColumnAmount) != -1) {
      ++neededFieldsCount;
      txt = m_columnList[m_colTypeNum[ColumnAmount]];
      if (txt.isEmpty())
        txt = "0" + m_wiz->m_decimalSymbol + "00";

      if (txt.startsWith('(') || txt.startsWith('[')) { // check if brackets notation is used for negative numbers
        txt.remove(QRegularExpression("[()]"));
        txt = '-' + txt;
      }

      tr.m_amount = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(txt));
    }

    // process type field
    if (m_colTypeNum.value(ColumnType) != -1) {
      ++neededFieldsCount;
      txt = m_columnList[m_colTypeNum[ColumnType]];
      tr.m_eAction = processActionType(txt);
      m_redefine->setColumnList(m_columnList);
      m_redefine->setColumnTypeNumber(m_colTypeNum);
      m_redefine->setColumnTypeName(m_colTypeName);
      if (!createValidActionTypes(m_validActionTypes, tr))
        return false;
      if (!validateActionType(tr.m_eAction, txt)) // action type not recognized and user didn't give input
        return false;
    }

    // process fee field
    if (m_colTypeNum.value(ColumnFee) != -1) {
      txt = m_columnList[m_colTypeNum[ColumnFee]];
      if (txt.isEmpty())
        txt = "0" + m_wiz->m_decimalSymbol + "00";

      if (txt.startsWith('(') || txt.startsWith('[')) // check if brackets notation is used for negative numbers
        txt.remove(QRegularExpression("[()]"));

      MyMoneyMoney fee = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(txt));
      if (m_feeIsPercentage && m_feeRate.isEmpty())      //   fee is percent
        fee *= tr.m_amount / MyMoneyMoney(100); // as percentage
      fee.abs();
      tr.m_fees = fee;
    }

    // process name field
    if (m_colTypeNum.value(ColumnName) != -1) {
      txt = m_columnList[m_colTypeNum[ColumnName]];
      if (!m_nameFilter.isEmpty()) {    //          If filter exists...
        QStringList list;
        list = txt.split(m_nameFilter);  //      ...split the name
        txt = list[1];
      } else
        tr.m_strSecurity = txt;
    } else if (!tr.m_strSymbol.isEmpty())
      tr.m_strSecurity = m_map.value(tr.m_strSymbol);
    else if (!m_securityName.isEmpty())
      tr.m_strSecurity = m_securityName;
    tr.m_strPayee = tr.m_strSecurity;

    // process symbol field
    if (m_colTypeNum.value(ColumnSymbol) != -1)
      tr.m_strSymbol = m_columnList[m_colTypeNum[ColumnSymbol]].toUpper();
    else if (!tr.m_strSecurity.isEmpty())
      tr.m_strSymbol = m_map.key(tr.m_strSecurity);


    // process memo field
    if (m_colTypeNum.value(ColumnMemo) != -1)
      memo += m_columnList[m_colTypeNum[ColumnMemo]];

    for (int i = 0; i < m_wiz->m_memoColList.count(); i++) {
      if (m_wiz->m_memoColList[i] != m_colTypeNum[ColumnMemo]) {
        if (!memo.isEmpty())
          memo += "\n";
        memo += m_columnList[m_wiz->m_memoColList[i]];
      }
    }
    tr.m_strMemo = memo;

    tr.m_strInterestCategory.clear(); // no special category
    tr.m_strBrokerageAccount.clear(); // no brokerage account auto-detection

  if (neededFieldsCount <= 3) {
    QString errMsg = i18n("<center>The columns selected are invalid.</center>"
                          "There must an amount or quantity fields, symbol or security name, plus date and type field.");
    if (m_wiz->m_skipSetup)
      errMsg += i18n("<center>You possibly need to check the start and end line settings, or reset 'Skip setup'.</center>");
    KMessageBox::sorry(m_wiz, errMsg);
    m_wiz->m_importError = true;
    return false;
  }

  MyMoneyStatement::Split s1;
  s1.m_amount = tr.m_amount;
  s1.m_strMemo = tr.m_strMemo;
  MyMoneyStatement::Split s2 = s1;
  s2.m_amount = MyMoneyMoney(-s1.m_amount);
  s2.m_accountId = m_wiz->m_csvUtil->checkCategory(tr.m_strInterestCategory, s1.m_amount, s2.m_amount);

  // deduct fees from amount
  if (tr.m_eAction == MyMoneyStatement::Transaction::eaCashDividend ||
      tr.m_eAction == MyMoneyStatement::Transaction::eaSell ||
      tr.m_eAction == MyMoneyStatement::Transaction::eaInterest)
    tr.m_amount = tr.m_amount - tr.m_fees;

  else if (tr.m_eAction == MyMoneyStatement::Transaction::eaBuy) {
    if (tr.m_amount.isPositive())
      tr.m_amount = -tr.m_amount; //if broker doesn't use minus sings for buy transactions, set it manually here
    tr.m_amount = tr.m_amount - tr.m_fees;
  } else if (tr.m_eAction == MyMoneyStatement::Transaction::eaNone)
    tr.m_listSplits += s2;

  st.m_listTransactions += tr; // Add the transaction to the statement
  return true;
}

bool InvestProcessing::createValidActionTypes(QList<MyMoneyStatement::Transaction::EAction> &validActionTypes, MyMoneyStatement::Transaction &tr)
{
  validActionTypes.clear();
  if (tr.m_shares.isPositive() &&
      tr.m_price.isPositive() &&
      !tr.m_amount.isZero())
    validActionTypes << MyMoneyStatement::Transaction::eaReinvestDividend <<
                        MyMoneyStatement::Transaction::eaBuy <<
                        MyMoneyStatement::Transaction::eaSell;
  else if (tr.m_shares.isZero() &&
           tr.m_price.isZero() &&
           !tr.m_amount.isZero())
    validActionTypes << MyMoneyStatement::Transaction::eaCashDividend <<
                        MyMoneyStatement::Transaction::eaInterest;
  else if (tr.m_shares.isPositive() &&
           tr.m_price.isZero() &&
           tr.m_amount.isZero())
    validActionTypes << MyMoneyStatement::Transaction::eaShrsin <<
                        MyMoneyStatement::Transaction::eaShrsout;
  else {
    KMessageBox::sorry(m_wiz, i18n("The values in the columns you have selected\ndo not match any expected investment type.\nPlease check the fields in the current transaction,\nand also your selections.")
                       , i18n("CSV import"));
    return false;
  }
  return true;
}

void InvestProcessing::storeActionType(MyMoneyStatement::Transaction::EAction &actionType, const QString &userType)
{
  switch(actionType) {
  case MyMoneyStatement::Transaction::eaBuy:
    m_buyList << userType;
    break;
  case MyMoneyStatement::Transaction::eaSell:
    m_sellList << userType;
    break;
  case MyMoneyStatement::Transaction::eaReinvestDividend:
    m_reinvdivList << userType;
    break;
  case MyMoneyStatement::Transaction::eaCashDividend:
    m_divXList << userType;
    break;
  case MyMoneyStatement::Transaction::eaInterest:
    m_intIncList << userType;
    break;
  case MyMoneyStatement::Transaction::eaShrsin:
    m_shrsinList << userType;
    break;
  case MyMoneyStatement::Transaction::eaShrsout:
    m_shrsoutList << userType;
    break;
  default:
    break;
  }
}

bool InvestProcessing::validateActionType(MyMoneyStatement::Transaction::EAction &actionType, const QString &userType)
{
  bool store = false;
  if (!m_validActionTypes.contains(actionType)) {
    QString info;
    if (actionType == MyMoneyStatement::Transaction::eaNone) {
      info = i18n("<center>The transaction below has an unrecognised type or action.</center>");
      store = true;
    }
    else
      info = i18n("<center>The transaction below has an invalid type or action.</center>");
    info += i18n("<center>Please select an appropriate entry, if available.</center>"
                 "<center>Otherwise, click Cancel to abort.</center>");
    m_redefine->setValidActionTypes(m_validActionTypes);
    actionType = m_redefine->askActionType(info);
  }

  if (actionType == MyMoneyStatement::Transaction::eaNone)  // user didn't point any transaction
    return false;
  if (store)
    storeActionType(actionType, userType);
  return true;
}

MyMoneyStatement::Transaction::EAction InvestProcessing::processActionType(QString& type)
{
  // most frequent action
  for (QStringList::const_iterator it = m_buyList.constBegin(); it != m_buyList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaBuy;

  // second most frequent action
  for (QStringList::const_iterator it = m_sellList.constBegin(); it != m_sellList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaSell;

  for (QStringList::const_iterator it = m_reinvdivList.constBegin(); it != m_reinvdivList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaReinvestDividend;

  // needs to be after reinvdiv
  for (QStringList::const_iterator it = m_divXList.constBegin(); it != m_divXList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaCashDividend;

  for (QStringList::const_iterator it = m_intIncList.constBegin(); it != m_intIncList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaInterest;

  for (QStringList::const_iterator it = m_shrsinList.constBegin(); it != m_shrsinList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaShrsin;

  for (QStringList::const_iterator it = m_shrsoutList.constBegin(); it != m_shrsoutList.constEnd(); ++it)
    if (type.contains(*it, Qt::CaseInsensitive))
      return MyMoneyStatement::Transaction::eaShrsout;

  return MyMoneyStatement::Transaction::eaNone;
}

void InvestProcessing::slotImportClicked()
{
  m_wiz->m_importError = false;

  if (m_wiz->m_decimalSymbol.isEmpty()) {
    KMessageBox::sorry(0, i18n("<center>Please select the decimal symbol used in your file.\n</center>"), i18n("Investment import"));
    m_wiz->m_importError = true;
    return;
  }

  m_securityName = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  if (m_securityName.isEmpty())
    m_securityName = m_symbolTableDlg->m_securityName;

  if (m_securityName.isEmpty() &&
      m_colTypeNum.value(ColumnSymbol) < 1 &&
      m_colTypeNum.value(ColumnName) < 1) {
    KMessageBox::sorry(0, i18n("<center>Please enter a name or symbol for the security.\n</center>"), i18n("CSV import"));
    m_wiz->m_importError = true;
    return;
  }

  if (!m_securityList.contains(m_securityName)){
    m_securityList << m_securityName;
  }

    //  all necessary data is present

    m_wiz->m_endLine = m_wiz->m_pageLinesDate->ui->spinBox_skipToLast->value();
    int skp = m_wiz->m_pageLinesDate->ui->spinBox_skip->value(); //         skip all headers
    if (skp > m_wiz->m_endLine) {
      KMessageBox::sorry(0, i18n("<center>The start line is greater than the end line.\n</center>"
                                 "<center>Please correct your settings.</center>"), i18n("CSV import"));
      m_wiz->m_importError = true;
      return;
    }

    createStatement();
}

void InvestProcessing::saveAs()
{
    QStringList outFile = m_wiz->m_inFileName .split('.');
    const QString &name = QString((outFile.isEmpty() ? "InvestProcessing" : outFile[0]) + ".qif");

    QString outFileName = QFileDialog::getSaveFileName(0, i18n("Save QIF"), name, QString::fromLatin1("*.qif | %1").arg(i18n("QIF Files")));
    QFile oFile(outFileName);
    oFile.open(QIODevice::WriteOnly);
    QTextStream out(&oFile);
    oFile.close();
}

void InvestProcessing::readSettings(const KSharedConfigPtr& config)
{
  KConfigGroup securitiesGroup(config, "Securities");
  m_securityList.clear();
  m_listSecurities.clear();
  m_symbolTableScanned = false;
  m_securityList = securitiesGroup.readEntry("SecurityNameList", QStringList());
  for (int i = 0; i < m_wiz->m_profileList.count(); i++) {
    if (m_wiz->m_profileList[i] != m_wiz->m_profileName)
      continue;
    KConfigGroup profilesGroup(config, "Invest-" + m_wiz->m_profileList[i]);

    m_wiz->m_inFileName = profilesGroup.readEntry("Directory", QString());

    QStringList list = profilesGroup.readEntry("BuyParam", QStringList());
    if (!list.isEmpty())
      m_buyList = list;
    list = profilesGroup.readEntry("ShrsinParam", QStringList());
    if (!list.isEmpty())
      m_shrsinList = list;
    list = profilesGroup.readEntry("DivXParam", QStringList());
    if (!list.isEmpty())
      m_divXList = list;
    list = profilesGroup.readEntry("IntIncParam", QStringList());
    if (!list.isEmpty())
      m_intIncList = list;
    list = profilesGroup.readEntry("BrokerageParam", QStringList());
    if (!list.isEmpty())
      m_brokerageList = list;
    list = profilesGroup.readEntry("ReinvdivParam", QStringList());
    if (!list.isEmpty())
      m_reinvdivList = list;
    list = profilesGroup.readEntry("SellParam", QStringList());
    if (!list.isEmpty())
      m_sellList = list;
    list = profilesGroup.readEntry("RemoveParam", QStringList());
    if (!list.isEmpty())
      m_shrsoutList = list;

    m_securityNameIndex = profilesGroup.readEntry("SecurityName", -1);
    m_nameFilter = profilesGroup.readEntry("Filter", QString());
    if (m_nameFilter.endsWith('#'))      //  Terminates a trailing blank
      m_nameFilter.chop(1);

    m_priceFraction = profilesGroup.readEntry("PriceFraction", 0);
    m_colTypeNum[ColumnDate] = profilesGroup.readEntry("DateCol", -1);
    m_colTypeNum[ColumnType] = profilesGroup.readEntry("PayeeCol", -1);  //use for type col.
    m_colTypeNum[ColumnPrice] = profilesGroup.readEntry("PriceCol", -1);
    m_colTypeNum[ColumnQuantity] = profilesGroup.readEntry("QuantityCol", -1);
    m_colTypeNum[ColumnAmount] = profilesGroup.readEntry("AmountCol", -1);
    m_colTypeNum[ColumnName] = profilesGroup.readEntry("NameCol", -1);
    m_colTypeNum[ColumnFee] = profilesGroup.readEntry("FeeCol", -1);
    m_colTypeNum[ColumnSymbol] = profilesGroup.readEntry("SymbolCol", -1);
    m_colTypeNum[ColumnMemo] = -1; // initialize, otherwise random data may go here
    m_feeIsPercentage = profilesGroup.readEntry("FeeIsPercentage", 0);
    m_feeRate = profilesGroup.readEntry("FeeRate", QString());
    m_minFee = profilesGroup.readEntry("MinFee", QString());

    m_wiz->m_memoColList = profilesGroup.readEntry("MemoCol", QList<int>());
    m_wiz->m_dateFormatIndex = profilesGroup.readEntry("DateFormat", -1);
    m_wiz->m_textDelimiterIndex = profilesGroup.readEntry("TextDelimiter", 0);
    m_wiz->m_fieldDelimiterIndex = profilesGroup.readEntry("FieldDelimiter", -1);
    m_wiz->m_decimalSymbolIndex = profilesGroup.readEntry("DecimalSymbol", -1);

    if (m_wiz->m_decimalSymbolIndex == -1) { // if no decimal symbol in config, then get one from locale settings
      if (QLocale().decimalPoint() == '.')
        m_wiz->m_decimalSymbolIndex = 0;
      else
        m_wiz->m_decimalSymbolIndex = 1;
    }
    if (m_wiz->m_decimalSymbolIndex == 0)
      m_wiz->m_ThousandsSeparatorIndex = 1;
    else
      m_wiz->m_ThousandsSeparatorIndex = 0;

    m_wiz->m_parse->setDecimalSymbolIndex(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_parse->setDecimalSymbol(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_parse->setThousandsSeparatorIndex(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_parse->setThousandsSeparator(m_wiz->m_decimalSymbolIndex);
    m_wiz->m_decimalSymbol = m_wiz->m_parse->decimalSymbol(m_wiz->m_decimalSymbolIndex);

    m_wiz->m_parse->setFieldDelimiterIndex(m_wiz->m_fieldDelimiterIndex);
    m_wiz->m_parse->setTextDelimiterIndex(m_wiz->m_textDelimiterIndex);
    m_wiz->m_fieldDelimiterCharacter = m_wiz->m_parse->fieldDelimiterCharacter(m_wiz->m_fieldDelimiterIndex);
    m_wiz->m_textDelimiterCharacter = m_wiz->m_parse->textDelimiterCharacter(m_wiz->m_textDelimiterIndex);
    m_wiz->m_startLine = profilesGroup.readEntry("StartLine", 0) + 1;
    m_wiz->m_pageLinesDate->m_trailerLines = profilesGroup.readEntry("TrailerLines", 0);
    m_wiz->m_encodeIndex = profilesGroup.readEntry("Encoding", 0);
    break;
  }
}

void InvestProcessing::resetComboBox(columnTypeE comboBox)
{
  switch (comboBox) {
    case ColumnAmount:
      m_wiz->m_pageInvestment->ui->comboBoxInv_amountCol->setCurrentIndex(-1);
      break;
    case ColumnDate:
      m_wiz->m_pageInvestment->ui->comboBoxInv_dateCol->setCurrentIndex(-1);
      break;
    case ColumnFee:
      m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(-1);
      break;
    case ColumnMemo:
      m_wiz->m_pageInvestment->ui->comboBoxInv_memoCol->setCurrentIndex(-1);
      break;
    case ColumnPrice:
      m_wiz->m_pageInvestment->ui->comboBoxInv_priceCol->setCurrentIndex(-1);
      break;
    case ColumnQuantity:
      m_wiz->m_pageInvestment->ui->comboBoxInv_quantityCol->setCurrentIndex(-1);
      break;
    case ColumnType:
      m_wiz->m_pageInvestment->ui->comboBoxInv_typeCol->setCurrentIndex(-1);
      break;
    case ColumnSymbol:
      m_wiz->m_pageInvestment->ui->comboBoxInv_symbolCol->setCurrentIndex(-1);
      break;
    case ColumnName:
      m_wiz->m_pageInvestment->ui->comboBoxInv_nameCol->setCurrentIndex(-1);
      break;
    default:
      KMessageBox::sorry(0, i18n("<center>Field name not recognised.</center><center>'<b>%1</b>'</center>Please re-enter your column selections.", comboBox), i18n("CSV import"));
  }
}

void  InvestProcessing::setSecurityName(QString name)
{
  m_securityName = name;
}

void InvestProcessing::slotNamesEdited()
{
  int row = 0;
  int symTableRow = -1;

  for (row = m_wiz->m_startLine - 1; row < m_wiz->m_endLine; row ++) {
    if (m_wiz->ui->tableWidget->item(row, m_colTypeNum.value(ColumnSymbol)) == 0) {  //  Item does not exist
      continue;
    }
    symTableRow++;
    if (m_wiz->ui->tableWidget->item(row, m_colTypeNum.value(ColumnSymbol))->text().trimmed().isEmpty()) {
      continue;
    }
    //  Replace detail with edited security name.
    QString securityName = m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 2)->text();
    if (m_colTypeNum.value(ColumnName) > -1)
      m_wiz->ui->tableWidget->item(row, m_colTypeNum.value(ColumnName))->setText(securityName);
    //  Replace symbol with edited symbol.
    QString securitySymbol = m_symbolTableDlg->m_widget->tableWidget->item(symTableRow, 0)->text();
    if (m_colTypeNum.value(ColumnSymbol) > -1)
      m_wiz->ui->tableWidget->item(row, m_colTypeNum.value(ColumnSymbol))->setText(securitySymbol);
    m_map.insert(securitySymbol, securityName);
  }

  emit isImportable();
}

void InvestProcessing::securityNameSelected(const QString& name)
{
  if ((m_securityList.contains(name)) || (name.isEmpty())) {
    return;
  }

  m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setInsertPolicy(QComboBox::InsertAlphabetically);
  m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setDuplicatesEnabled(false);
  m_securityName = name;
  m_securityList << name;
  m_securityList.removeDuplicates();
  m_securityList.sort();
}

void InvestProcessing::securityNameEdited()
{
  QString name = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  int index = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->findText(name);
  if ((index >= 0) || (name.isEmpty())) {
    return;
  }
  int rc = KMessageBox::warningContinueCancel(0, i18n("<center>Do you want to add a new security</center>\n"
           "<center>%1 </center>\n"
           "<center>to the selection list?</center>\n"
           "<center>Click \'Continue\' to add the name.</center>\n"
           "<center>Otherwise, click \'Cancel\'.</center>",
           name), i18n("Add Security Name"));
  if (rc == KMessageBox::Cancel) {
    m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->clearEditText();
    m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->setCurrentIndex(-1);
  } else {
    m_securityName = name;
    m_securityList << name;
    m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->addItem(name);
    m_securityList.removeDuplicates();
    m_securityList.sort();
  }
}

QStringList InvestProcessing::securityList()
{
  return m_securityList;
}

void InvestProcessing::calculateFee()
{
  QString txt;
  QString newTxt;
  MyMoneyMoney minFee;
  MyMoneyMoney percent;
  double d;
  bool ok;

  m_feeRate = m_wiz->m_pageInvestment->ui->lineEdit_feeRate->text();
  if (m_feeRate.isEmpty() || m_colTypeNum.value(ColumnAmount) == -1) //check if feeRate is in place
    return;

  percent = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(m_feeRate));

  m_minFee = m_wiz->m_pageInvestment->ui->lineEdit_minFee->text();
  if (m_minFee.isEmpty())
    m_minFee = "0" + m_wiz->m_decimalSymbol + "00";

  minFee = MyMoneyMoney(m_wiz->m_parse->possiblyReplaceSymbol(m_minFee));

  if (m_colTypeNum.value(ColumnFee) == -1) { // if fee column not present, add it at the end
    m_colNumType.insert(m_wiz->m_maxColumnCount, ColumnFee);
    m_colTypeNum.insert(ColumnFee, m_wiz->m_maxColumnCount);
  }

  if (m_colTypeNum.value(ColumnFee) >= m_wiz->m_maxColumnCount) // if fee column out of boudary, expand it
    m_wiz->m_maxColumnCount ++;

  m_wiz->ui->tableWidget->setColumnCount(m_wiz->m_maxColumnCount);
  txt.setNum(m_colTypeNum.value(ColumnFee) + 1);
  m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->addItem(txt); //add generated column to fee combobox...
  m_wiz->m_pageInvestment->ui->comboBoxInv_feeCol->setCurrentIndex(m_colTypeNum.value(ColumnFee)); // ...and select it by default

  for (int i = m_wiz->m_startLine - 1; i < m_wiz->m_endLine; i++)
    {
      m_columnList = m_wiz->m_parse->parseLine(m_wiz->m_lineList[i]);
      txt = m_columnList[m_colTypeNum.value(ColumnAmount)];
      txt.replace(QRegExp("[,. ]"),"").toInt(&ok);
      if (!ok) //if the line is in statement's header, skip
        {
          m_wiz->m_lineList[i] = m_wiz->m_lineList[i] + m_wiz->m_fieldDelimiterCharacter;
          continue;
        }
      txt = m_columnList[m_colTypeNum.value(ColumnAmount)];
      txt = txt.remove('"');
      if (txt.contains(')')) {
          txt = '-' + txt.remove(QRegExp("[()]"));   //            Mark as -ve
        }
      newTxt = m_wiz->m_parse->possiblyReplaceSymbol(txt);
      MyMoneyMoney amount = MyMoneyMoney(newTxt);
      MyMoneyMoney fee = percent * amount / MyMoneyMoney(100);
      if (fee < minFee)
        fee = minFee;
      d = fee.toDouble();
      txt.setNum(d, 'f', 4);
      txt.replace('.', m_wiz->m_decimalSymbol); //make sure decimal symbol is uniform in whole line

      if (m_wiz->m_decimalSymbol == m_wiz->m_fieldDelimiterCharacter) { //make sure fee has the same notation as the line it's being attached to
        if (m_columnList.count() == m_wiz->m_maxColumnCount)
          m_wiz->m_lineList[i] = m_wiz->m_lineList[i].left(m_wiz->m_lineList[i].length() - txt.length() - 2 * m_wiz->m_textDelimiterCharacter.length() - m_wiz->m_fieldDelimiterCharacter.length());
        m_wiz->m_lineList[i] = m_wiz->m_lineList[i] + m_wiz->m_fieldDelimiterCharacter + m_wiz->m_textDelimiterCharacter + txt + m_wiz->m_textDelimiterCharacter;
      }
      else {
        if (m_columnList.count() == m_wiz->m_maxColumnCount)
          m_wiz->m_lineList[i] = m_wiz->m_lineList[i].left(m_wiz->m_lineList[i].length()-txt.length() - m_wiz->m_fieldDelimiterCharacter.length() );
        m_wiz->m_lineList[i] = m_wiz->m_lineList[i] + m_wiz->m_fieldDelimiterCharacter + txt;
      }

      QTableWidgetItem *item = new QTableWidgetItem;
      item->setText(txt + "  ");
      m_wiz->ui->tableWidget->setItem(i, m_colTypeNum.value(ColumnFee), item);
    }
    m_wiz->updateWindowSize();
}

void InvestProcessing::hideSecurity()
{
  QString name = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentText();
  if (name.isEmpty()) {
    return;
  }
  int rc = KMessageBox::warningContinueCancel(0, i18n("<center>You have selected to remove from the selection list</center>\n"
           "<center>%1. </center>\n"
           "<center>Click \'Continue\' to remove the name, or</center>\n"
           "<center>Click \'Cancel\'' to leave 'as is'.</center>",
           name), i18n("Hide Security Name"));
  if (rc == KMessageBox::Continue) {
    int index = m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->currentIndex();
    m_wiz->m_pageInvestment->ui->comboBoxInv_securityName->removeItem(index);
    m_securityList.removeAt(index);
    m_securityName.clear();
  }
}
