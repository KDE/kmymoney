/*
 *    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kmm_codec.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QComboBox>
#include <QList>
#include <QLocale>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
/*
 * The following list has been extracted from
 * https://www.iana.org/assignments/character-sets/character-sets.xml
 */
static QMap<QString, int> nameToMib = {
    {"US-ASCII", 3},
    {"ISO-8859-1", 4},
    {"ISO-8859-2", 5},
    {"ISO-8859-3", 6},
    {"ISO-8859-4", 7},
    {"ISO-8859-5", 8},
    {"ISO-8859-6", 9},
    {"ISO-8859-7", 10},
    {"ISO-8859-8", 11},
    {"ISO-8859-9", 12},
    {"ISO-8859-10", 13},
    {"ISO_6937-2-add", 14},
    {"JIS_X0201", 15},
    {"JIS_Encoding", 16},
    {"Shift_JIS", 17},
    {"EUC-JP", 18},
    {"Extended_UNIX_Code_Fixed_Width_for_Japanese", 19},
    {"BS_4730", 20},
    {"SEN_850200_C", 21},
    {"IT", 22},
    {"ES", 23},
    {"DIN_66003", 24},
    {"NS_4551-1", 25},
    {"NF_Z_62-010", 26},
    {"ISO-10646-UTF-1", 27},
    {"ISO_646.basic:1983", 28},
    {"INVARIANT", 29},
    {"ISO_646.irv:1983", 30},
    {"NATS-SEFI", 31},
    {"NATS-SEFI-ADD", 32},
    {"NATS-DANO", 33},
    {"NATS-DANO-ADD", 34},
    {"SEN_850200_B", 35},
    {"KS_C_5601-1987", 36},
    {"ISO-2022-KR", 37},
    {"EUC-KR", 38},
    {"ISO-2022-JP", 39},
    {"JIS_C6220-1969-jp", 41},
    {"JIS_C6220-1969-ro", 42},
    {"PT", 43},
    {"greek7-old", 44},
    {"latin-greek", 45},
    {"NF_Z_62-010_(1973)", 46},
    {"Latin-greek-1", 47},
    {"ISO_5427", 48},
    {"JIS_C6226-1978", 49},
    {"BS_viewdata", 50},
    {"INIS", 51},
    {"INIS-8", 52},
    {"INIS-cyrillic", 53},
    {"ISO_5427:1981", 54},
    {"ISO_5428:1980", 55},
    {"GB_1988-80", 56},
    {"GB_2312-80", 57},
    {"NS_4551-2", 58},
    {"videotex-suppl", 59},
    {"PT2", 60},
    {"ES2", 61},
    {"MSZ_7795.3", 62},
    {"JIS_C6226-1983", 63},
    {"greek7", 64},
    {"ASMO_449", 65},
    {"iso-ir-90", 66},
    {"JIS_C6229-1984-a", 67},
    {"JIS_C6229-1984-b", 68},
    {"JIS_C6229-1984-b-add", 69},
    {"JIS_C6229-1984-hand", 70},
    {"JIS_C6229-1984-hand-add", 71},
    {"JIS_C6229-1984-kana", 72},
    {"ISO_2033-1983", 73},
    {"ANSI_X3.110-1983", 74},
    {"T.61-7bit", 75},
    {"T.61-8bit", 76},
    {"ECMA-cyrillic", 77},
    {"CSA_Z243.4-1985-1", 78},
    {"CSA_Z243.4-1985-2", 79},
    {"CSA_Z243.4-1985-gr", 80},
    {"ISO-8859-6-E", 81},
    {"ISO-8859-6-I", 82},
    {"T.101-G2", 83},
    {"ISO-8859-8-E", 84},
    {"ISO-8859-8-I", 85},
    {"CSN_369103", 86},
    {"JUS_I.B1.002", 87},
    {"IEC_P27-1", 88},
    {"JUS_I.B1.003-serb", 89},
    {"JUS_I.B1.003-mac", 90},
    {"greek-ccitt", 91},
    {"NC_NC00-10:81", 92},
    {"ISO_6937-2-25", 93},
    {"GOST_19768-74", 94},
    {"ISO_8859-supp", 95},
    {"ISO_10367-box", 96},
    {"latin-lap", 97},
    {"JIS_X0212-1990", 98},
    {"DS_2089", 99},
    {"us-dk", 100},
    {"dk-us", 101},
    {"KSC5636", 102},
    {"UNICODE-1-1-UTF-7", 103},
    {"ISO-2022-CN", 104},
    {"ISO-2022-CN-EXT", 105},
    {"UTF-8", 106},
    {"ISO-8859-15", 111},
    {"ISO-8859-16", 112},
    {"GBK", 113},
    {"GB18030", 114},
    {"OSD_EBCDIC_DF04_15", 115},
    {"OSD_EBCDIC_DF03_IRV", 116},
    {"OSD_EBCDIC_DF04_1", 117},
    {"ISO-10646-UCS-2", 1000},
    {"ISO-10646-UCS-4", 1001},
    {"ISO-10646-UCS-Basic", 1002},
    {"ISO-10646-Unicode-Latin1", 1003},
    {"ISO-10646-J-1", 1004},
    {"ISO-Unicode-IBM-1261", 1005},
    {"ISO-Unicode-IBM-1268", 1006},
    {"ISO-Unicode-IBM-1276", 1007},
    {"ISO-Unicode-IBM-1264", 1008},
    {"ISO-Unicode-IBM-1265", 1009},
    {"UNICODE-1-1", 1010},
    {"UTF-7", 1012},
    {"UTF-16BE", 1013},
    {"UTF-16LE", 1014},
    {"UTF-16", 1015},
    {"ISO-8859-1-Windows-3.0-Latin-1", 2000},
    {"ISO-8859-1-Windows-3.1-Latin-1", 2001},
    {"ISO-8859-2-Windows-Latin-2", 2002},
    {"ISO-8859-9-Windows-Latin-5", 2003},
    {"hp-roman8", 2004},
    {"Adobe-Standard-Encoding", 2005},
    {"Ventura-US", 2006},
    {"Ventura-International", 2007},
    {"DEC-MCS", 2008},
    {"IBM850", 2009},
    {"PC8-Danish-Norwegian", 2012},
    {"IBM862", 2013},
    {"PC8-Turkish", 2014},
    {"IBM-Symbols", 2015},
    {"IBM-Thai", 2016},
    {"HP-Legal", 2017},
    {"HP-Pi-font", 2018},
    {"HP-Math8", 2019},
    {"Adobe-Symbol-Encoding", 2020},
    {"HP-DeskTop", 2021},
    {"Ventura-Math", 2022},
    {"Microsoft-Publishing", 2023},
    {"Windows-31J", 2024},
    {"GB2312", 2025},
    {"Big5", 2026},
    {"macintosh", 2027},
    {"IBM037", 2028},
    {"IBM038", 2029},
    {"IBM273", 2030},
    {"IBM274", 2031},
    {"IBM275", 2032},
    {"IBM277", 2033},
    {"IBM278", 2034},
    {"IBM280", 2035},
    {"IBM281", 2036},
    {"IBM284", 2037},
    {"IBM285", 2038},
    {"IBM290", 2039},
    {"IBM297", 2040},
    {"IBM420", 2041},
    {"IBM423", 2042},
    {"IBM424", 2043},
    {"IBM437", 2011},
    {"IBM500", 2044},
    {"IBM851", 2045},
    {"IBM852", 2010},
    {"IBM855", 2046},
    {"IBM857", 2047},
    {"IBM860", 2048},
    {"IBM861", 2049},
    {"IBM863", 2050},
    {"IBM864", 2051},
    {"IBM865", 2052},
    {"IBM868", 2053},
    {"IBM869", 2054},
    {"IBM870", 2055},
    {"IBM871", 2056},
    {"IBM880", 2057},
    {"IBM891", 2058},
    {"IBM903", 2059},
    {"IBM904", 2060},
    {"IBM905", 2061},
    {"IBM918", 2062},
    {"IBM1026", 2063},
    {"EBCDIC-AT-DE", 2064},
    {"EBCDIC-AT-DE-A", 2065},
    {"EBCDIC-CA-FR", 2066},
    {"EBCDIC-DK-NO", 2067},
    {"EBCDIC-DK-NO-A", 2068},
    {"EBCDIC-FI-SE", 2069},
    {"EBCDIC-FI-SE-A", 2070},
    {"EBCDIC-FR", 2071},
    {"EBCDIC-IT", 2072},
    {"EBCDIC-PT", 2073},
    {"EBCDIC-ES", 2074},
    {"EBCDIC-ES-A", 2075},
    {"EBCDIC-ES-S", 2076},
    {"EBCDIC-UK", 2077},
    {"EBCDIC-US", 2078},
    {"UNKNOWN-8BIT", 2079},
    {"MNEMONIC", 2080},
    {"MNEM", 2081},
    {"VISCII", 2082},
    {"VIQR", 2083},
    {"KOI8-R", 2084},
    {"HZ-GB-2312", 2085},
    {"IBM775", 2087},
    {"KOI8-U", 2088},
    {"Amiga-1251", 2104},
    {"KOI7-switched", 2105},
};

