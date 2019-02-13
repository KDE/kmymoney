/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "internationalaccountidentifiertest.h"
#include "../ibanbic.h"
#include "../ibanbicdata.h"

#include <QtTest>
#include <KGlobal>
#include <KStandardDirs>

QTEST_MAIN(internationalAccountIdentifierTest);

bool internationalAccountIdentifierTest::dataInstalled(const QString& countryCode)
{
    ibanBicData data;
    return data.bankIdentifierLength(countryCode) != 0;
}

void internationalAccountIdentifierTest::initTestCase()
{
  // Called before the first testfunction is executed
}

void internationalAccountIdentifierTest::cleanupTestCase()
{
  // Called after the last testfunction was executed
}

void internationalAccountIdentifierTest::init()
{
  // Called before each testfunction is executed
}

void internationalAccountIdentifierTest::cleanup()
{
  // Called after every testfunction
}

void internationalAccountIdentifierTest::comparison()
{

}

void internationalAccountIdentifierTest::ibanChecksum_data()
{
  QTest::addColumn<QString>("iban");
  QTest::addColumn<bool>("testResult");

  QTest::newRow("KDE e.V.") << "DE82200700240066644600" << true;
  QTest::newRow("Invalid iban") << "DE82200700240066644601" << false;
  QTest::newRow("IBAN with letters") << "BH82AEHI21601643513576" << true;
}

void internationalAccountIdentifierTest::ibanChecksum()
{
  QFETCH(QString, iban);
  QFETCH(bool, testResult);

  QCOMPARE(payeeIdentifiers::ibanBic::validateIbanChecksum(iban), testResult);
}

void internationalAccountIdentifierTest::paperformatIban_data()
{
  QTest::addColumn<QString>("iban");
  QTest::addColumn<QString>("paperformat");

  /** Random ibans generated using http://www.mobilefish.com/services/random_iban_generator/random_iban_generator.php */
  QTest::newRow("AL55359338525014419438694535") << "AL55359338525014419438694535" << "AL55 3593 3852 5014 4194 3869 4535";
  QTest::newRow("AD6507599863323512292387")     << "AD6507599863323512292387"     << "AD65 0759 9863 3235 1229 2387";
  QTest::newRow("AT550611200130969602")         << "AT550611200130969602"         << "AT55 0611 2001 3096 9602";
  QTest::newRow("AZ45YKNN50322618666505570288") << "AZ45YKNN50322618666505570288" << "AZ45 YKNN 5032 2618 6665 0557 0288";
  QTest::newRow("BH82AEHI21601643513576")       << "BH82AEHI21601643513576"       << "BH82 AEHI 2160 1643 5135 76";
  QTest::newRow("FR1767089178626632115068411")  << "FR1767089178626632115068411"  << "FR17 6708 9178 6266 3211 5068 411";
  QTest::newRow("GE87UH3052380574220575")       << "GE87UH3052380574220575"       << "GE87 UH30 5238 0574 2205 75";
  QTest::newRow("DE88476823289460743695")       << "DE88476823289460743695"       << "DE88 4768 2328 9460 7436 95";
  QTest::newRow("it81K2055156417927233643224")  << "it81K2055156417927233643224"  << "IT81 K205 5156 4179 2723 3643 224";

  // Unfinished ibans
  QTest::newRow("PK")                           << "PK"                           << "PK";
  QTest::newRow("NO5194556")                    << "NO5194556"                    << "NO51 9455 6";

  // Non canonical ibans
  QTest::newRow("VG33 NRCC 0371 8957 2076 3593") << "VG33 NRCC 0371 8957 2076 3593" << "VG33 NRCC 0371 8957 2076 3593";
  QTest::newRow(" SI 4523 946 27 23 327 14 9  ") << " SI 4523 946 27 23 327 14 9  " << "SI45 2394 6272 3327 149";
  QTest::newRow("MK 3-27/3287/612--98207//26")  << "MK 3-27/3287/612--98207//26"  << "MK32 7328 7612 9820 726";
}

