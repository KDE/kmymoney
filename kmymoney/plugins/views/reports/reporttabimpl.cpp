/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018 Michael Kiefer <Michael-Kiefer@web.de>
    SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reporttabimpl.h"

#include <KLocalizedString>
#include <QtMath>

#include "kmymoneyutils.h"
#include "daterangedlg.h"

#include "ui_reporttabgeneral.h"
#include "ui_reporttabrowcolpivot.h"
#include "ui_reporttabrowcolquery.h"
#include "ui_reporttabchart.h"
#include "ui_reporttabrange.h"
#include "ui_reporttabcapitalgain.h"
#include "ui_reporttabperformance.h"

#include "mymoney/mymoneyreport.h"
#include "mymoneyenums.h"

ReportTabGeneral::ReportTabGeneral(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabGeneral;
    ui->setupUi(this);
}

ReportTabGeneral::~ReportTabGeneral()
{
    delete ui;
}

ReportTabRowColPivot::ReportTabRowColPivot(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabRowColPivot;
    ui->setupUi(this);
}

ReportTabRowColPivot::~ReportTabRowColPivot()
{
    delete ui;
}

ReportTabRowColQuery::ReportTabRowColQuery(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabRowColQuery;
    ui->setupUi(this);
    ui->buttonGroup1->setExclusive(false);
    ui->buttonGroup1->setId(ui->m_checkMemo, 0);
    ui->buttonGroup1->setId(ui->m_checkShares, 1);
    ui->buttonGroup1->setId(ui->m_checkPrice, 2);
    ui->buttonGroup1->setId(ui->m_checkReconciled, 3);
    ui->buttonGroup1->setId(ui->m_checkAccount, 4);
    ui->buttonGroup1->setId(ui->m_checkNumber, 5);
    ui->buttonGroup1->setId(ui->m_checkPayee, 6);
    ui->buttonGroup1->setId(ui->m_checkCategory, 7);
    ui->buttonGroup1->setId(ui->m_checkAction, 8);
    ui->buttonGroup1->setId(ui->m_checkBalance, 9);
    connect(ui->m_checkHideTransactions, &QAbstractButton::toggled, this, &ReportTabRowColQuery::slotHideTransactionsChanged);
}

void ReportTabRowColQuery::slotHideTransactionsChanged(bool checked)
{
    if (checked)                                          // toggle m_checkHideSplitDetails only if it's mandatory
        ui->m_checkHideSplitDetails->setChecked(checked);
    ui->m_checkHideSplitDetails->setEnabled(!checked);    // hiding transactions without hiding splits isn't allowed
}

ReportTabRowColQuery::~ReportTabRowColQuery()
{
    delete ui;
}

ReportTabChart::ReportTabChart(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabChart;
    ui->setupUi(this);

    ui->m_comboType->addItem(i18nc("type of graphic chart", "Line"), static_cast<int>(eMyMoney::Report::ChartType::Line));
    ui->m_comboType->addItem(i18nc("type of graphic chart", "Bar"), static_cast<int>(eMyMoney::Report::ChartType::Bar));
    ui->m_comboType->addItem(i18nc("type of graphic chart", "Stacked Bar"), static_cast<int>(eMyMoney::Report::ChartType::StackedBar));
    ui->m_comboType->addItem(i18nc("type of graphic chart", "Pie"), static_cast<int>(eMyMoney::Report::ChartType::Pie));
    ui->m_comboType->addItem(i18nc("type of graphic chart", "Ring"), static_cast<int>(eMyMoney::Report::ChartType::Ring));
    connect(ui->m_comboType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ReportTabChart::slotChartTypeChanged);
    Q_EMIT ui->m_comboType->currentIndexChanged(ui->m_comboType->currentIndex());

    ui->m_comboPalette->addItem(i18nc("type of graphic palette", "Use application setting"), static_cast<int>(eMyMoney::Report::ChartPalette::Application));
    ui->m_comboPalette->addItem(i18nc("type of graphic palette", "Default"), static_cast<int>(eMyMoney::Report::ChartPalette::Default));
    ui->m_comboPalette->addItem(i18nc("type of graphic palette", "Rainbow"), static_cast<int>(eMyMoney::Report::ChartPalette::Rainbow));
    ui->m_comboPalette->addItem(i18nc("type of graphic palette", "Subdued"), static_cast<int>(eMyMoney::Report::ChartPalette::Subdued));
}

