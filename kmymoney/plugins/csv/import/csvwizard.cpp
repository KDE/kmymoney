/*
    SPDX-FileCopyrightText: 2015-2016 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2018-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "csvwizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractButton>
#include <QAction>
#include <QFileDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QScreen>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTextCodec>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "bankingwizardpage.h"
#include "core/convdate.h"
#include "core/csvimportercore.h"
#include "core/csvutil.h"
#include "csvimporter.h"
#include "icons.h"
#include "investmentwizardpage.h"
#include "mymoneyqifprofile.h"
#include "pluginsettings.h"
#include "priceswizardpage.h"

#include "ui_csvwizard.h"
#include "ui_introwizardpage.h"
#include "ui_separatorwizardpage.h"
#include "ui_rowswizardpage.h"
#include "ui_formatswizardpage.h"

using namespace Icons;

CSVWizard::CSVWizard(CSVImporter *plugin) :
    ui(new Ui::CSVWizard),
    m_skipSetup(false),
    m_plugin(plugin),
    m_imp(new CSVImporterCore),
    m_wiz(new QWizard)
{
    ui->setupUi(this);
    ui->tableView->setModel(m_imp->m_file->m_model);
    readWindowSize(CSVImporterCore::configFile());
    m_wiz->setWizardStyle(QWizard::ClassicStyle);
    ui->horizontalLayout->addWidget(m_wiz);
    m_curId = -1;
    m_lastId = -1;
    m_wiz->installEventFilter(this); // event filter for escape key presses

    m_wiz->button(QWizard::BackButton)->setIcon(Icons::get(Icon::ArrowLeft));
    m_wiz->button(QWizard::CancelButton)->setIcon(Icons::get(Icon::DialogCancel));
    m_wiz->button(QWizard::FinishButton)->setIcon(Icons::get(Icon::DocumentImport));
    m_wiz->button(QWizard::CustomButton1)->setIcon(Icons::get(Icon::DocumentOpen));
    m_wiz->button(QWizard::CustomButton2)->setIcon(Icons::get(Icon::DocumentExport));
    m_wiz->button(QWizard::NextButton)->setIcon(Icons::get(Icon::ArrowRight));

    m_pageIntro = new IntroPage(this, m_imp);
    m_wiz->setPage(PageIntro, m_pageIntro);

    m_pageSeparator = new SeparatorPage(this, m_imp);
    m_wiz->setPage(PageSeparator, m_pageSeparator);

    m_pageRows = new RowsPage(this, m_imp);
    m_wiz->setPage(PageRows, m_pageRows);

    m_pageFormats = new FormatsPage(this, m_imp);
    m_wiz->setPage(PageFormats, m_pageFormats);

    showStage();
    m_wiz->button(QWizard::CustomButton1)->setEnabled(false);

    m_stageLabels << ui->label_intro << ui->label_separators << ui->label_rows << ui->label_columns << ui->label_columns << ui->label_columns << ui->label_formats;
    m_pageFormats->setFinalPage(true);

    connect(m_wiz->button(QWizard::FinishButton), &QAbstractButton::clicked, this, &CSVWizard::importClicked);
    connect(m_wiz->button(QWizard::CancelButton), &QAbstractButton::clicked, this, &CSVWizard::reject);
    connect(m_wiz->button(QWizard::CustomButton1), &QAbstractButton::clicked, this, &CSVWizard::fileDialogClicked);
    connect(m_wiz->button(QWizard::CustomButton2), &QAbstractButton::clicked, this, &CSVWizard::saveAsQIFClicked);
    connect(m_wiz, &QWizard::currentIdChanged, this, &CSVWizard::slotIdChanged);

    ui->tableView->setWordWrap(false);

    m_vScrollBar = ui->tableView->verticalScrollBar();
    m_vScrollBar->setTracking(false);

    m_clearBrush = KColorScheme(QPalette::Normal).background(KColorScheme::NormalBackground);
    m_clearBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::NormalText);
    m_colorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::PositiveBackground);
    m_colorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::PositiveText);
    m_errorBrush = KColorScheme(QPalette::Normal).background(KColorScheme::NegativeBackground);
    m_errorBrushText = KColorScheme(QPalette::Normal).foreground(KColorScheme::NegativeText);
    m_wiz->setSideWidget(ui->wizardBox);
    show();
}

CSVWizard::~CSVWizard()
{
    delete m_imp;
    delete ui;
}

void CSVWizard::presetFilename(const QString& name)
{
    m_fileName = name;
}

void CSVWizard::showStage()
{
    QString str = ui->label_intro->text();
    ui->label_intro->setText(QString::fromLatin1("<b>%1</b>").arg(str));
}

void CSVWizard::readWindowSize(const KSharedConfigPtr& config) {
    KConfigGroup miscGroup(config, CSVImporterCore::m_confMiscName);
    m_initialWidth = miscGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfWidth), 800);
    m_initialHeight = miscGroup.readEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfHeight), 400);
}

void CSVWizard::saveWindowSize(const KSharedConfigPtr& config) {
    KConfigGroup miscGroup(config, CSVImporterCore::m_confMiscName);
    m_initialHeight = this->geometry().height();
    m_initialWidth = this->geometry().width();
    miscGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfWidth), m_initialWidth);
    miscGroup.writeEntry(CSVImporterCore::m_miscSettingsConfName.value(ConfHeight), m_initialHeight);
    miscGroup.sync();
}

void CSVWizard::slotIdChanged(int id)
{
    QString txt;
    m_lastId = m_curId;
    m_curId = id;
    if ((m_lastId == -1) || (m_curId == -1)) {
        return;
    }
    txt = m_stageLabels[m_lastId]->text();
    txt.remove(QRegularExpression(QStringLiteral("[<b>/]")));
    m_stageLabels[m_lastId]->setText(txt);

    txt = m_stageLabels[m_curId]->text();
    txt = QString::fromLatin1("<b>%1</b>").arg(txt);
    m_stageLabels[m_curId]->setText(txt);
}

void CSVWizard::clearColumnsBackground(const int col)
{
    QList<int> columnList;
    columnList << col;
    clearColumnsBackground(columnList);
}

void CSVWizard::clearColumnsBackground(const QList<int> &columnList)
{
    QStandardItemModel *model = m_imp->m_file->m_model;
    for (int i = m_imp->m_profile->m_startLine; i <= m_imp->m_profile->m_endLine; ++i) {
        Q_FOREACH (const auto j, columnList) {
            model->item(i, j)->setBackground(m_clearBrush);
            model->item(i, j)->setForeground(m_clearBrushText);
        }
    }
}

void CSVWizard::clearBackground()
{
    QStandardItemModel *model = m_imp->m_file->m_model;
    int rowCount = model->rowCount();
    int colCount = model->columnCount();
    for (int i = 0; i < rowCount; ++i) {
        for (int j = 0; j < colCount; ++j) {
            model->item(i, j)->setBackground(m_clearBrush);
            model->item(i, j)->setForeground(m_clearBrushText);
        }
    }
}

void CSVWizard::markUnwantedRows()
{
    QStandardItemModel *model = m_imp->m_file->m_model;
    int rowCount = model->rowCount();
    int colCount = model->columnCount();
    QBrush brush;
    QBrush brushText;
    for (int i = 0; i < rowCount; ++i) {
        if ((i < m_imp->m_profile->m_startLine) || (i > m_imp->m_profile->m_endLine)) {
            brush = m_errorBrush;
            brushText = m_errorBrushText;
        } else {
            brush = m_clearBrush;
            brushText = m_clearBrushText;
        }
        for (int j = 0; j < colCount; ++j) {
            model->item(i, j)->setBackground(brush);
            model->item(i, j)->setForeground(brushText);
        }
    }
}

void CSVWizard::updateWindowSize()
{
    QTableView *table = this->ui->tableView;
    table->resizeColumnsToContents();
    this->repaint();

    QSize screen = this->screen()->size(); // get available screen size
    QRect wizard = this->frameGeometry(); // get current wizard size

    int newWidth = (table->contentsMargins().left() //
                    + table->contentsMargins().right() //
                    + table->horizontalHeader()->length() //
                    + table->verticalHeader()->width() //
                    + (wizard.width() - table->width()));
    if (table->verticalScrollBar()->isEnabled()) {
        if (!table->verticalScrollBar()->isVisible() && // vertical scrollbar may be not visible after repaint...
                table->horizontalScrollBar()->isVisible())  // ...so use horizontal scrollbar dimension
            newWidth += table->horizontalScrollBar()->height();
        else
            newWidth += table->verticalScrollBar()->width();
    }

    int newHeight = (table->contentsMargins().top() //
                     + table->contentsMargins().bottom() //
                     + table->verticalHeader()->length() //
                     + table->horizontalHeader()->height() //
                     + (wizard.height() - table->height()));

    if (table->horizontalScrollBar()->isEnabled()) {
        if (!table->horizontalScrollBar()->isVisible() && // horizontal scrollbar may be not visible after repaint...
                table->verticalScrollBar()->isVisible())      // ...so use vertical scrollbar dimension
            newHeight += table->verticalScrollBar()->width();
        else
            newHeight += table->horizontalScrollBar()->height();
    }

    // limit wizard size to screen size
    if (newHeight > screen.height())
        newHeight = screen.height();

    if (newWidth > screen.width())
        newWidth = screen.width();


    // don't shrink wizard if required size is less than initial
    if (newWidth < this->m_initialWidth)
        newWidth = this->m_initialWidth;
    if (newHeight < this->m_initialHeight)
        newHeight = this->m_initialHeight;

    newWidth -= (wizard.width() - this->geometry().width());      // remove window frame
    newHeight -= (wizard.height() - this->geometry().height());

    wizard.setWidth(newWidth);
    wizard.setHeight(newHeight);
    wizard.moveTo((screen.width() - wizard.width()) / 2,
                  (screen.height() - wizard.height()) / 2);
    this->setGeometry(wizard);
}

bool CSVWizard::eventFilter(QObject *object, QEvent *event)
{
    // prevent the QWizard part of CSVWizard window from closing on Escape key press
    if (object == this->m_wiz) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                close();
                return true;
            }
        }
    }
    return false;
}

void CSVWizard::saveSettings() const
{
    m_imp->m_profile->m_lastUsedDirectory = m_imp->m_file->m_inFileName;
    m_imp->m_profile->writeSettings(CSVImporterCore::configFile());
    m_imp->profilesAction(m_imp->m_profile->type(), ProfileAction::UpdateLastUsed, m_imp->m_profile->m_profileName, m_imp->m_profile->m_profileName);
}

void CSVWizard::slotClose()
{
    saveSettings();
    m_st = MyMoneyStatement();        // nothing imported
    accept();
}

void CSVWizard::fileDialogClicked()
{
    m_imp->profileFactory(m_pageIntro->m_profileType, m_pageIntro->ui->m_profiles->currentText());
    bool profileExists = m_imp->m_profile->readSettings(CSVImporterCore::configFile());

    if (!m_fileName.isEmpty()) {
        if (!m_imp->m_file->getInFileName(m_fileName)) {
            if (!m_imp->m_file->getInFileName(m_imp->m_profile->m_lastUsedDirectory)) {
                return;
            }
        }
    } else if (!m_imp->m_file->getInFileName(m_imp->m_profile->m_lastUsedDirectory))
        return;

    saveWindowSize(CSVImporterCore::configFile());
    m_imp->m_file->readFile(m_imp->m_profile);
    m_imp->m_file->setupParser(m_imp->m_profile);

    m_skipSetup = m_pageIntro->ui->m_skipSetup->isChecked();

    switch(m_imp->m_profile->type()) {
    case Profile::Investment:
        if (!m_pageInvestment) {
            m_pageInvestment = new InvestmentPage(this, m_imp);
            m_wiz->setPage(CSVWizard::PageInvestment, m_pageInvestment);
        }
        break;
    case Profile::Banking:
        if (!m_pageBanking) {
            m_pageBanking = new BankingPage(this, m_imp);
            m_wiz->setPage(CSVWizard::PageBanking, m_pageBanking);
        }
        break;
    case Profile::StockPrices:
    case Profile::CurrencyPrices:
        if (!m_pagePrices) {
            m_pagePrices = new PricesPage(this, m_imp);
            m_wiz->setPage(CSVWizard::PagePrices, m_pagePrices);
        }
        break;
    default:
        return;
    }

    m_wiz->next();  // go to separator page

    if (m_skipSetup && profileExists) {
        // programmatically skip to last page
        while((m_wiz->currentPage() != m_pageFormats) && (m_wiz->nextId() != -1)) {
            m_wiz->next();
        }
    }
}

void CSVWizard::importClicked()
{
    switch (m_imp->m_profile->type()) {
    case Profile::Banking:
        if (!m_pageBanking->validateCreditDebit())
            return;
        break;
    case Profile::Investment:
        if (!m_pageInvestment->validateActionType())
            return;
        break;
    default:
        break;
    }

    saveSettings();

    if (!m_imp->createStatement(m_st)) {
        m_st = MyMoneyStatement();
        return;
    }
    accept();
}

const MyMoneyStatement& CSVWizard::statement() const
{
    return m_st;
}

void CSVWizard::saveAsQIFClicked()
{
    switch (m_imp->m_profile->type()) {
    case Profile::Banking:
        if (!m_pageBanking->validateCreditDebit())
            return;
        break;
    case Profile::Investment:
        if (!m_pageInvestment->validateActionType())
            return;
        break;
    default:
        break;
    }

    bool isOK = m_imp->createStatement(m_st);
    if (!isOK || m_st.m_listTransactions.isEmpty())
        return;

    QString outFileName = m_imp->m_file->m_inFileName;
    outFileName.truncate(outFileName.lastIndexOf('.'));
    outFileName.append(QLatin1String(".qif"));
    outFileName = QFileDialog::getSaveFileName(this, i18n("Save QIF"), outFileName, i18n("QIF Files (*.qif)"));
    if (outFileName.isEmpty())
        return;

    MyMoneyQifProfile qifProfile(PluginSettings::qifExportProfile());

    switch (m_imp->m_profile->type()) {
    case Profile::Banking:
        m_pageBanking->makeQIF(m_st, outFileName, qifProfile);
        break;
    case Profile::Investment:
        m_pageInvestment->makeQIF(m_st, outFileName, qifProfile);
        break;
    default:
        break;
    }
}

void CSVWizard::initializeComboBoxes(const QHash<Column, QComboBox *> &columns)
{
    QStringList columnNumbers;
    for (int i = 0; i < m_imp->m_file->m_columnCount; ++i)
        columnNumbers.append(QString::number(i + 1));

    Q_FOREACH (const auto column, columns) {
        // block signals from combobox during their initialization
        QSignalBlocker blocker(column);
        // clear all existing items before adding new ones
        column->clear();
        // populate comboboxes with col # values
        column->addItems(columnNumbers);
        // all comboboxes are set to 0 so set them to -1
        column->setCurrentIndex(-1);
    }
}

//-------------------------------------------------------------------------------------------------------
IntroPage::IntroPage(CSVWizard *dlg, CSVImporterCore *imp) :
    CSVWizardPage(dlg, imp),
    m_profileType(Profile::Banking),
    ui(new Ui::IntroPage)
{
    ui->setupUi(this);
}

IntroPage::~IntroPage()
{
    delete ui;
}

int IntroPage::nextId() const
{
    return CSVWizard::PageSeparator;
}

void IntroPage::initializePage()
{
    m_imp->m_file->m_model->clear();

    wizard()->setButtonText(QWizard::CustomButton1, i18n("Select File"));
    wizard()->button(QWizard::CustomButton1)->setToolTip(i18n("A profile must be selected before selecting a file."));
    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch <<
           QWizard::CustomButton1 <<
           QWizard::CancelButton;
    wizard()->setButtonLayout(layout);

    ui->m_profiles->lineEdit()->setClearButtonEnabled(true);

    connect(ui->m_profiles, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IntroPage::slotComboSourceIndexChanged);
    connect(ui->m_add, &QAbstractButton::clicked, this, &IntroPage::slotAddProfile);
    connect(ui->m_remove, &QAbstractButton::clicked, this, &IntroPage::slotRemoveProfile);
    connect(ui->m_rename, &QAbstractButton::clicked, this, &IntroPage::slotRenameProfile);
    connect(ui->m_profilesBank, &QAbstractButton::toggled, this, &IntroPage::slotBankRadioToggled);
    connect(ui->m_profilesInvest, &QAbstractButton::toggled, this, &IntroPage::slotInvestRadioToggled);
    connect(ui->m_profilesCurrencyPrices, &QAbstractButton::toggled, this, &IntroPage::slotCurrencyPricesRadioToggled);
    connect(ui->m_profilesStockPrices, &QAbstractButton::toggled, this, &IntroPage::slotStockPricesRadioToggled);
    if (m_dlg->m_initialHeight == -1 || m_dlg->m_initialWidth == -1) {
        m_dlg->m_initialHeight = m_dlg->geometry().height();
        m_dlg->m_initialWidth = m_dlg->geometry().width();
    } else {
        //resize wizard to its initial size and center it
        m_dlg->setGeometry(
            QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, QSize(m_dlg->m_initialWidth, m_dlg->m_initialHeight), screen()->availableGeometry()));
    }
    m_dlg->ui->tableView->hide();
}

bool IntroPage::validatePage()
{
    return true;
}

void IntroPage::slotAddProfile()
{
    profileChanged(ProfileAction::Add);
}

void IntroPage::slotRemoveProfile()
{
    profileChanged(ProfileAction::Remove);
}

void IntroPage::slotRenameProfile()
{
    profileChanged(ProfileAction::Rename);
}

void IntroPage::profileChanged(const ProfileAction action)
{
    QString cbText = ui->m_profiles->currentText();
    if (cbText.isEmpty()) // you cannot neither add nor remove empty name profile or rename to empty name
        return;

    int cbIndex = ui->m_profiles->currentIndex();

    switch (action) {
    case ProfileAction::Rename:
    case ProfileAction::Add:
    {
        int dupIndex = m_profiles.indexOf(QRegularExpression (cbText));
        if (dupIndex == cbIndex && cbIndex != -1)  // if profile name wasn't changed then return
            return;
        else if (dupIndex != -1) {    // profile with the same name already exists
            ui->m_profiles->setItemText(cbIndex, m_profiles.value(cbIndex));
            KMessageBox::information(m_dlg,
                                     i18n("<center>Profile <b>%1</b> already exists.<br>"
                                          "Please enter another name</center>", cbText));
            return;
        }
        break;
    }
    case ProfileAction::Remove:
        if (m_profiles.value(cbIndex) != cbText) // user changed name of the profile and tries to remove it
            return;
        break;
    default:
        break;
    }

    if (CSVImporterCore::profilesAction(m_profileType, action, m_profiles.value(cbIndex), cbText)) {
        switch (action) {
        case ProfileAction::Add:
            m_profiles.append(cbText);
            ui->m_profiles->addItem(cbText);
            ui->m_profiles->setCurrentIndex(m_profiles.count() - 1);
            KMessageBox::information(m_dlg,
                                     i18n("<center>Profile <b>%1</b> has been added.</center>", cbText));
            break;
        case ProfileAction::Remove:
            m_profiles.removeAt(cbIndex);
            ui->m_profiles->removeItem(cbIndex);
            KMessageBox::information(m_dlg,
                                     i18n("<center>Profile <b>%1</b> has been removed.</center>",
                                          cbText));
            break;
        case ProfileAction::Rename:
            ui->m_profiles->setItemText(cbIndex, cbText);
            KMessageBox::information(m_dlg,
                                     i18n("<center>Profile name has been renamed from <b>%1</b> to <b>%2</b>.</center>",
                                          m_profiles.value(cbIndex), cbText));
            m_profiles[cbIndex] = cbText;
            break;
        default:
            break;
        }
    }
}

void IntroPage::slotComboSourceIndexChanged(int idx)
{
    if (idx == -1) {
        wizard()->button(QWizard::CustomButton1)->setEnabled(false);
        ui->m_skipSetup->setEnabled(false);
        ui->m_remove->setEnabled(false);
        ui->m_rename->setEnabled(false);
    }
    else {
        wizard()->button(QWizard::CustomButton1)->setEnabled(true);
        ui->m_skipSetup->setEnabled(true);
        ui->m_remove->setEnabled(true);
        ui->m_rename->setEnabled(true);
    }
}

void IntroPage::profileTypeChanged(const Profile profileType, bool toggled)
{
    if (!toggled)
        return;

    KConfigGroup profilesGroup(CSVImporterCore::configFile(), CSVImporterCore::m_confProfileNames);
    m_profileType = profileType;
    QString profileTypeStr;
    switch (m_profileType) {
    case Profile::Banking:
        ui->m_profilesInvest->setChecked(false);
        ui->m_profilesStockPrices->setChecked(false);
        ui->m_profilesCurrencyPrices->setChecked(false);
        break;
    case Profile::Investment:
        ui->m_profilesBank->setChecked(false);
        ui->m_profilesStockPrices->setChecked(false);
        ui->m_profilesCurrencyPrices->setChecked(false);
        break;
    case Profile::StockPrices:
        ui->m_profilesBank->setChecked(false);
        ui->m_profilesInvest->setChecked(false);
        ui->m_profilesCurrencyPrices->setChecked(false);
        break;
    case Profile::CurrencyPrices:
        ui->m_profilesBank->setChecked(false);
        ui->m_profilesInvest->setChecked(false);
        ui->m_profilesStockPrices->setChecked(false);
        break;
    default:
        break;
    }
    profileTypeStr = CSVImporterCore::m_profileConfPrefix.value(m_profileType);

    m_profiles = profilesGroup.readEntry(profileTypeStr, QStringList());
    int priorProfile = profilesGroup.readEntry(CSVImporterCore::m_confPriorName + profileTypeStr, 0);
    ui->m_profiles->clear();
    ui->m_profiles->addItems(m_profiles);
    ui->m_profiles->setCurrentIndex(priorProfile);
    ui->m_profiles->setEnabled(true);
    ui->m_add->setEnabled(true);
}

void IntroPage::slotBankRadioToggled(bool toggled)
{
    profileTypeChanged(Profile::Banking, toggled);
}

void IntroPage::slotInvestRadioToggled(bool toggled)
{
    profileTypeChanged(Profile::Investment, toggled);
}

void IntroPage::slotCurrencyPricesRadioToggled(bool toggled)
{
    profileTypeChanged(Profile::CurrencyPrices, toggled);
}

void IntroPage::slotStockPricesRadioToggled(bool toggled)
{
    profileTypeChanged(Profile::StockPrices, toggled);
}

SeparatorPage::SeparatorPage(CSVWizard *dlg, CSVImporterCore *imp) :
    CSVWizardPage(dlg, imp),
    ui(new Ui::SeparatorPage)
{
    ui->setupUi(this);
    connect(ui->m_encoding, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SeparatorPage::encodingChanged);
    connect(ui->m_fieldDelimiter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SeparatorPage::fieldDelimiterChanged);
    connect(ui->m_textDelimiter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SeparatorPage::textDelimiterChanged);
}

SeparatorPage::~SeparatorPage()
{
    delete ui;
}

void SeparatorPage::initializePage()
{
    m_dlg->ui->tableView->show();
    // comboboxes are preset to -1 and, in new profile case, can be set here to -1 as well ...
    // ... so block their signals until setting them ...
    ui->m_encoding->blockSignals(true);
    ui->m_fieldDelimiter->blockSignals(true);
    ui->m_textDelimiter->blockSignals(true);
    initializeEncodingCombobox();
    ui->m_encoding->setCurrentIndex(ui->m_encoding->findData(m_imp->m_profile->m_encodingMIBEnum));
    ui->m_fieldDelimiter->setCurrentIndex((int)m_imp->m_profile->m_fieldDelimiter);
    ui->m_textDelimiter->setCurrentIndex((int)m_imp->m_profile->m_textDelimiter);
    ui->m_encoding->blockSignals(false);
    ui->m_fieldDelimiter->blockSignals(false);
    ui->m_textDelimiter->blockSignals(false);

    // ... and ensure that their signal receivers will always be called
    Q_EMIT ui->m_encoding->currentIndexChanged(ui->m_encoding->currentIndex());
    Q_EMIT ui->m_fieldDelimiter->currentIndexChanged(ui->m_fieldDelimiter->currentIndex());
    Q_EMIT ui->m_textDelimiter->currentIndexChanged(ui->m_textDelimiter->currentIndex());
    m_dlg->updateWindowSize();

    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch << QWizard::BackButton << QWizard::NextButton << QWizard::CancelButton;
    wizard()->setButtonLayout(layout);
}

void SeparatorPage::initializeEncodingCombobox()
{
    ui->m_encoding->clear();

    QList<QTextCodec *>   codecs;
    QMap<QString, QTextCodec *> codecMap;
    const QRegularExpression iso8859RegExp(QLatin1String("ISO[- ]8859-([0-9]+).*"));

    Q_FOREACH (const auto mib, QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        QString sortKey = codec->name().toUpper();
        int rank;

        const auto iso8859(iso8859RegExp.match(sortKey));
        if (sortKey.startsWith(QLatin1String("UTF-8"))) {             // krazy:exclude=strings
            rank = 1;
        } else if (sortKey.startsWith(QLatin1String("UTF-16"))) {            // krazy:exclude=strings
            rank = 2;
        } else if (iso8859.hasMatch()) {
            if (iso8859.captured(1).size() == 1)
                rank = 3;
            else
                rank = 4;
        } else {
            rank = 5;
        }
        sortKey.prepend(QChar('0' + rank));

        codecMap.insert(sortKey, codec);
    }
    codecs = codecMap.values();

    Q_FOREACH (const auto codec, codecs)
        ui->m_encoding->addItem(codec->name(), codec->mibEnum());
}

void SeparatorPage::encodingChanged(const int index)
{
    if (index == -1) {
        ui->m_encoding->setCurrentIndex(ui->m_encoding->findText("UTF-8"));
        return;
    } else if (index == ui->m_encoding->findData(m_imp->m_profile->m_encodingMIBEnum))
        return;
    m_imp->m_profile->m_encodingMIBEnum = ui->m_encoding->currentData().toInt();
    m_imp->m_file->readFile(m_imp->m_profile);
    Q_EMIT completeChanged();
}

void SeparatorPage::fieldDelimiterChanged(const int index)
{
    if (index == -1 &&                                        // if field delimiter isn't set...
            !m_imp->m_autodetect.value(AutoFieldDelimiter))  // ... and user disabled autodetecting...
        return;                                                 // ... then wait for him to choose
    else if (index == (int)m_imp->m_profile->m_fieldDelimiter)
        return;

    m_imp->m_profile->m_fieldDelimiter = static_cast<FieldDelimiter>(int(index));
    m_imp->m_file->readFile(m_imp->m_profile);      // get column count, we get with this fieldDelimiter
    m_imp->m_file->setupParser(m_imp->m_profile);

    if (index == -1) {
        ui->m_fieldDelimiter->blockSignals(true);
        ui->m_fieldDelimiter->setCurrentIndex((int)m_imp->m_profile->m_fieldDelimiter);
        ui->m_fieldDelimiter->blockSignals(false);
    }
    m_dlg->updateWindowSize();
    Q_EMIT completeChanged();
}

void SeparatorPage::textDelimiterChanged(const int index)
{
    if (index == -1) {                                  // if text delimiter isn't set...
        ui->m_textDelimiter->setCurrentIndex(0);        // ...then set it to 0, as for now there is no better idea how to detect it
        return;
    }

    m_imp->m_profile->m_textDelimiter = static_cast<TextDelimiter>(index);
    m_imp->m_file->setupParser(m_imp->m_profile);
    Q_EMIT completeChanged();
}

bool SeparatorPage::isComplete() const
{
    bool rc = false;
    if (ui->m_encoding->currentIndex() != -1 &&
            ui->m_fieldDelimiter->currentIndex() != -1 &&
            ui->m_textDelimiter->currentIndex() != -1) {
        switch(m_imp->m_profile->type()) {
        case Profile::Banking:
            if (m_imp->m_file->m_columnCount > 2)
                rc = true;
            break;
        case Profile::Investment:
            if (m_imp->m_file->m_columnCount > 3)
                rc = true;
            break;
        case Profile::CurrencyPrices:
        case Profile::StockPrices:
            if (m_imp->m_file->m_columnCount > 1)
                rc = true;
            break;
        default:
            break;
        }
    }
    return rc;
}

bool SeparatorPage::validatePage()
{
    return true;
}

void SeparatorPage::cleanupPage()
{
    //  On completion with error force use of 'Back' button.
    //  ...to allow resetting of 'Skip setup'
    m_dlg->m_pageIntro->initializePage();  //  Need to show button(QWizard::CustomButton1) not 'NextButton'
}

RowsPage::RowsPage(CSVWizard *dlg, CSVImporterCore *imp) :
    CSVWizardPage(dlg, imp),
    ui(new Ui::RowsPage)
{
    ui->setupUi(this);
    connect(ui->m_headerHasName, &QAbstractButton::clicked, this, [&](bool checked) {
        m_imp->m_profile->m_autoAccountName = checked;
    });
    connect(ui->m_startLine, QOverload<int>::of(&QSpinBox::valueChanged), this, &RowsPage::startRowChanged);
    connect(ui->m_endLine, QOverload<int>::of(&QSpinBox::valueChanged), this, &RowsPage::endRowChanged);
}

RowsPage::~RowsPage()
{
    delete ui;
}

void RowsPage::initializePage()
{
    ui->m_startLine->blockSignals(true);
    ui->m_endLine->blockSignals(true);
    ui->m_startLine->setMaximum(m_imp->m_file->m_rowCount);
    ui->m_endLine->setMaximum(m_imp->m_file->m_rowCount);
    ui->m_startLine->setValue(m_imp->m_profile->m_startLine + 1);
    ui->m_endLine->setValue(m_imp->m_profile->m_endLine + 1);
    ui->m_headerHasName->setChecked(m_imp->m_profile->m_autoAccountName);
    ui->m_startLine->blockSignals(false);
    ui->m_endLine->blockSignals(false);

    m_dlg->markUnwantedRows();
    m_dlg->m_vScrollBar->setValue(m_imp->m_profile->m_startLine);

    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch <<
           QWizard::BackButton <<
           QWizard::NextButton <<
           QWizard::CancelButton;
    wizard()->setButtonLayout(layout);
}

void RowsPage::cleanupPage()
{
    m_dlg->clearBackground();
}

int RowsPage::nextId() const
{
    int ret;
    switch (m_imp->m_profile->type()) {
    case Profile::Banking:
        ret = CSVWizard::PageBanking;
        break;
    case Profile::Investment:
        ret = CSVWizard::PageInvestment;
        break;
    case Profile::StockPrices:
    case Profile::CurrencyPrices:
        ret = CSVWizard::PagePrices;
        break;
    default:
        ret = CSVWizard::PageRows;
        break;
    }
    return ret;
}

void RowsPage::startRowChanged(int val)
{
    if (val > m_imp->m_file->m_rowCount) {
        ui->m_startLine->setValue(m_imp->m_file->m_rowCount - 1);
        return;
    }
    --val;
    if (val > m_imp->m_profile->m_endLine) {
        if (m_imp->m_profile->m_endLine <= m_imp->m_file->m_rowCount)
            ui->m_startLine->setValue(m_imp->m_profile->m_endLine + 1);
        return;
    }
    m_imp->m_profile->m_startLine = val;
    m_dlg->m_vScrollBar->setValue(val);
    m_dlg->markUnwantedRows();
}

void RowsPage::endRowChanged(int val)
{
    if (val > m_imp->m_file->m_rowCount) {
        ui->m_endLine->setValue(m_imp->m_file->m_rowCount - 1);
        return;
    }
    --val;
    if (val < m_imp->m_profile->m_startLine) {
        if (m_imp->m_profile->m_startLine <= m_imp->m_file->m_rowCount)
            ui->m_endLine->setValue(m_imp->m_profile->m_startLine + 1);
        return;
    }
    m_imp->m_profile->m_trailerLines = m_imp->m_file->m_rowCount - (val + 1);
    m_imp->m_profile->m_endLine = val;
    m_dlg->markUnwantedRows();
}


FormatsPage::FormatsPage(CSVWizard *dlg, CSVImporterCore *imp) :
    CSVWizardPage(dlg, imp),
    ui(new Ui::FormatsPage),
    m_isDecimalSymbolOK(false),
    m_isDateFormatOK(false)
{
    ui->setupUi(this);
    connect(ui->m_dateFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FormatsPage::dateFormatChanged);
    connect(ui->m_decimalSymbol, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FormatsPage::decimalSymbolChanged);
}

FormatsPage::~FormatsPage()
{
    delete ui;
}

void FormatsPage::initializePage()
{
    m_isDecimalSymbolOK = false;
    m_isDateFormatOK = false;
    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch
           << QWizard::CustomButton2
           << QWizard::BackButton
           << QWizard::FinishButton
           << QWizard::CancelButton;
    wizard()->setButtonText(QWizard::FinishButton, i18n("Import CSV"));
    wizard()->setOption(QWizard::HaveCustomButton2, true);
    wizard()->setButtonText(QWizard::CustomButton2, i18n("Make QIF File"));
    wizard()->setButtonLayout(layout);
    wizard()->button(QWizard::CustomButton2)->setEnabled(false);
    wizard()->button(QWizard::FinishButton)->setEnabled(false);

    ui->m_thousandsDelimiter->setEnabled(false);

    ui->m_dateFormat->blockSignals(true);
    ui->m_dateFormat->setCurrentIndex((int)m_imp->m_profile->m_dateFormat);
    ui->m_dateFormat->blockSignals(false);
    Q_EMIT ui->m_dateFormat->currentIndexChanged((int)m_imp->m_profile->m_dateFormat); // Q_EMIT update signal manually regardless of change to combobox

    ui->m_decimalSymbol->blockSignals(true);
    if (m_imp->m_profile->m_decimalSymbol == DecimalSymbol::Auto && !m_imp->m_autodetect.value(AutoDecimalSymbol))
        ui->m_decimalSymbol->setCurrentIndex(-1);
    else
        ui->m_decimalSymbol->setCurrentIndex((int)m_imp->m_profile->m_decimalSymbol);
    ui->m_decimalSymbol->blockSignals(false);
    Q_EMIT ui->m_decimalSymbol->currentIndexChanged((int)m_imp->m_profile->m_decimalSymbol); // Q_EMIT update signal manually regardless of change to combobox
}

void FormatsPage::decimalSymbolChanged(int index)
{
    const QList<int> columns = m_imp->getNumericalColumns();
    switch (index) {
    case -1:
        if (!m_imp->m_autodetect.value(AutoDecimalSymbol)) {
            break;
        }
    // intentional fall through

    case 2:
    {
        ui->m_decimalSymbol->blockSignals(true);
        m_imp->m_profile->m_decimalSymbol = DecimalSymbol::Auto;
        int failColumn = m_imp->detectDecimalSymbols(columns);
        if (failColumn != -2) {
            KMessageBox::error(this, i18n("<center>Autodetect could not detect your decimal symbol in column %1.</center>"
                                          "<center>Try manual selection to see problematic cells and correct your data.</center>", failColumn), i18n("CSV import"));
            ui->m_decimalSymbol->setCurrentIndex(-1);
            ui->m_thousandsDelimiter->setCurrentIndex(-1);
        } else if (index == -1) { // if detection went well and decimal symbol was unspecified then we'll be specifying it
            DecimalSymbol firstDecSymbol = m_imp->m_decimalSymbolIndexMap.first();
            bool allSymbolsEqual = true;
            Q_FOREACH (const auto mapDecSymbol, m_imp->m_decimalSymbolIndexMap) {
                if (firstDecSymbol != mapDecSymbol)
                    allSymbolsEqual = false;
            }
            if (allSymbolsEqual) {   // if symbol in all columns is equal then set it...
                m_imp->m_profile->m_decimalSymbol = firstDecSymbol;
                ui->m_decimalSymbol->setCurrentIndex((int)firstDecSymbol);
                ui->m_thousandsDelimiter->setCurrentIndex((int)firstDecSymbol);
            } else {  // else set to auto
                m_imp->m_profile->m_decimalSymbol = DecimalSymbol::Auto;
                ui->m_decimalSymbol->setCurrentIndex((int)DecimalSymbol::Auto);
                ui->m_thousandsDelimiter->setCurrentIndex((int)DecimalSymbol::Auto);
            }
        }
        ui->m_decimalSymbol->blockSignals(false);
        break;
    }
    default:
        Q_FOREACH (const auto column, columns)
            m_imp->m_decimalSymbolIndexMap.insert(column, static_cast<DecimalSymbol>(index));
        ui->m_thousandsDelimiter->setCurrentIndex(index);
        m_imp->m_profile->m_decimalSymbol = static_cast<DecimalSymbol>(index);
    }

    m_isDecimalSymbolOK = validateDecimalSymbols(columns);
    Q_EMIT completeChanged();
}

bool FormatsPage::validateDecimalSymbols(const QList<int> &columns)
{
    bool isOK = true;
    Q_FOREACH (const auto col, columns) {
        m_imp->m_file->m_parse->setDecimalSymbol(m_imp->m_decimalSymbolIndexMap.value(col));
        m_dlg->clearColumnsBackground(col);
        for (int row = m_imp->m_profile->m_startLine; row <= m_imp->m_profile->m_endLine; ++row) {
            QStandardItem *item = m_imp->m_file->m_model->item(row, col);
            QString rawNumber = item->text();
            m_imp->m_file->m_parse->possiblyReplaceSymbol(rawNumber);
            if (!m_imp->m_file->m_parse->invalidConversion() ||
                    rawNumber.isEmpty()) {                   // empty strings are welcome
                item->setBackground(m_dlg->m_colorBrush);
                item->setForeground(m_dlg->m_colorBrushText);
            } else {
                isOK = false;
                m_dlg->ui->tableView->scrollTo(item->index(), QAbstractItemView::EnsureVisible);
                item->setBackground(m_dlg->m_errorBrush);
                item->setForeground(m_dlg->m_errorBrushText);
            }
        }

    }
    return isOK;
}

void FormatsPage::dateFormatChanged(const int index)
{
    if (index == -1)
        return;

    int col = m_imp->m_profile->m_colTypeNum.value(Column::Date);
    m_imp->m_profile->m_dateFormat = static_cast<DateFormat>(index);
    m_imp->m_convertDate->setDateFormatIndex(static_cast<DateFormat>(index));
    m_isDateFormatOK = validateDateFormat(col);
    if (!m_isDateFormatOK) {
        KMessageBox::error(this, i18n("<center>There are invalid date formats in column '%1'.</center>"
                                      "<center>Please check your selections.</center>"
                                      , col + 1), i18n("CSV import"));
    }
    Q_EMIT completeChanged();
}

bool FormatsPage::validateDateFormat(const int col)
{
    m_dlg->clearColumnsBackground(col);
    QDate emptyDate;

    bool isOK = true;
    for (int row = m_imp->m_profile->m_startLine; row <= m_imp->m_profile->m_endLine; ++row) {
        QStandardItem* item = m_imp->m_file->m_model->item(row, col);
        QDate dat = m_imp->m_convertDate->convertDate(item->text());
        if (dat == emptyDate) {
            isOK = false;
            m_dlg->ui->tableView->scrollTo(item->index(), QAbstractItemView::EnsureVisible);
            item->setBackground(m_dlg->m_errorBrush);
            item->setForeground(m_dlg->m_errorBrushText);
        } else {
            item->setBackground(m_dlg->m_colorBrush);
            item->setForeground(m_dlg->m_colorBrushText);
        }
    }
    return isOK;
}

bool FormatsPage::isComplete() const
{
    const bool enable = m_isDecimalSymbolOK && m_isDateFormatOK;
    const auto button = wizard()->button(QWizard::CustomButton2);

    if ((m_imp->m_profile->type() != Profile::StockPrices) && (m_imp->m_profile->type() != Profile::CurrencyPrices)) {
        if (enable) {
            // check if QIF profile exists and is selected
            QStringList profileList;
            KSharedConfigPtr config = KSharedConfig::openConfig();
            KConfigGroup grp = config->group("Profiles");
            profileList = grp.readEntry("profiles", QStringList());

            const auto profile = PluginSettings::qifExportProfile();

            if (profile.isEmpty() || profileList.contains(profile)) {
                button->setEnabled(true);
                button->setToolTip(QString());
            } else {
                button->setToolTip(i18nc("@info:tooltip QIF profile not selected", "Please select a QIF profile in the CSV importer settings"));
            }
        }
    }
    return enable;
}

void FormatsPage::cleanupPage()
{
    QList<int> columns = m_imp->getNumericalColumns();
    columns.append(m_imp->m_profile->m_colTypeNum.value(Column::Date));
    m_dlg->clearColumnsBackground(columns);
    m_dlg->m_st = MyMoneyStatement();  // any change on investment/banking page invalidates created statement

    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch <<
           QWizard::BackButton <<
           QWizard::NextButton <<
           QWizard::CancelButton;
    wizard()->setButtonLayout(layout);
}