void internationalAccountIdentifierTest::paperformatIban()
{
  QFETCH(QString, iban);
  QFETCH(QString, paperformat);

  QCOMPARE(payeeIdentifiers::ibanBic::ibanToPaperformat(iban), paperformat);
}

void internationalAccountIdentifierTest::electronicformatIban_data()
{
  QTest::addColumn<QString>("iban");
  QTest::addColumn<QString>("electronic");

  QTest::newRow("AL89 5112 2491 7164 1236 8777 7047") << "AL89 5112 2491 7164 1236 8777 7047" << "AL89511224917164123687777047";
  QTest::newRow("AZ11 BIAH 1276 1568 9842 3064 6155") << "AZ11 BIAH 1276 1568 9842 3064 6155" << "AZ11BIAH12761568984230646155";
  QTest::newRow("AZ73  WMRx 62 73 6   823 2803 9705") << "AZ73  WMRx 62 73 6   823 2803 9705" << "AZ73WMRX6273682328039705";
  QTest::newRow("AZ55/MKDW-9866$8070(4022)5306 7865") << "AZ55/MKDW-9866$8070(4022)5306 7865" << "AZ55MKDW98668070402253067865";
  QTest::newRow("AZ 57                             ") << "AZ 57                             " << "AZ57";
  QTest::newRow("dk3958515811555611")                 << "dk3958515811555611"                 << "DK3958515811555611";
}

void internationalAccountIdentifierTest::electronicformatIban()
{
  QFETCH(QString, iban);
  QFETCH(QString, electronic);
  QCOMPARE(payeeIdentifiers::ibanBic::ibanToElectronic(iban), electronic);
}

void internationalAccountIdentifierTest::setIban_data()
{
  QTest::addColumn<QString>("iban");
  QTest::addColumn<QString>("electronic");
  QTest::addColumn<QString>("paperformat");

  QTest::newRow("AL89 5112 2491 7164 1236 8777 7047") << "AL89 5112 2491 7164 1236 8777 7047" << "AL89511224917164123687777047" << "AL89 5112 2491 7164 1236 8777 7047" ;
  QTest::newRow("AZ73  WMRX 62 73 6   823 2803 9705") << "AZ73  WMRX 62 73 6   823 2803 9705" << "AZ73WMRX6273682328039705"     << "AZ73 WMRX 6273 6823 2803 9705";
  QTest::newRow("AZ55/MKDW-9866$8070(4022)5306 7865") << "AZ55/MKDW-9866$8070(4022)5306 7865" << "AZ55MKDW98668070402253067865" << "AZ55 MKDW 9866 8070 4022 5306 7865";
  QTest::newRow("AZ 57                             ") << "AZ 57                             " << "AZ57"                         << "AZ57";
  QTest::newRow("DK3958515811555611")                 << "DK3958515811555611"                 << "DK3958515811555611"           << "DK39 5851 5811 5556 11";
}

void internationalAccountIdentifierTest::setIban()
{
  QFETCH(QString, iban);
  QFETCH(QString, electronic);
  QFETCH(QString, paperformat);

  payeeIdentifiers::ibanBic ident;
  ident.setIban(iban);
  QCOMPARE(ident.electronicIban(), electronic);
  QCOMPARE(ident.paperformatIban(), paperformat);
}

void internationalAccountIdentifierTest::setBic_data()
{
  QTest::addColumn<QString>("input");
  QTest::addColumn<QString>("normalized");
  QTest::addColumn<QString>("full");

  QTest::newRow("Lower case")     << "chasgb2lxXx" << "CHASGB2L"    << "CHASGB2LXXX";
  QTest::newRow("Arbitrary case") << "RZtIaT22263" << "RZTIAT22263" << "RZTIAT22263";
  QTest::newRow("Without XXX")    << "MARKDEFF"    << "MARKDEFF"    << "MARKDEFFXXX";
  QTest::newRow("With XXX")       << "MARKDEFFXXX" << "MARKDEFF"    << "MARKDEFFXXX";
  QTest::newRow("Arbitray bic")   << "GENODEF1JEV" << "GENODEF1JEV" << "GENODEF1JEV";
}

