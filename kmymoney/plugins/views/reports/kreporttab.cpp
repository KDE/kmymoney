
#include "kreporttab.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTabWidget>
#include <QVBoxLayout>
#include <kmm_codec.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_printer.h"
#include "kmmtextbrowser.h"
#include "kreportchartview.h"
#include "kreportsview.h"
#include "kreportsview_p.h"
#include "mymoneyexception.h"
#include "reportcontrolimpl.h"

/**
 * KReportTab Implementation
 */
KReportTab::KReportTab(QTabWidget* parent, const MyMoneyReport& report, const KReportsView* eventHandler, KReportsView::OpenOption openOption)
    : QWidget(parent)
    , m_tableView(new KMMTextBrowser(this))
    , m_chartView(new reports::KReportChartView(this))
    , m_control(new ReportControl(this))
    , m_layout(new QVBoxLayout(this))
    , m_report(report)
    , m_deleteMe(false)
    , m_chartEnabled(false)
    , m_showingChart(report.isChartByDefault())
    , m_needReload(openOption == KReportsView::OpenImmediately)
    , m_isChartViewValid(false)
    , m_isTableViewValid(false)
    , m_table(nullptr)
{
    m_layout->setSpacing(6);
    // TODO
    //    m_tableView->setZoomFactor(KMyMoneySettings::zoomFactor());

    // set button icons
    m_control->ui->buttonChart->setIcon(Icons::get(Icon::OfficeCharBar));
    m_control->ui->buttonConfigure->setIcon(Icons::get(Icon::Configure));
    m_control->ui->buttonDelete->setIcon(Icons::get(Icon::EditRemove));
    m_control->ui->buttonExport->setIcon(Icons::get(Icon::DocumentExport));
    m_control->ui->buttonNew->setIcon(Icons::get(Icon::DocumentNew));

    // and actions
    m_control->ui->buttonConfigure->setDefaultAction(pActions[eMenu::Action::ReportConfigure]);
    m_control->ui->buttonDelete->setDefaultAction(pActions[eMenu::Action::ReportDelete]);
    m_control->ui->buttonExport->setDefaultAction(pActions[eMenu::Action::ReportExport]);
    m_control->ui->buttonNew->setDefaultAction(pActions[eMenu::Action::ReportNew]);

    m_chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_chartView->hide();
    m_tableView->hide();
    m_tableView->setOpenLinks(false);

    m_layout->addWidget(m_control);
    m_layout->addWidget(m_tableView);
    m_layout->addWidget(m_chartView);
    m_layout->setStretch(1, 10);
    m_layout->setStretch(2, 10);

    connect(m_control->ui->buttonChart, &QAbstractButton::clicked, eventHandler, &KReportsView::slotToggleChart);

    // setup style for all buttons to contain icon and text
    const auto buttons = m_control->findChildren<QToolButton*>();
    for (auto* button : qAsConst(buttons)) {
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    enableAllReportActions();

    connect(m_tableView, &KMMTextBrowser::anchorClicked, eventHandler, &KReportsView::slotOpenUrl);

    // if this is a default report, then you can't delete it!
    if (report.id().isEmpty())
        m_control->ui->buttonDelete->setEnabled(false);

    int tabNr = parent->addTab(this, Icons::get(Icon::Report), report.name());
    parent->setTabEnabled(tabNr, true);
    parent->setCurrentIndex(tabNr);

    // get users character set encoding
    m_encoding = KMM_Codec::encodingForLocale();
}

KReportTab::~KReportTab()
{
    delete m_table;
}

void KReportTab::wheelEvent(QWheelEvent* event)
{
    // Zoom text on Ctrl + Scroll
    if (event->modifiers() & Qt::CTRL) {
        if (!m_showingChart) {
            // TODO
            //            qreal factor = m_tableView->zoomFactor();
            if (event->angleDelta().y() > 0)
                //                factor += 0.1;
                //            else if (event->delta() < 0)
                //                factor -= 0.1;
                //            m_tableView->setZoomFactor(factor);
                event->accept();
            return;
        }
    }
}

void KReportTab::print()
{
    if (m_tableView) {
        auto printer = KMyMoneyPrinter::startPrint();
        if (printer != nullptr) {
            if (m_showingChart) {
                QPainter painter(printer);
                m_chartView->paint(&painter, painter.window());
                QFont font = painter.font();
                font.setPointSizeF(font.pointSizeF() * 0.8);
                painter.setFont(font);
                painter.drawText(0, 0, MyMoneyUtils::formatDate(QDate::currentDate()));

                /// @todo extract url from KMyMoneyApp
                QUrl file;
                if (file.isValid()) {
                    painter.drawText(0, painter.window().height(), file.toLocalFile());
                }
            } else {
                m_tableView->print(printer);
            }
        }
    }
}

void KReportTab::printPreview()
{
    if (m_showingChart) {
        if (m_chartView) {
            QPrintPreviewDialog dlg(KMyMoneyPrinter::instance(), m_chartView);
            connect(&dlg, &QPrintPreviewDialog::paintRequested, m_tableView, [&](QPrinter* printer) {
                QPainter painter(printer);
                m_chartView->paint(&painter, painter.window());
            });
            dlg.exec();
        }
    } else {
        if (m_tableView) {
            QPrintPreviewDialog dlg(KMyMoneyPrinter::instance(), m_tableView);
            connect(&dlg, &QPrintPreviewDialog::paintRequested, m_tableView, [&](QPrinter* printer) {
                m_tableView->print(printer);
            });
            dlg.exec();
        }
    }
}

void KReportTab::saveAs(const QString& filename, const QString& selectedMimeType)
{
    if (selectedMimeType == QStringLiteral("application/pdf")) {
        auto printer = KMyMoneyPDFPrinter::startPrint(m_report.name());
        if (printer != nullptr) {
            printer->setOutputFileName(filename);
            m_tableView->print(printer);
        }
        return;
    }

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        if (selectedMimeType == QStringLiteral("text/csv")) {
            QTextStream(&file) << m_table->renderReport(QLatin1String("csv"), m_encoding, QString());
        } else if (selectedMimeType == QStringLiteral("text/html")) {
            QString table = m_table->renderReport(QLatin1String("html"), m_encoding, m_report.name());
            // remove the background information only needed by Qt
            QRegularExpression removeBackgroundExp(R"(\n\s+background: .+bg-texture.png.+fixed;)");
            table.replace(removeBackgroundExp, QString());
            QTextStream stream(&file);
            stream << table;
        } else if (selectedMimeType == QStringLiteral("application/xml")) {
            QTextStream(&file) << m_table->toXml();
        }
        file.close();
    }
}