ReportTabChart::~ReportTabChart()
{
    delete ui;
}

void ReportTabChart::slotChartTypeChanged(int index)
{
    if (index == static_cast<int>(eMyMoney::Report::ChartType::Pie) ||
            index == static_cast<int>(eMyMoney::Report::ChartType::Ring)) {
        ui->m_checkCHGridLines->setText(i18n("Show circular grid lines"));
        ui->m_checkSVGridLines->setText(i18n("Show sagittal grid lines"));
        ui->m_logYaxis->setChecked(false);
        ui->m_logYaxis->setEnabled(false);
        ui->m_negExpenses->setChecked(false);
        ui->m_negExpenses->setEnabled(false);
    } else {
        ui->m_checkCHGridLines->setText(i18n("Show horizontal grid lines"));
        ui->m_checkSVGridLines->setText(i18n("Show vertical grid lines"));
        ui->m_logYaxis->setEnabled(true);
        ui->m_negExpenses->setEnabled(true);
    }
}

void ReportTabChart::setNegExpenses(bool set)
{
    // logarithm on negative numbers does not make sense, so disable it
    if (set) {
        ui->m_logYaxis->setChecked(false);
        ui->m_logYaxis->setEnabled(false);
    } else {
        ui->m_logYaxis->setEnabled(true);
    }
}

ReportTabRange::ReportTabRange(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ReportTabRange),
      m_logYaxis(false)
{
    ui->setupUi(this);
    m_dateRange = new DateRangeDlg;
    ui->dateRangeGrid->addWidget(m_dateRange, 0, 0, 1, 2);
    connect(ui->m_yLabelsPrecision, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ReportTabRange::slotYLabelsPrecisionChanged);
    Q_EMIT ui->m_yLabelsPrecision->valueChanged(ui->m_yLabelsPrecision->value());
    connect(ui->m_dataRangeStart, &QLineEdit::editingFinished, this, &ReportTabRange::slotEditingFinishedStart);
    connect(ui->m_dataRangeEnd, &QLineEdit::editingFinished, this, &ReportTabRange::slotEditingFinishedEnd);
    connect(ui->m_dataMajorTick, &QLineEdit::editingFinished, this, &ReportTabRange::slotEditingFinishedMajor);
    connect(ui->m_dataMinorTick, &QLineEdit::editingFinished, this, &ReportTabRange::slotEditingFinishedMinor);
    connect(ui->m_dataLock, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ReportTabRange::slotDataLockChanged);
    Q_EMIT ui->m_dataLock->currentIndexChanged(ui->m_dataLock->currentIndex());
}

ReportTabRange::~ReportTabRange()
{
    delete ui;
}

void ReportTabRange::setRangeLogarythmic(bool set)
{
    // major and minor tick have no influence if axis is logarithmic so hide them
    if (set) {
        ui->lblDataMajorTick->hide();
        ui->lblDataMinorTick->hide();
        ui->m_dataMajorTick->hide();
        ui->m_dataMinorTick->hide();

        m_logYaxis = true;
    } else {
        ui->lblDataMajorTick->show();
        ui->lblDataMinorTick->show();
        ui->m_dataMajorTick->show();
        ui->m_dataMinorTick->show();

        m_logYaxis = false;
    }

    updateDataRangeValidators(ui->m_yLabelsPrecision->value()); // update data range validators and re-validate
}

void ReportTabRange::updateDataRangeValidators(const int& precision)
{

    const QValidator *dbValStart = ui->m_dataRangeStart->validator();
    const QValidator *dbValEnd = ui->m_dataRangeEnd->validator();

    delete dbValStart;
    if (dbValStart != dbValEnd) {
        delete dbValEnd;
    }

    if (m_logYaxis) {
        dbValStart = new MyLogarithmicDoubleValidator(precision, qPow(10, -precision), ui->m_dataRangeStart);
        dbValEnd = new MyLogarithmicDoubleValidator(precision, qPow(10, -precision + 4), ui->m_dataRangeEnd);
    } else { // the validator will be used by two QLineEdit objects so let this tab be their parent
        dbValStart = new MyDoubleValidator(precision, this);
        dbValEnd = dbValStart;
    }

    ui->m_dataRangeStart->setValidator(dbValStart);
    ui->m_dataRangeEnd->setValidator(dbValEnd);

    QString dataRangeStart = ui->m_dataRangeStart->text();
    QString dataRangeEnd = ui->m_dataRangeEnd->text();
    if (!ui->m_dataRangeStart->hasAcceptableInput()) {
        dbValStart->fixup(dataRangeStart);
        ui->m_dataRangeStart->setText(dataRangeStart);
    }
    if (ui->m_dataRangeEnd->hasAcceptableInput()) {
        dbValEnd->fixup(dataRangeEnd);
        ui->m_dataRangeEnd->setText(dataRangeEnd);
    }
}

