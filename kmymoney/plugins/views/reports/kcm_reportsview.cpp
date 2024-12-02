/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz lukasz.wojnilowicz @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_reportsview.h"
#include <config-kmymoney-version.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrl>
#include <QFileInfo>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KAboutData>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <kio_version.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"
#include "ui_reportsviewsettings.h"

class ReportsViewSettingsWidgetPrivate
{
    Q_DISABLE_COPY(ReportsViewSettingsWidgetPrivate)

public:
    ReportsViewSettingsWidgetPrivate() :
        ui(new Ui::ReportsViewSettings),
        m_fileKLineEdit(nullptr)
    {
    }

    ~ReportsViewSettingsWidgetPrivate()
    {
        delete ui;
    }


    /**
     * Collector for both signals
     * urlSelected and editingFinished.
     *
     * Only shows a warning
     * if the selected file
     * is not a readable plain file -
     * and only one time.
     *
     * @param[in] css  css file name
     *
     * @see ReportsViewSettingsWidget#slotCssUrlSelected
     * @see ReportsViewSettingsWidget#slotEditingFinished
     */
    void checkCssFile(QString& css) {

        if (css == m_cssFileOld) {
            // do not check again to avoid emitting a warning more than 1 time
            return;
        }

        m_cssFileOld = css;

        QFileInfo* info = new QFileInfo(css);

        if (!info->exists()) {
            KMessageBox::error(0, i18n("File %1 does not exist", css));
            return;
        }

        QList<QString> warnings;

        if (!info->isFile()) {
            warnings.append(i18n("it is not a plain file"));
        }

        if (!info->isReadable()) {
            warnings.append(i18n("it is not readable"));
        }

        if (info->size() < 1) {
            warnings.append(i18n("it is empty"));
        }

        if (warnings.size() < 1) {
            // no warnings, fine
            return;
        }

        QString out = i18np("There is a problem with file %1", "There are problems with file %1", css);

        QList<QString>::const_iterator i;
        for (i = warnings.cbegin(); i != warnings.cend(); ++i) {
            out += '\n' + *i;
        }

        KMessageBox::error(0, out);
    }

    Ui::ReportsViewSettings *ui;
    /**
     * Old value of css file to avoid warnings
     * when a signal is emitted
     * but the value itself did not change.
     */
    QString m_cssFileOld;

    /**
     * Pointer to the KLineEdit of the KFileDialog which we need
     * to receive signal editingFinished.
     */
    KLineEdit* m_fileKLineEdit;
};

ReportsViewSettingsWidget::ReportsViewSettingsWidget(QWidget* parent) :
    QWidget(parent),
    d_ptr(new ReportsViewSettingsWidgetPrivate)
{
    Q_D(ReportsViewSettingsWidget);
    d->ui->setupUi(this);

#if KIO_VERSION >= QT_VERSION_CHECK(5, 108, 0)
    d->ui->kcfg_CssFileDefault->setNameFilters({i18n("CSS files") + QLatin1String(" (*.css)"), i18n("All files") + QLatin1String(" (*)")});
#else
    d->ui->kcfg_CssFileDefault->setFilter(QLatin1String("*.css|") + i18n("CSS files") + QLatin1String("\n*|") + i18n("All files"));
#endif

    // keep initial (default) css file in mind
    d->m_cssFileOld = KMyMoneySettings::cssFileDefault();

    // set default css file in ReportsViewSettingsWidget dialog
    d->ui->kcfg_CssFileDefault->setUrl(QUrl::fromLocalFile(KMyMoneySettings::cssFileDefault()));

    d->m_fileKLineEdit = d->ui->kcfg_CssFileDefault->lineEdit();

    connect(d->ui->kcfg_CssFileDefault, &KUrlRequester::urlSelected,
            this, &ReportsViewSettingsWidget::slotCssUrlSelected);

    connect(d->m_fileKLineEdit, &QLineEdit::editingFinished,
            this, &ReportsViewSettingsWidget::slotEditingFinished);
}

ReportsViewSettingsWidget::~ReportsViewSettingsWidget()
{
    Q_D(ReportsViewSettingsWidget);
    delete d;
}

/**
 * Receiver for signal urlSelected.
 *
 * Signal urlSelected only is emitted
 * when a file is selected with the file chooser.
 *
 * @param[in] cssUrl  url of css file
 *
 * @see ReportsViewSettingsWidget#Private#checkCssFile
 */
void ReportsViewSettingsWidget::slotCssUrlSelected(const QUrl &cssUrl)
{
    Q_D(ReportsViewSettingsWidget);
    auto css = cssUrl.toLocalFile();
    d->checkCssFile(css);
}

/**
 * Receiver for signal editingFinished.
 *
 * Signal editingFinished is emitted
 * on focus out only,
 * not  when a file is selected with the file chooser.
 *
 * @see ReportsViewSettingsWidget#Private#checkCssFile
 */
void ReportsViewSettingsWidget::slotEditingFinished()
{
    Q_D(ReportsViewSettingsWidget);
    auto txt = d->m_fileKLineEdit->text();
    d->checkCssFile(txt);
}

KCMReportsView::KCMReportsView(QWidget *parent, const QVariantList& args)
    : KCModule(parent, args)
{
    ReportsViewSettingsWidget* w = new ReportsViewSettingsWidget(this);
    // addConfig(ReportsViewSettings::self(), w);
    addConfig(KMyMoneySettings::self(), w);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(w);
    setButtons(NoAdditionalButton);
    load();
}

KCMReportsView::~KCMReportsView()
{
}

K_PLUGIN_CLASS(KCMReportsView)

#include "kcm_reportsview.moc"
