#ifndef ONLINEJOBADMINISTRATIONTEST_H
#define ONLINEJOBADMINISTRATIONTEST_H

#include <QObject>
#include <QtCore/QString>

class MyMoneyFile;
class IMyMoneyStorage;

#define KMM_MYMONEY_UNIT_TESTABLE friend class onlineJobAdministrationTest;

class onlineJobAdministrationTest : public QObject
{
    Q_OBJECT

    IMyMoneyStorage* storage;
    MyMoneyFile* file;
    QString accountId;
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void getSettings();
    void registerOnlineTask();
};

#endif // ONLINEJOBADMINISTRATIONTEST_H