static QMap<int, QString> mibToName;

QByteArray KMM_Codec::encodingForLocale()
{
    QStringDecoder decoder(QLocale::system().name().toUtf8());
    if (decoder.isValid()) {
        const auto name = decoder.name();
        if (name) {
            return name;
        }
    }
    return "UTF-8";
}

QStringList KMM_Codec::availableCodecs()
{
    return QStringConverter::availableCodecs();
}

QList<int> KMM_Codec::availableMibs()
{
    QList<int> mibs;
    QStringList upperCaseCodecNames;
    const auto codecs = availableCodecs();
    for (const auto& codec : codecs) {
        upperCaseCodecNames << codec.toUpper();
    }
    QMap<QString, int>::const_iterator it;
    for (it = nameToMib.cbegin(); it != nameToMib.cend(); ++it) {
        if (upperCaseCodecNames.contains(it.key().toUpper())) {
            mibs << it.value();
        }
    }
    return mibs;
}

QString KMM_Codec::codecNameForMib(int mib)
{
    // fill map if not initialized
    if (mibToName.isEmpty()) {
        QMap<QString, int>::const_iterator it;
        for (it = nameToMib.cbegin(); it != nameToMib.cend(); ++it) {
            mibToName.insert(it.value(), it.key());
        }
    }
    const auto it = mibToName.find(mib);
    if (it != mibToName.cend()) {
        return it.value();
    }
    return QLatin1String("UTF-8");
}

