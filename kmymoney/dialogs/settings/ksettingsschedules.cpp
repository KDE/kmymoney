/*
    SPDX-FileCopyrightText: 2005-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>

#include "ksettingsschedules.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QLocale>
#include <QStandardItemModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#ifdef ENABLE_HOLIDAYS
#include <KHolidays/Holiday>
#include <KHolidays/HolidayRegion>
using namespace KHolidays;
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsschedules.h"


class KSettingsSchedulesPrivate
{
    Q_DISABLE_COPY(KSettingsSchedulesPrivate)

public:
    KSettingsSchedulesPrivate()
        : ui(new Ui::KSettingsSchedules)
        /** @todo with KF5 >= 5.88 use KCountry for this and get rid of the table
         * That's the reason why I did not enclose the country names in i18n() calls
         * because I don't want the translators waste time on this temporary code.
         * With KCountry we get that for free.
         */
        , m_countryCodesToCountryNames({
              {QStringLiteral("IC"), QStringLiteral("Canary Islands")},
              {QStringLiteral("AF"), QStringLiteral("Afghanistan")},
              {QStringLiteral("AX"), QStringLiteral("Åland Islands")},
              {QStringLiteral("AL"), QStringLiteral("Albania")},
              {QStringLiteral("DZ"), QStringLiteral("Algeria")},
              {QStringLiteral("AS"), QStringLiteral("American Samoa")},
              {QStringLiteral("AD"), QStringLiteral("Andorra")},
              {QStringLiteral("AO"), QStringLiteral("Angola")},
              {QStringLiteral("AI"), QStringLiteral("Anguilla")},
              {QStringLiteral("AQ"), QStringLiteral("Antarctica")},
              {QStringLiteral("AG"), QStringLiteral("Antigua and Barbuda")},
              {QStringLiteral("AR"), QStringLiteral("Argentina")},
              {QStringLiteral("AM"), QStringLiteral("Armenia")},
              {QStringLiteral("AW"), QStringLiteral("Aruba")},
              {QStringLiteral("AU"), QStringLiteral("Australia")},
              {QStringLiteral("AT"), QStringLiteral("Austria")},
              {QStringLiteral("AZ"), QStringLiteral("Azerbaijan")},
              {QStringLiteral("BS"), QStringLiteral("Bahamas")},
              {QStringLiteral("BH"), QStringLiteral("Bahrain")},
              {QStringLiteral("BD"), QStringLiteral("Bangladesh")},
              {QStringLiteral("BB"), QStringLiteral("Barbados")},
              {QStringLiteral("BY"), QStringLiteral("Belarus")},
              {QStringLiteral("BE"), QStringLiteral("Belgium")},
              {QStringLiteral("BZ"), QStringLiteral("Belize")},
              {QStringLiteral("BJ"), QStringLiteral("Benin")},
              {QStringLiteral("BM"), QStringLiteral("Bermuda")},
              {QStringLiteral("BT"), QStringLiteral("Bhutan")},
              {QStringLiteral("BO"), QStringLiteral("Bolivia")},
              {QStringLiteral("BA"), QStringLiteral("Bosnia and Herzegovina")},
              {QStringLiteral("BW"), QStringLiteral("Botswana")},
              {QStringLiteral("BV"), QStringLiteral("Bouvet Island")},
              {QStringLiteral("BR"), QStringLiteral("Brazil")},
              {QStringLiteral("IO"), QStringLiteral("British Indian Ocean Territory")},
              {QStringLiteral("BN"), QStringLiteral("Brunei Darussalam")},
              {QStringLiteral("BG"), QStringLiteral("Bulgaria")},
              {QStringLiteral("BF"), QStringLiteral("Burkina Faso")},
              {QStringLiteral("BI"), QStringLiteral("Burundi")},
              {QStringLiteral("KH"), QStringLiteral("Cambodia")},
              {QStringLiteral("CM"), QStringLiteral("Cameroon")},
              {QStringLiteral("CA"), QStringLiteral("Canada")},
              {QStringLiteral("CV"), QStringLiteral("Cape Verde")},
              {QStringLiteral("KY"), QStringLiteral("Cayman Islands")},
              {QStringLiteral("CF"), QStringLiteral("Central African Republic")},
              {QStringLiteral("TD"), QStringLiteral("Chad")},
              {QStringLiteral("CL"), QStringLiteral("Chile")},
              {QStringLiteral("CN"), QStringLiteral("China")},
              {QStringLiteral("CX"), QStringLiteral("Christmas Island")},
              {QStringLiteral("CC"), QStringLiteral("Cocos (Keeling) Islands")},
              {QStringLiteral("CO"), QStringLiteral("Colombia")},
              {QStringLiteral("KM"), QStringLiteral("Comoros")},
              {QStringLiteral("CG"), QStringLiteral("Congo")},
              {QStringLiteral("CD"), QStringLiteral("Congo, the Democratic Republic of the")},
              {QStringLiteral("CK"), QStringLiteral("Cook Islands")},
              {QStringLiteral("CR"), QStringLiteral("Costa Rica")},
              {QStringLiteral("CI"), QStringLiteral("Côte d'Ivoire")},
              {QStringLiteral("HR"), QStringLiteral("Croatia")},
              {QStringLiteral("CU"), QStringLiteral("Cuba")},
              {QStringLiteral("CY"), QStringLiteral("Cyprus")},
              {QStringLiteral("CZ"), QStringLiteral("Czech Republic")},
              {QStringLiteral("DK"), QStringLiteral("Denmark")},
              {QStringLiteral("DJ"), QStringLiteral("Djibouti")},
              {QStringLiteral("DM"), QStringLiteral("Dominica")},
              {QStringLiteral("DO"), QStringLiteral("Dominican Republic")},
              {QStringLiteral("EC"), QStringLiteral("Ecuador")},
              {QStringLiteral("EG"), QStringLiteral("Egypt")},
              {QStringLiteral("SV"), QStringLiteral("El Salvador")},
              {QStringLiteral("GQ"), QStringLiteral("Equatorial Guinea")},
              {QStringLiteral("ER"), QStringLiteral("Eritrea")},
              {QStringLiteral("EE"), QStringLiteral("Estonia")},
              {QStringLiteral("ET"), QStringLiteral("Ethiopia")},
              {QStringLiteral("FK"), QStringLiteral("Falkland Islands (Malvinas)")},
              {QStringLiteral("FO"), QStringLiteral("Faroe Islands")},
              {QStringLiteral("FJ"), QStringLiteral("Fiji")},
              {QStringLiteral("FI"), QStringLiteral("Finland")},
              {QStringLiteral("FR"), QStringLiteral("France")},
              {QStringLiteral("GF"), QStringLiteral("French Guiana")},
              {QStringLiteral("PF"), QStringLiteral("French Polynesia")},
              {QStringLiteral("TF"), QStringLiteral("French Southern Territories")},
              {QStringLiteral("GA"), QStringLiteral("Gabon")},
              {QStringLiteral("GM"), QStringLiteral("Gambia")},
              {QStringLiteral("GE"), QStringLiteral("Georgia")},
              {QStringLiteral("DE"), QStringLiteral("Germany")},
              {QStringLiteral("GH"), QStringLiteral("Ghana")},
              {QStringLiteral("GI"), QStringLiteral("Gibraltar")},
              {QStringLiteral("GR"), QStringLiteral("Greece")},
              {QStringLiteral("GL"), QStringLiteral("Greenland")},
              {QStringLiteral("GD"), QStringLiteral("Grenada")},
              {QStringLiteral("GP"), QStringLiteral("Guadeloupe")},
              {QStringLiteral("GU"), QStringLiteral("Guam")},
              {QStringLiteral("GT"), QStringLiteral("Guatemala")},
              {QStringLiteral("GG"), QStringLiteral("Guernsey")},
              {QStringLiteral("GN"), QStringLiteral("Guinea")},
              {QStringLiteral("GW"), QStringLiteral("Guinea-Bissau")},
              {QStringLiteral("GY"), QStringLiteral("Guyana")},
              {QStringLiteral("HT"), QStringLiteral("Haiti")},
              {QStringLiteral("HM"), QStringLiteral("Heard Island and McDonald Islands")},
              {QStringLiteral("VA"), QStringLiteral("Holy See (Vatican City State)")},
              {QStringLiteral("HN"), QStringLiteral("Honduras")},
              {QStringLiteral("HK"), QStringLiteral("Hong Kong")},
              {QStringLiteral("HU"), QStringLiteral("Hungary")},
              {QStringLiteral("IS"), QStringLiteral("Iceland")},
              {QStringLiteral("IN"), QStringLiteral("India")},
              {QStringLiteral("ID"), QStringLiteral("Indonesia")},
              {QStringLiteral("IR"), QStringLiteral("Iran")},
              {QStringLiteral("IQ"), QStringLiteral("Iraq")},
              {QStringLiteral("IE"), QStringLiteral("Ireland")},
              {QStringLiteral("IM"), QStringLiteral("Isle of Man")},
              {QStringLiteral("IL"), QStringLiteral("Israel")},
              {QStringLiteral("IT"), QStringLiteral("Italy")},
              {QStringLiteral("JM"), QStringLiteral("Jamaica")},
              {QStringLiteral("JP"), QStringLiteral("Japan")},
              {QStringLiteral("JE"), QStringLiteral("Jersey")},
              {QStringLiteral("JO"), QStringLiteral("Jordan")},
              {QStringLiteral("KZ"), QStringLiteral("Kazakhstan")},
              {QStringLiteral("KE"), QStringLiteral("Kenya")},
              {QStringLiteral("KI"), QStringLiteral("Kiribati")},
              {QStringLiteral("KP"), QStringLiteral("Korea, Democratic People's Republic of")},
              {QStringLiteral("KR"), QStringLiteral("Korea, Republic of")},
              {QStringLiteral("KW"), QStringLiteral("Kuwait")},
              {QStringLiteral("KG"), QStringLiteral("Kyrgyzstan")},
              {QStringLiteral("LA"), QStringLiteral("Lao People's Democratic Republic")},
              {QStringLiteral("LV"), QStringLiteral("Latvia")},
              {QStringLiteral("LB"), QStringLiteral("Lebanon")},
              {QStringLiteral("LS"), QStringLiteral("Lesotho")},
              {QStringLiteral("LR"), QStringLiteral("Liberia")},
              {QStringLiteral("LY"), QStringLiteral("Libyan Arab Jamahiriya")},
              {QStringLiteral("LI"), QStringLiteral("Liechtenstein")},
              {QStringLiteral("LT"), QStringLiteral("Lithuania")},
              {QStringLiteral("LU"), QStringLiteral("Luxembourg")},
              {QStringLiteral("MO"), QStringLiteral("Macao")},
              {QStringLiteral("MK"), QStringLiteral("Macedonia, the former Yugoslav Republic of")},
              {QStringLiteral("MG"), QStringLiteral("Madagascar")},
              {QStringLiteral("MW"), QStringLiteral("Malawi")},
              {QStringLiteral("MY"), QStringLiteral("Malaysia")},
              {QStringLiteral("MV"), QStringLiteral("Maldives")},
              {QStringLiteral("ML"), QStringLiteral("Mali")},
              {QStringLiteral("MT"), QStringLiteral("Malta")},
              {QStringLiteral("MH"), QStringLiteral("Marshall Islands")},
              {QStringLiteral("MQ"), QStringLiteral("Martinique")},
              {QStringLiteral("MR"), QStringLiteral("Mauritania")},
              {QStringLiteral("MU"), QStringLiteral("Mauritius")},
              {QStringLiteral("YT"), QStringLiteral("Mayotte")},
              {QStringLiteral("MX"), QStringLiteral("Mexico")},
              {QStringLiteral("FM"), QStringLiteral("Micronesia, Federated States of")},
              {QStringLiteral("MD"), QStringLiteral("Moldova, Republic of")},
              {QStringLiteral("MC"), QStringLiteral("Monaco")},
              {QStringLiteral("MN"), QStringLiteral("Mongolia")},
              {QStringLiteral("ME"), QStringLiteral("Montenegro")},
              {QStringLiteral("MS"), QStringLiteral("Montserrat")},
              {QStringLiteral("MA"), QStringLiteral("Morocco")},
              {QStringLiteral("MZ"), QStringLiteral("Mozambique")},
              {QStringLiteral("MM"), QStringLiteral("Myanmar")},
              {QStringLiteral("NA"), QStringLiteral("Namibia")},
              {QStringLiteral("NR"), QStringLiteral("Nauru")},
              {QStringLiteral("NP"), QStringLiteral("Nepal")},
              {QStringLiteral("NL"), QStringLiteral("Netherlands")},
              {QStringLiteral("AN"), QStringLiteral("Netherlands Antilles")},
              {QStringLiteral("NC"), QStringLiteral("New Caledonia")},
              {QStringLiteral("NZ"), QStringLiteral("New Zealand")},
              {QStringLiteral("NI"), QStringLiteral("Nicaragua")},
              {QStringLiteral("NE"), QStringLiteral("Niger")},
              {QStringLiteral("NG"), QStringLiteral("Nigeria")},
              {QStringLiteral("NU"), QStringLiteral("Niue")},
              {QStringLiteral("NF"), QStringLiteral("Norfolk Island")},
              {QStringLiteral("MP"), QStringLiteral("Northern Mariana Islands")},
              {QStringLiteral("NO"), QStringLiteral("Norway")},
              {QStringLiteral("OM"), QStringLiteral("Oman")},
              {QStringLiteral("PK"), QStringLiteral("Pakistan")},
              {QStringLiteral("PW"), QStringLiteral("Palau")},
              {QStringLiteral("PS"), QStringLiteral("Palestinian Territory, Occupied")},
              {QStringLiteral("PA"), QStringLiteral("Panama")},
              {QStringLiteral("PG"), QStringLiteral("Papua New Guinea")},
              {QStringLiteral("PY"), QStringLiteral("Paraguay")},
              {QStringLiteral("PE"), QStringLiteral("Peru")},
              {QStringLiteral("PH"), QStringLiteral("Philippines")},
              {QStringLiteral("PN"), QStringLiteral("Pitcairn")},
              {QStringLiteral("PL"), QStringLiteral("Poland")},
              {QStringLiteral("PT"), QStringLiteral("Portugal")},
              {QStringLiteral("PR"), QStringLiteral("Puerto Rico")},
              {QStringLiteral("QA"), QStringLiteral("Qatar")},
              {QStringLiteral("RE"), QStringLiteral("Réunion")},
              {QStringLiteral("RO"), QStringLiteral("Romania")},
              {QStringLiteral("RU"), QStringLiteral("Russian Federation")},
              {QStringLiteral("RW"), QStringLiteral("Rwanda")},
              {QStringLiteral("BL"), QStringLiteral("Saint Barthélemy")},
              {QStringLiteral("SH"), QStringLiteral("Saint Helena, Ascension and Tristan da Cunha")},
              {QStringLiteral("KN"), QStringLiteral("Saint Kitts and Nevis")},
              {QStringLiteral("LC"), QStringLiteral("Saint Lucia")},
              {QStringLiteral("MF"), QStringLiteral("Saint Martin (French part)")},
              {QStringLiteral("PM"), QStringLiteral("Saint Pierre and Miquelon")},
              {QStringLiteral("VC"), QStringLiteral("Saint Vincent and the Grenadines")},
              {QStringLiteral("WS"), QStringLiteral("Samoa")},
              {QStringLiteral("SM"), QStringLiteral("San Marino")},
              {QStringLiteral("ST"), QStringLiteral("Sao Tome and Principe")},
              {QStringLiteral("SA"), QStringLiteral("Saudi Arabia")},
              {QStringLiteral("SN"), QStringLiteral("Senegal")},
              {QStringLiteral("RS"), QStringLiteral("Serbia")},
              {QStringLiteral("SC"), QStringLiteral("Seychelles")},
              {QStringLiteral("SL"), QStringLiteral("Sierra Leone")},
              {QStringLiteral("SG"), QStringLiteral("Singapore")},
              {QStringLiteral("SK"), QStringLiteral("Slovakia")},
              {QStringLiteral("SI"), QStringLiteral("Slovenia")},
              {QStringLiteral("SB"), QStringLiteral("Solomon Islands")},
              {QStringLiteral("SO"), QStringLiteral("Somalia")},
              {QStringLiteral("ZA"), QStringLiteral("South Africa")},
              {QStringLiteral("GS"), QStringLiteral("South Georgia and the South Sandwich Islands")},
              {QStringLiteral("ES"), QStringLiteral("Spain")},
              {QStringLiteral("LK"), QStringLiteral("Sri Lanka")},
              {QStringLiteral("SD"), QStringLiteral("Sudan")},
              {QStringLiteral("SR"), QStringLiteral("Suriname")},
              {QStringLiteral("SJ"), QStringLiteral("Svalbard and Jan Mayen")},
              {QStringLiteral("SZ"), QStringLiteral("Swaziland")},
              {QStringLiteral("SE"), QStringLiteral("Sweden")},
              {QStringLiteral("CH"), QStringLiteral("Switzerland")},
              {QStringLiteral("SY"), QStringLiteral("Syrian Arab Republic")},
              {QStringLiteral("TW"), QStringLiteral("Taiwan")},
              {QStringLiteral("TJ"), QStringLiteral("Tajikistan")},
              {QStringLiteral("TZ"), QStringLiteral("Tanzania")},
              {QStringLiteral("TH"), QStringLiteral("Thailand")},
              {QStringLiteral("TL"), QStringLiteral("Timor-Leste")},
              {QStringLiteral("TG"), QStringLiteral("Togo")},
              {QStringLiteral("TK"), QStringLiteral("Tokelau")},
              {QStringLiteral("TO"), QStringLiteral("Tonga")},
              {QStringLiteral("TT"), QStringLiteral("Trinidad and Tobago")},
              {QStringLiteral("TN"), QStringLiteral("Tunisia")},
              {QStringLiteral("TR"), QStringLiteral("Turkey")},
              {QStringLiteral("TM"), QStringLiteral("Turkmenistan")},
              {QStringLiteral("TC"), QStringLiteral("Turks and Caicos Islands")},
              {QStringLiteral("TV"), QStringLiteral("Tuvalu")},
              {QStringLiteral("UG"), QStringLiteral("Uganda")},
              {QStringLiteral("UA"), QStringLiteral("Ukraine")},
              {QStringLiteral("AE"), QStringLiteral("United Arab Emirates")},
              {QStringLiteral("GB"), QStringLiteral("United Kingdom")},
              {QStringLiteral("US"), QStringLiteral("United States")},
              {QStringLiteral("UM"), QStringLiteral("United States Minor Outlying Islands")},
              {QStringLiteral("UY"), QStringLiteral("Uruguay")},
              {QStringLiteral("UZ"), QStringLiteral("Uzbekistan")},
              {QStringLiteral("VU"), QStringLiteral("Vanuatu")},
              {QStringLiteral("VE"), QStringLiteral("Venezuela")},
              {QStringLiteral("VN"), QStringLiteral("Viet Nam")},
              {QStringLiteral("VG"), QStringLiteral("Virgin Islands, British")},
              {QStringLiteral("VI"), QStringLiteral("Virgin Islands, U.S.")},
              {QStringLiteral("WF"), QStringLiteral("Wallis and Futuna")},
              {QStringLiteral("EH"), QStringLiteral("Western Sahara")},
              {QStringLiteral("YE"), QStringLiteral("Yemen")},
              {QStringLiteral("ZM"), QStringLiteral("Zambia")},
              {QStringLiteral("ZW"), QStringLiteral("Zimbabwe")},
              {QStringLiteral("EA"), QStringLiteral("Ceuta and Melilla")},
              {QStringLiteral("XK"), QStringLiteral("Kosovo")},
          })
    {
    }

    ~KSettingsSchedulesPrivate()
    {
        delete ui;
    }

    Ui::KSettingsSchedules *ui;
    const QMap<QString, QString> m_countryCodesToCountryNames;
};