void ReportTabRange::slotEditingFinished(EDimension dim)
{
    qreal dataRangeStart = locale().toDouble(ui->m_dataRangeStart->text());
    qreal dataRangeEnd = locale().toDouble(ui->m_dataRangeEnd->text());

    if (dataRangeEnd < dataRangeStart) { // end must be higher than start
        if (dim == eRangeEnd) {
            ui->m_dataRangeStart->setText(ui->m_dataRangeEnd->text());
            dataRangeStart = dataRangeEnd;
        } else {
            ui->m_dataRangeEnd->setText(ui->m_dataRangeStart->text());
            dataRangeEnd = dataRangeStart;
        }
    }
    if (!m_logYaxis) { // major and minor ticks only have influence when axis is linear
        qreal dataMajorTick = locale().toDouble(ui->m_dataMajorTick->text());
        qreal dataMinorTick = locale().toDouble(ui->m_dataMinorTick->text());
        if ((dataRangeStart != 0 || dataRangeEnd != 0)) { // if data range isn't going to be reset
            if ((dataRangeEnd - dataRangeStart) < dataMajorTick) // major tick cannot be greater than data range
                dataMajorTick = dataRangeEnd - dataRangeStart;

            if (dataMajorTick != 0 && // if major tick isn't going to be reset
                    dataMajorTick < (dataRangeEnd - dataRangeStart) * 0.01) // constraint major tick to be greater or equal to 1% of data range
                dataMajorTick = (dataRangeEnd - dataRangeStart) * 0.01;

            //set precision of major tick to be greater by 1
            ui->m_dataMajorTick->setText(locale().toString(dataMajorTick, 'f', ui->m_yLabelsPrecision->value() + 1).remove(locale().groupSeparator()).remove(QRegularExpression("0+$")).remove(QRegularExpression("\\" + locale().decimalPoint() + "$")));
        }

        if (dataMajorTick < dataMinorTick) { // major tick must be higher than minor
            if (dim == eMinorTick) {
                ui->m_dataMajorTick->setText(ui->m_dataMinorTick->text());
                dataMajorTick = dataMinorTick;
            } else {
                ui->m_dataMinorTick->setText(ui->m_dataMajorTick->text());
                dataMinorTick = dataMajorTick;
            }
        }

        if (dataMinorTick < dataMajorTick * 0.1) { // constraint minor tick to be greater or equal to 10% of major tick, and set precision to be greater by 1
            dataMinorTick = dataMajorTick * 0.1;
            ui->m_dataMinorTick->setText(locale().toString(dataMinorTick, 'f', ui->m_yLabelsPrecision->value() + 1).remove(locale().groupSeparator()).remove(QRegularExpression("0+$")).remove(QRegularExpression("\\" + locale().decimalPoint() + "$")));
        }
    }
}

void ReportTabRange::slotEditingFinishedStart()
{
    slotEditingFinished(eRangeStart);
}

void ReportTabRange::slotEditingFinishedEnd()
{
    slotEditingFinished(eRangeEnd);
}

void ReportTabRange::slotEditingFinishedMajor()
{
    slotEditingFinished(eMajorTick);
}

void ReportTabRange::slotEditingFinishedMinor()
{
    slotEditingFinished(eMinorTick);
}

void ReportTabRange::slotYLabelsPrecisionChanged(const int& value)
{
    ui->m_dataMajorTick->setValidator(0);
    ui->m_dataMinorTick->setValidator(0);

    MyDoubleValidator *dblVal2 = new MyDoubleValidator(value + 1);
    ui->m_dataMajorTick->setValidator(dblVal2);
    ui->m_dataMinorTick->setValidator(dblVal2);

    updateDataRangeValidators(value);
}

