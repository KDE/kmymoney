#ifndef ONLINEJOBMODEL_H
#define ONLINEJOBMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

#include "mymoney/mymoneyfile.h"

class Models;
class onlineJobModel : public QAbstractTableModel
{
    Q_OBJECT
public: 
  /**
   * @brief Item Data roles for onlineJobs
   * In addition to Qt::ItemDataRole
   */
  enum roles {
    OnlineJobId = Qt::UserRole /**< QString of onlineJob.id() */
  };

  enum columns {
    ColAccount,
    ColAction,
    ColDestination,
    ColValue
  };

  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation , int role = Qt::DisplayRole) const;
  /** @brief Remove onlineJob identified by row */
  bool removeRow( int row, const QModelIndex & parent = QModelIndex() );
  /** @brief Remove onlineJobs identified by row and count */
  bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

signals:
    
public slots:
  void reloadAll();

  void slotObjectAdded(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj);
  void slotObjectModified(MyMoneyFile::notificationObjectT objType, const MyMoneyObject * const obj);
  void slotObjectRemoved(MyMoneyFile::notificationObjectT objType, const QString& id);

  /** @brief Load data from MyMoneyFile */
  void load();
  void unload();
  
protected:
  /** Only @ref Models should be able to construct this class */
  explicit onlineJobModel(QObject *parent = 0);
  friend class Models;
  
private:
  QStringList m_jobIdList;

};

#endif // ONLINEJOBMODEL_H
