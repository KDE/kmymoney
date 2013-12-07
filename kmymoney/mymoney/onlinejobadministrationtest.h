#ifndef ONLINEJOBADMINISTRATIONTEST_H
#define ONLINEJOBADMINISTRATIONTEST_H

#include <QObject>
#include <QtCore/QString>

class MyMoneyFile;
class IMyMoneyStorage;

class onlineJobAdministrationTest : public QObject
{
    Q_OBJECT

    IMyMoneyStorage* storage;
    MyMoneyFile* file;
    QString accountId;
private slots:
    void initTestCase();
    void cleanupTestCase();
    void addOnlineJob();
    void getSettings();
};

#endif // ONLINEJOBADMINISTRATIONTEST_H
