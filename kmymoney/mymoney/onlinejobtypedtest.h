#ifndef ONLINEJOBTYPEDTEST_H
#define ONLINEJOBTYPEDTEST_H

#include "QtCore/QObject"

#define KMM_MYMONEY_UNIT_TESTABLE friend class onlineJobTypedTest;

class onlineJobTypedTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();
  void cleanupTestCase();

  void copyContructor();
  void copyContructorFailure();
  void copyByAssignment();
};

#endif // ONLINEJOBTYPEDTEST_H