void internationalAccountIdentifierTest::setBic()
{
  QFETCH(QString, input);
  QFETCH(QString, normalized);
  QFETCH(QString, full);

  payeeIdentifiers::ibanBic ident;
  ident.setBic(input);
  QCOMPARE(ident.bic(), normalized);
  QCOMPARE(ident.fullBic(), full);
}

void internationalAccountIdentifierTest::equalOperator_data()
{
  QTest::addColumn<QString>("bic1");
  QTest::addColumn<QString>("iban1");
  QTest::addColumn<QString>("bic2");
  QTest::addColumn<QString>("iban2");
  QTest::addColumn<bool>("equals");

  QTest::newRow("equal") << "MARKDEFFXXX" << "DE88476823289460743695" << "MARKDEFF" << "        DE88 4768 2328 9460 7436 95" << true;
  QTest::newRow("BIC unequal") << "MARKDEFF001" << "DE884768  23289460743695" << "MARKDEFF" << "DE88 4768 2328 9460 7436 95" << false;
  QTest::newRow("IBAN unequal") << "MARKDEFFXXX" << "DE88476823289460743695" << "MARKDEFF" << "GE87UH3052380574220575" << false;
}

void internationalAccountIdentifierTest::equalOperator()
{
  QFETCH(QString, bic1);
  QFETCH(QString, iban1);
  QFETCH(QString, bic2);
  QFETCH(QString, iban2);
  QFETCH(bool, equals);

  payeeIdentifiers::ibanBic ident1;
  ident1.setBic(bic1);
  ident1.setIban(iban1);

  payeeIdentifiers::ibanBic ident2;
  ident2.setBic(bic2);
  ident2.setIban(iban2);

  if (equals)
    QCOMPARE(ident1, ident2);
  else
    QVERIFY(!(ident1 == ident2));
}

void internationalAccountIdentifierTest::uneqalOperator_data()
{
  equalOperator_data();
}

void internationalAccountIdentifierTest::uneqalOperator()
{
  QFETCH(QString, bic1);
  QFETCH(QString, iban1);
  QFETCH(QString, bic2);
  QFETCH(QString, iban2);
  QFETCH(bool, equals);

  payeeIdentifiers::ibanBic ident1;
  ident1.setBic(bic1);
  ident1.setIban(iban1);

  payeeIdentifiers::ibanBic ident2;
  ident2.setBic(bic2);
  ident2.setIban(iban2);

  if (equals)
    QVERIFY(!(ident1 != ident2));
  else
    QVERIFY(ident1 != ident2);
}

void internationalAccountIdentifierTest::getProperties_data()
{
  QTest::addColumn<QString>("countryCode");
  QTest::addColumn<int>("bbanLength");
  QTest::addColumn<int>("bankIdentifierLength");

  QTest::newRow("Germany")     << "DE" << 18 << 8;
  QTest::newRow("France")      << "FR" << 23 << 10;
  QTest::newRow("Switzerland") << "CH" << 17 << 5;
}

void internationalAccountIdentifierTest::getProperties()
{
  QFETCH(QString, countryCode);
  QFETCH(int, bbanLength);

  if (!dataInstalled(countryCode))
    QSKIP(qPrintable(QString("Could not find ibanBicData service for this country (was looking for \"%1\"). Did you install the services?").arg(countryCode)), SkipSingle);

  QCOMPARE(payeeIdentifiers::ibanBic::ibanLengthByCountry(countryCode), bbanLength + 4);
}