void ReportTabRange::slotDataLockChanged(int index) {
    if (index == static_cast<int>(eMyMoney::Report::DataLock::Automatic)) {
        ui->m_dataRangeStart->setText(QStringLiteral("0"));
        ui->m_dataRangeEnd->setText(QStringLiteral("0"));
        ui->m_dataMajorTick->setText(QStringLiteral("0"));
        ui->m_dataMinorTick->setText(QStringLiteral("0"));
        ui->m_dataRangeStart->setEnabled(false);
        ui->m_dataRangeEnd->setEnabled(false);
        ui->m_dataMajorTick->setEnabled(false);
        ui->m_dataMinorTick->setEnabled(false);
    } else {
        ui->m_dataRangeStart->setEnabled(true);
        ui->m_dataRangeEnd->setEnabled(true);
        ui->m_dataMajorTick->setEnabled(true);
        ui->m_dataMinorTick->setEnabled(true);
    }
}

ReportTabCapitalGain::ReportTabCapitalGain(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabCapitalGain;
    ui->setupUi(this);
    connect(ui->m_investmentSum, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ReportTabCapitalGain::slotInvestmentSumChanged);
}

ReportTabCapitalGain::~ReportTabCapitalGain()
{
    delete ui;
}

void ReportTabCapitalGain::slotInvestmentSumChanged(int index) {
    Q_UNUSED(index);
    if (ui->m_investmentSum->currentData() == static_cast<int>(eMyMoney::Report::InvestmentSum::Owned)) {
        ui->m_settlementPeriod->setValue(0);
        ui->m_settlementPeriod->setEnabled(false);
        ui->m_showSTLTCapitalGains->setChecked(false);
        ui->m_showSTLTCapitalGains->setEnabled(false);
        ui->m_termSeparator->setEnabled(false);
    } else {
        ui->m_settlementPeriod->setEnabled(true);
        ui->m_showSTLTCapitalGains->setEnabled(true);
        ui->m_termSeparator->setEnabled(true);
    }
}

ReportTabPerformance::ReportTabPerformance(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::ReportTabPerformance;
    ui->setupUi(this);
}

ReportTabPerformance::~ReportTabPerformance()
{
    delete ui;
}

MyDoubleValidator::MyDoubleValidator(int decimals, QObject * parent) :
    QDoubleValidator(0, 0, decimals, parent)
{
}

QValidator::State MyDoubleValidator::validate(QString &s, int &i) const
{
    Q_UNUSED(i);
    if (s.isEmpty() || s == "-") {
        return QValidator::Intermediate;
    }

    QString decimalPoint = locale().decimalPoint();

    if(s.indexOf(decimalPoint) != -1) {
        int charsAfterPoint = s.length() - s.indexOf(decimalPoint) - 1;

        if (charsAfterPoint > decimals()) {
            return QValidator::Invalid;
        }
    }

    bool ok;
    locale().toDouble(s, &ok);

    if (ok) {
        return QValidator::Acceptable;
    } else {
        return QValidator::Invalid;
    }
}

MyLogarithmicDoubleValidator::MyLogarithmicDoubleValidator(const int decimals, const qreal defaultValue, QObject *parent)
    : QDoubleValidator(qPow(10, -decimals), 0, decimals, parent)
{
    m_defaultText = KMyMoneyUtils::normalizeNumericString(defaultValue, locale(), 'f', decimals);
}

QValidator::State MyLogarithmicDoubleValidator::validate(QString &s, int &i) const
{
    Q_UNUSED(i);
    if (s.isEmpty() || s == QStringLiteral("0")) {
        return QValidator::Intermediate;
    }

    QString decimalPoint = locale().decimalPoint();

    // start numbering placeholders with a two-digit number to avoid
    // interpreting the following zero as part of the placeholder index
    const QRegularExpression re((QStringLiteral("^0\\%110{0,%12}$")
                                 .arg(decimalPoint)
                                 .arg(decimals() - 1)));
    if (re.match(s).hasMatch())
        return QValidator::Intermediate;

    if (s.indexOf(decimalPoint) != -1) {
        int charsAfterPoint = s.length() - s.indexOf(decimalPoint) - 1;

        if (charsAfterPoint > decimals()) {
            return QValidator::Invalid;
        }
    }

    bool ok;
    const qreal result = locale().toDouble(s, &ok);

    if (ok && result >= bottom()) {
        return QValidator::Acceptable;
    } else {
        return QValidator::Invalid;
    }
}

void MyLogarithmicDoubleValidator::fixup(QString &input) const
{
    input = m_defaultText;
}
