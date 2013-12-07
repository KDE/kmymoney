#ifndef ONLINEJOBKNOWNTASKTEST_H
#define ONLINEJOBKNOWNTASKTEST_H

#include "QtCore/QObject"

#define KMM_MYMONEY_UNIT_TESTABLE friend class onlineJobKnownTaskTest;

class onlineJobKnownTaskTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();
  void cleanupTestCase();

  void copyContructor();
  void copyContructorFailure();
  void copyByAssignment();
};

#endif // ONLINEJOBKNOWNTASKTEST_H