void internationalAccountIdentifierTest::iban2bic_data()
{
  QTest::addColumn<QString>("iban");
  QTest::addColumn<QString>("bic");

  QTest::newRow("Germany (Unicef Germany)")   << "DE57370205000000300000" << "BFSWDE33XXX";
  QTest::newRow("Switzerland (Unicef Swiss)") << "CH8809000000800072119"  << "POFICHBEXXX";
}

void internationalAccountIdentifierTest::iban2bic()
{
  QFETCH(QString, iban);
  QFETCH(QString, bic);

  if (!dataInstalled(iban.left(2)))
    QSKIP(qPrintable(QString("Could not find ibanBicData service for this country (was looking for \"%1\"). Did you install the services?").arg(iban)), SkipSingle);

  QCOMPARE(payeeIdentifiers::ibanBic::bicByIban(iban), bic);
}

void internationalAccountIdentifierTest::nameByBic_data()
{
  QTest::addColumn<QString>("bic");
  QTest::addColumn<QString>("name");

  QTest::newRow("Germany (Bundesbank)") << "MARKDEF1100" << "Bundesbank";
}

void internationalAccountIdentifierTest::nameByBic()
{
  QFETCH(QString, bic);
  QFETCH(QString, name);

  if (!dataInstalled(bic.mid(4, 2)))
    QSKIP(qPrintable(QString("Could not find ibanBicData service for this country (was looking for \"%1\"). Did you install the services?").arg(bic)), SkipSingle);

  QCOMPARE(payeeIdentifiers::ibanBic::institutionNameByBic(bic), name);
}

void internationalAccountIdentifierTest::bicAndNameByIban_data()
{
  QTest::addColumn<QString>("iban");
  QTest::addColumn<QString>("bic");

  QTest::newRow("Germany (Unicef Germany)")   << "DE57370205000000300000" << "BFSWDE33XXX";
  QTest::newRow("Switzerland (Unicef Swiss)") << "CH8809000000800072119"  << "POFICHBEXXX";
}

void internationalAccountIdentifierTest::bicAndNameByIban()
{
  QFETCH(QString, iban);
  QFETCH(QString, bic);

  QCOMPARE(payeeIdentifiers::ibanBic::bicByIban(iban), bic);
}

void internationalAccountIdentifierTest::qStringNullAndEmpty()
{
  const QString nullStr1;
  QVERIFY(nullStr1.isNull());
  QVERIFY(nullStr1.isEmpty());

  const QString nullStr2 = QString();
  QVERIFY(nullStr2.isEmpty());
  QVERIFY(nullStr2.isNull());

  const QString empty = QString("");
  QVERIFY(empty.isEmpty());
  QVERIFY(!empty.isNull());
}

Q_DECLARE_METATYPE(payeeIdentifiers::ibanBic::bicAllocationStatus);

void internationalAccountIdentifierTest::bicAllocated_data()
{
  QTest::addColumn<QString>("bic");
  QTest::addColumn<payeeIdentifiers::ibanBic::bicAllocationStatus>("allocated");

  QTest::newRow("Bundesbank")   << "MARKDEFFXXX" << payeeIdentifiers::ibanBic::bicAllocated;
  QTest::newRow("Not existing") << "DOOFDE12NOT" << payeeIdentifiers::ibanBic::bicNotAllocated;
  QTest::newRow("Unknown")      << "NODAFRTAFOR" << payeeIdentifiers::ibanBic::bicAllocationUncertain;
}

void internationalAccountIdentifierTest::bicAllocated()
{
  QFETCH(QString, bic);
  QFETCH(payeeIdentifiers::ibanBic::bicAllocationStatus, allocated);

  const QString countryCode = bic.mid(4, 2);
  if (!dataInstalled(countryCode))
    QSKIP(qPrintable(QString("Could not find ibanBicData service for this country (was looking for \"%1\"). Did you install the services?").arg(countryCode)), SkipSingle);

  QCOMPARE(payeeIdentifiers::ibanBic::isBicAllocated(bic), allocated);
}