int KMM_Codec::mibForCodecName(const QString& name)
{
    const auto it = nameToMib.find(name);
    if (it != nameToMib.cend()) {
        return it.value();
    }
    return -1;
}

static bool hasEncodingForName(const QString& codecName)
{
    return QStringDecoder(codecName.toUtf8()).isValid();
}

#else

QByteArray KMM_Codec::encodingForLocale()
{
    return QTextCodec::codecForLocale()->name();
}

QStringList KMM_Codec::availableCodecs()
{
    const auto codecs = QTextCodec::availableCodecs();
    QStringList codecNames;
    for (const auto& codec : codecs) {
        codecNames << QString(codec);
    }
    return codecNames;
}

QList<int> KMM_Codec::availableMibs()
{
    return QTextCodec::availableMibs();
}

QString KMM_Codec::codecNameForMib(int mib)
{
    return QTextCodec::codecForMib(mib)->name();
}

int KMM_Codec::mibForCodecName(const QString& name)
{
    const auto codec = QTextCodec::codecForName(name.toLatin1());
    return (codec) ? codec->mibEnum() : -1;
}

static bool hasEncodingForName(const QString& codecName)
{
    return (KMM_Codec::mibForCodecName(codecName) != -1);
}
#endif

void KMM_Codec::loadComboBox(QComboBox* cb)
{
    cb->clear();

    QMap<QString, QString> codecMap;
    static const QRegularExpression iso8859RegExp(QLatin1String("ISO[- ]8859-([0-9]+).*"));

    const auto availableMibs = KMM_Codec::availableMibs();
    for (const auto& mib : qAsConst(availableMibs)) {
        const auto codecName = codecNameForMib(mib);
        if (hasEncodingForName(codecName)) {
            auto sortKey = codecName.toUpper();
            int rank;

            const auto iso8859(iso8859RegExp.match(sortKey));
            if (sortKey.startsWith(QLatin1String("UTF-8"))) { // krazy:exclude=strings
                rank = 1;
            } else if (sortKey.startsWith(QLatin1String("UTF-16"))) { // krazy:exclude=strings
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

            codecMap.insert(sortKey, codecName);
        }
    }

    QMap<QString, QString>::const_iterator it;
    for (it = codecMap.cbegin(); it != codecMap.cend(); ++it) {
        const auto mib = mibForCodecName(it.value());
        if (mib != -1) {
            cb->addItem(it.value(), mib);
        }
    }
}