KSettingsSchedules::KSettingsSchedules(QWidget* parent) :
    QWidget(parent),
    d_ptr(new KSettingsSchedulesPrivate)
{
    Q_D(KSettingsSchedules);
    d->ui->setupUi(this);
    // hide the internally used holidayRegion field
    d->ui->kcfg_HolidayRegion->hide();

    loadList();

    // setup connections so that region gets selected once field is filled
    connect(d->ui->kcfg_HolidayRegion, &QLineEdit::textChanged, this, &KSettingsSchedules::slotLoadRegion);

    // setup connections so that changes are forwarded to the field
    connect(d->ui->m_holidayRegion, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KSettingsSchedules::slotSetRegion);
}

KSettingsSchedules::~KSettingsSchedules()
{
    Q_D(KSettingsSchedules);
    delete d;
}

void KSettingsSchedules::loadList()
{
    struct CompareByName : public std::binary_function<QString, QString, bool> {
        bool operator()(const QString& lhs, const QString& rhs) const
        {
            return QString::localeAwareCompare(lhs, rhs) < 0;
        }
    };

    Q_D(KSettingsSchedules);
    const auto model = qobject_cast<QStandardItemModel*>(d->ui->m_holidayRegion->model());

    QStringList regionList;
#ifdef ENABLE_HOLIDAYS
    QStringList regionCodes = HolidayRegion::regionCodes();

    QSet<QString> m_regionalHolidays;

    /// @todo with KF5 >= 5.88 use KCountry to construct the list and get rid of the table above
    foreach (const QString &regionCode, regionCodes) {
        const auto countryCode(HolidayRegion::countryCode(regionCode).split(QLatin1Char('-')).at(0).toUpper());
        if (d->m_countryCodesToCountryNames.contains(countryCode)) {
            const auto countryName = d->m_countryCodesToCountryNames[countryCode];
            const auto regionName(HolidayRegion::name(regionCode));
            const QLocale langLocale(HolidayRegion::languageCode(regionCode));
            const auto languageName = QLocale().languageToString(langLocale.language());
            const auto description = HolidayRegion::description(regionCode);
            const QString region =
                !description.isEmpty() ? description : i18nc("Holiday region (region language)", "Holidays for %1 (%2)", regionName, languageName);

            m_regionalHolidays << QStringLiteral("%1\0%2\0%3").arg(countryName, region, regionCode);
        } else {
            qDebug() << "Country code for" << countryCode << regionCode << HolidayRegion::name(regionCode) << "is missing";
        }
    }

    regionList = m_regionalHolidays.values();
    std::sort(regionList.begin(), regionList.end(), CompareByName());
#endif

    model->clear();

    QString lastCountry;
    QStandardItem* item(new QStandardItem(i18nc("@item:inlistbox No holiday region selected", "None")));
    item->setData(QString(), Qt::UserRole);
    model->appendRow(item);

    QFont boldFont = item->font();
    boldFont.setBold(true);

    for (const auto& region : regionList) {
        const auto entries = region.split('\0');
        if (entries.at(0).compare(lastCountry)) {
            lastCountry = entries.at(0);
            item = new QStandardItem(lastCountry);
            item->setFlags(item->flags() & ~(Qt::ItemIsSelectable));
            item->setFont(boldFont);

            model->appendRow(item);
        }
        item = new QStandardItem(entries.at(1));
        item->setData(entries.at(2), Qt::UserRole);
        model->appendRow(item);
    }
}

