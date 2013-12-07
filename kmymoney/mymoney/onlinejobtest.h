#ifndef ONLINEJOBTEST_H
#define ONLINEJOBTEST_H

#include <QObject>
#include <QtCore/QString>

class onlineJobTest : public QObject
{
    Q_OBJECT

private slots:
//    void initTestCase();
//    void cleanupTestCase();

  void testDefaultConstructor();
  void testCopyConstructor();
  void testCopyAssignment();
  void testCopyConstructorWithNewId();
};

#endif // ONLINEJOBTEST_H
