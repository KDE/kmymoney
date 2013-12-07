#include "onlinejobadministrationtest.h"

#include <QtTest/QTest>

#include "onlinejobadministration.h"
#include "mymoney/mymoneyfile.h"
#include "mymoney/storage/mymoneyseqaccessmgr.h"
#include "sepaonlinetransfer.h"

QTEST_MAIN(onlineJobAdministrationTest)

void onlineJobAdministrationTest::initTestCase()
{
    file = MyMoneyFile::instance();
    storage = new MyMoneySeqAccessMgr;
    file->attachStorage(storage);

    try {
        MyMoneyAccount account = MyMoneyAccount();
        account.setName( "Test Account" );
        account.setAccountType( MyMoneyAccount::Savings );
        MyMoneyAccount asset = file->asset();
        MyMoneyFileTransaction transaction;
        file->addAccount(account , asset );
        accountId = account.id();
        transaction.commit();
    } catch (MyMoneyException* ex) {
        qFatal( ex->what().toLatin1().constData() );
    }
}

void onlineJobAdministrationTest::cleanupTestCase()
{
    file->detachStorage(storage);
    delete storage;
}

void onlineJobAdministrationTest::addOnlineJob()
{
#if 0
    onlineJob job = new germanOnlineTransfer();
    onlineJobAdministration::instance()->makeOnlineJobAvailable( accountId, germanOnlineTransfer::name() );
//    Q_ASSERT( transfer != 0 );
#endif
}

void onlineJobAdministrationTest::getSettings()
{
  onlineJobAdministration::instance()->taskSettings<sepaOnlineTransfer::settings>( sepaOnlineTransfer::name(), "A000001" );
}