void KReportTab::loadTab()
{
    m_needReload = true;
    if (isVisible()) {
        m_needReload = false;
        updateReport();
    }
}

void KReportTab::showEvent(QShowEvent* event)
{
    if (m_needReload) {
        m_needReload = false;
        updateReport();
    }
    QWidget::showEvent(event);
}

void KReportTab::updateReport()
{
    m_isChartViewValid = false;
    m_isTableViewValid = false;
    // reload the report from the engine. It might have
    // been changed by the user

    try {
        // Don't try to reload default reports from the engine
        if (!m_report.id().isEmpty())
            m_report = MyMoneyFile::instance()->report(m_report.id());
    } catch (const MyMoneyException&) {
    }

    delete m_table;
    m_table = nullptr;

    if (m_report.reportType() == eMyMoney::Report::ReportType::PivotTable) {
        m_table = new PivotTable(m_report, KMyMoneyUtils::forecastConfig());
        m_chartEnabled = true;
    } else if (m_report.reportType() == eMyMoney::Report::ReportType::QueryTable) {
        m_table = new QueryTable(m_report);
        m_chartEnabled = false;
    } else if (m_report.reportType() == eMyMoney::Report::ReportType::InfoTable) {
        m_table = new ObjectInfoTable(m_report);
        m_chartEnabled = false;
    }

    m_control->ui->buttonChart->setEnabled(m_chartEnabled);

    m_showingChart = !m_showingChart;
    toggleChart();
}

void KReportTab::enableAllReportActions()
{
    pActions[eMenu::Action::ReportNew]->setEnabled(true);
    pActions[eMenu::Action::ReportConfigure]->setEnabled(true);
    pActions[eMenu::Action::ReportExport]->setEnabled(true);
    pActions[eMenu::Action::ReportDelete]->setEnabled(true);
    pActions[eMenu::Action::ReportClose]->setEnabled(true);
}

void KReportTab::toggleChart()
{
    // for now it will just SHOW the chart.  In the future it actually has to toggle it.

    if (m_showingChart) {
        if (!m_isTableViewValid) {
            m_tableView->setHtml(m_table->renderReport(QLatin1String("html"), m_encoding, m_report.name()));
        }
        m_isTableViewValid = true;
        m_tableView->show();
        m_chartView->hide();

        m_control->ui->buttonChart->setText(i18n("Chart"));
        m_control->ui->buttonChart->setToolTip(i18n("Show the chart version of this report"));
        m_control->ui->buttonChart->setIcon(Icons::get(Icon::OfficeCharBar));
    } else {
        if (!m_isChartViewValid)
            m_table->drawChart(*m_chartView);
        m_isChartViewValid = true;
        m_tableView->hide();
        m_chartView->show();

        m_control->ui->buttonChart->setText(i18n("Report"));
        m_control->ui->buttonChart->setToolTip(i18n("Show the report version of this chart"));
        m_control->ui->buttonChart->setIcon(Icons::get(Icon::Report));
    }
    m_showingChart = !m_showingChart;
}

void KReportTab::updateDataRange()
{
    static const QRegularExpression trailingsZeroesRegEx("0+$");

    QList<DataDimension> grids = m_chartView->coordinatePlane()->gridDimensionsList(); // get dimensions of plotted graph
    if (grids.isEmpty())
        return;
    auto separator = locale().groupSeparator(); // QChar in Qt5, QString in Qt6
    auto decimalPoint = locale().decimalPoint(); // QChar in Qt5, QString in Qt6
    int precision = m_report.yLabelsPrecision();
    QList<QPair<QString, qreal>> dims; // create list of dimension values in string and qreal

    // get qreal values
    dims.append(qMakePair(QString(), grids.at(1).start));
    dims.append(qMakePair(QString(), grids.at(1).end));
    dims.append(qMakePair(QString(), grids.at(1).stepWidth));
    dims.append(qMakePair(QString(), grids.at(1).subStepWidth));

    // convert qreal values to string variables
    for (int i = 0; i < 4; ++i) {
        if (i > 2)
            ++precision;
        if (precision == 0)
            dims[i].first = locale().toString(qRound(dims.at(i).second));
        else
            dims[i].first = locale()
                                .toString(dims.at(i).second, 'f', precision)
                                .remove(separator)
                                .remove(trailingsZeroesRegEx)
                                .remove(QRegularExpression("\\" + decimalPoint + "$"));
    }

    // save string variables in report's data
    m_report.setDataRangeStart(dims.at(0).first);
    m_report.setDataRangeEnd(dims.at(1).first);
    m_report.setDataMajorTick(dims.at(2).first);
    m_report.setDataMinorTick(dims.at(3).first);
}