void KSettingsSchedules::slotSetRegion(int idx)
{
    Q_D(KSettingsSchedules);
    const auto model = qobject_cast<QStandardItemModel*>(d->ui->m_holidayRegion->model());
    const auto index = model->index(idx, 0);
    if (index.isValid()) {
        d->ui->kcfg_HolidayRegion->setText(index.data(Qt::UserRole).toString());
    }
}

void KSettingsSchedules::slotLoadRegion(const QString &region)
{
    Q_D(KSettingsSchedules);
    // only need this once
    disconnect(d->ui->kcfg_HolidayRegion, &KLineEdit::textChanged, this, &KSettingsSchedules::slotLoadRegion);

    auto newRow = 0;
    if (!region.isEmpty()) {
        const auto model = qobject_cast<QStandardItemModel*>(d->ui->m_holidayRegion->model());
        const auto indexes = model->match(model->index(0, 0), Qt::UserRole, region, 1, Qt::MatchCaseSensitive | Qt::MatchRecursive | Qt::MatchWrap);
        if (!indexes.isEmpty()) {
            newRow = indexes.first().row();
        }
    }

    if (newRow != d->ui->m_holidayRegion->currentIndex()) {
        QSignalBlocker blocked(d->ui->m_holidayRegion);
        d->ui->m_holidayRegion->setCurrentIndex(newRow);
    }
}

void KSettingsSchedules::slotResetRegion()
{
    Q_D(KSettingsSchedules);
    slotLoadRegion(d->ui->kcfg_HolidayRegion->text());
}
