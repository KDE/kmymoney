#include "onlinetasks/interfaces/tasks/onlinetask.h"

class dummyTask : public onlineTask
{
public:
  ONLINETASK_META(dummyTask, "org.kmymoney.onlinetasks.dummy");
  dummyTask()
  : m_testNumber( 0 )
  {
    
  }
  
  dummyTask(const dummyTask& other)
  : m_testNumber( other.m_testNumber )
  {
  }
  
  /**
   * @brief Checks if the task is ready for sending
   */
  virtual bool isValid() const { return true; };
  
  /**
   * @brief Human readable type-name
   */
  virtual QString jobTypeName() const { return QLatin1String("Dummy task"); };
  
  void setTestNumber( const int& number ) { m_testNumber = number; }
  int testNumber() { return m_testNumber; }
  
protected:

  virtual dummyTask* clone() const { return (new dummyTask(*this)); }
  virtual bool hasReferenceTo(const QString &id) const { return false; }
  virtual void writeXML(QDomDocument&, QDomElement&) const {}
  virtual dummyTask* createFromXml(const QDomElement &element) const { return (new dummyTask); }
  virtual QString responsibleAccount() const { return QString(); };
  virtual convertType canConvertInto( const QString& onlineTaskName ) const { return onlineTask::convertImpossible; }
  virtual convertType canConvert( const QString& onlineTaskName ) const { return onlineTask::convertImpossible; }
  virtual dummyTask* convertInto( const QString& onlineTaskName, QString& messageString, bool& payeeChanged ) const { return 0; };
  virtual void convert( const onlineTask& task, QString& messageString, bool& payeeChanged ) {};
  
  int m_testNumber;
};
