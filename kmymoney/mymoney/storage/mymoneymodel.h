/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef MYMONEYMODEL_H
#define MYMONEYMODEL_H

#include <QObject>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QVariant>
#include <QModelIndex>
#include <QRegularExpression>
#include <QDomDocument>
#include <QDomElement>

#include <QDebug>

#include "mymoneyenums.h"
#include "mymoneymodelbase.h"

#include "kmm_models_export.h"

class MyMoneyFile;

#if 0
template <typename T>
class TreeItemObject : public QSharedData
{
public:
    TreeItemObject(T data)
        : QSharedData()
        , objectData(data)
    {}

    TreeItemObject(const TreeItemObject& other)
        : QSharedData(other)
        , objectData(other.objectData)
    {
    }

    T   objectData;
};
#endif

template <typename T>
class TreeItem
{
public:
    TreeItem(T data, TreeItem<T>* parent = nullptr)
    {
      parentItem = parent;
      object = data;
    }

    ~TreeItem()
    {
      qDeleteAll(childItems);
    }

    TreeItem<T> *child(int row)
    {
      return childItems.value(row);
    }

    int childCount() const
    {
      return childItems.count();
    }

    void appendChild(TreeItem<T>* item)
    {
      childItems.append(item);
    }

    bool insertChild(int row, TreeItem<T>* item)
    {
      if (row < 0 || row > childItems.count())
        return false;

      childItems.insert(row, item);
      return true;
    }

    bool removeChild(int row)
    {
      if (row < 0 || row >= childItems.count())
        return false;

      delete childItems.takeAt(row);
      return true;
    }


    TreeItem<T> *parent()
    {
      return parentItem;
    }

    int row() const
    {
      if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem<T>*>(this));

      return 0;
    }


    T data() const
    {
      return object;
    }

    const T& constDataRef() const
    {
      return object;
    }

    T& dataRef()
    {
      return object;
    }

    void setData(const T& value)
    {
      object = value;
    }

private:
    T                                       object;
    QVector<TreeItem<T> *>                  childItems;
    TreeItem<T> *                           parentItem;
};

template <typename T>
class MyMoneyModel : public MyMoneyModelBase
{
public:
  explicit MyMoneyModel(QObject* parent, const QString& idLeadin, quint8 idSize)
      : MyMoneyModelBase(parent)
      , m_nextId(0)
      , m_idLeadin(idLeadin)
      , m_idSize(idSize)
      , m_dirty(false)
      , m_idMatchExp(QStringLiteral("^%1(\\d+)$").arg(m_idLeadin))
  {
    m_rootItem = new TreeItem<T>(T());
#if 0
    if ((parent == nullptr) || !parent->inherits("MyMoneyFile")) {
      qDebug() << "Warning: parent of any MyMoneyModel derivative must be the MyMoneyFile object";
    } else {
      m_file = dynamic_cast<MyMoneyFile*>(parent);
    }
#endif
  }

  virtual ~MyMoneyModel()
  {
    delete m_rootItem;
  }

  Qt::ItemFlags flags(const QModelIndex &index) const override
  {
    if (!index.isValid())
      return Qt::ItemFlags();
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
      return Qt::ItemFlags();

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
  {
    TreeItem<T>* parentItem;

    if (!parent.isValid())
      parentItem = m_rootItem;
    else
      parentItem = static_cast<TreeItem<T>*>(parent.internalPointer());

    TreeItem<T> *childItem = parentItem->child(row);
    if (childItem)
      return createIndex(row, column, childItem);
    else
      return QModelIndex();
  }

  QModelIndex parent(const QModelIndex &index) const override
  {
    if (!index.isValid())
      return QModelIndex();

    TreeItem<T> *childItem = static_cast<TreeItem<T>*>(index.internalPointer());
    TreeItem<T> *parentItem = childItem->parent();

    if (parentItem == m_rootItem)
      return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
  }

  int rowCount(const QModelIndex &parent = QModelIndex()) const override
  {
    TreeItem<T> *parentItem;

    if (!parent.isValid())
      parentItem = m_rootItem;
    else
      parentItem = static_cast<TreeItem<T>*>(parent.internalPointer());

    return parentItem->childCount();
  }

  bool insertRows(int startRow, int rows, const QModelIndex &parent = QModelIndex()) override
  {
    TreeItem<T> *parentItem;

    if (!parent.isValid())
      parentItem = m_rootItem;
    else
      parentItem = static_cast<TreeItem<T>*>(parent.internalPointer());

    if (startRow < 0 || startRow > parentItem->childCount())
      return false;

    beginInsertRows(parent, startRow, startRow + rows - 1);

    for (int row = 0; row < rows; ++row) {
      TreeItem<T> *newItem = new TreeItem<T>(T(), parentItem);
      if (!parentItem->insertChild(startRow, newItem))
        break;
    }

    endInsertRows();
    setDirty();
    return true;
  }

  bool removeRows(int startRow, int rows, const QModelIndex &parent = QModelIndex()) override
  {
    TreeItem<T> *parentItem;

    if (!parent.isValid())
      parentItem = m_rootItem;
    else
      parentItem = static_cast<TreeItem<T>*>(parent.internalPointer());

    if (startRow < 0 || startRow > parentItem->childCount())
      return false;

    beginRemoveRows(parent, startRow, startRow + rows - 1);

    for (int row = 0; row < rows; ++row) {
      if (!parentItem->removeChild(startRow))
        break;
    }

    endRemoveRows();
    setDirty();
    return true;
  }

  QModelIndex indexById(const QString& id) const
  {
    const QModelIndexList indexes = match(index(0, 0), eMyMoney::Model::Roles::IdRole, id, 1, Qt::MatchFixedString | Qt::MatchRecursive);
    if (indexes.isEmpty())
      return QModelIndex();
    return indexes.first();
  }

  T itemById(const QString& id) const
  {
    const QModelIndex idx = indexById(id);
    if (idx.isValid()) {
      return static_cast<TreeItem<T>*>(idx.internalPointer())->data();
    }
    return T();
  }

  T itemByIndex(const QModelIndex& idx) const
  {
    if (idx.isValid()) {
      return static_cast<TreeItem<T>*>(idx.internalPointer())->data();
    }
    return T();
  }

  QString nextId()
  {
    return QString("%1%2").arg(m_idLeadin).arg(++m_nextId, m_idSize, 10, QLatin1Char('0'));
  }

  void setDirty(bool dirty = true)
  {
    m_dirty = dirty;
  }

  bool isDirty() const
  {
    return m_dirty;
  }

  /**
    * clears all objects currently in the model
    */
  void unload()
  {
    beginResetModel();
    clearModelItems();
    // reset the next id to zero
    m_nextId = 0;
    m_dirty = false;
    endResetModel();
  }

  void load(const QMap<QString, T>& list)
  {
    beginResetModel();
    // first get rid of any existing entries
    clearModelItems();

    const auto itemCount = list.count();
    // reserve an extra slot for the null item
    insertRows(0, itemCount);

    // and don't count loading as a modification
    setDirty(false);
    m_nextId = 0;

    int row = 0;
    foreach (const auto item, list) {
      updateNextObjectId(item.id());
      static_cast<TreeItem<T>*>(index(row, 0).internalPointer())->dataRef() = item;
      ++row;
    }
    endResetModel();
    emit modelLoaded();

    qDebug() << "Model for" << m_idLeadin << "loaded with" << rowCount() << "items";
  }

  struct Worker
  {
    Worker() = default;
    virtual ~Worker() = default;
    virtual void operator()(const T& item) = 0;

  };

  struct xmlWriter : public Worker
  {
    xmlWriter(void (*writer)(const T&, QDomDocument&, QDomElement&), QDomDocument& document, QDomElement& element)
    : Worker()
    , m_writer(writer)
    , m_doc(document)
    , m_element(element) {}
    void operator()(const T& item) override { m_writer(item, m_doc, m_element); }

    void (*m_writer)(const T&, QDomDocument&, QDomElement&);
    QDomDocument& m_doc;
    QDomElement& m_element;
  };


  virtual int processItems(Worker *worker)
  {
    return processItems(worker, match(index(0, 0), eMyMoney::Model::Roles::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive));
  }

  int processItems(Worker *worker, const QModelIndexList& indexes)
  {
    for (int row = 0; row < indexes.count(); ++row) {
      worker->operator()(static_cast<TreeItem<T>*>(indexes.value(row).internalPointer())->constDataRef());
    }
    return indexes.count();
  }

  QList<T> itemList() const
  {
    QList<T> list;
    QModelIndexList indexes = match(index(0, 0), eMyMoney::Model::Roles::IdRole, m_idLeadin, -1, Qt::MatchStartsWith | Qt::MatchRecursive);
    for (int row = 0; row < indexes.count(); ++row) {
      list.append(static_cast<TreeItem<T>*>(indexes.value(row).internalPointer())->constDataRef());
    }
    return list;
  }

  void addItem(T& item)
  {
    const int row = rowCount();
    insertRows(row, 1);
    const QModelIndex idx = index(row, 0);
    // assign an ID and store the object and
    // make sure the caller receives the assigned ID
    item = T(nextId(), item);
    static_cast<TreeItem<T>*>(idx.internalPointer())->dataRef() = item;

    setDirty();
    emit dataChanged(idx, index(row, columnCount()-1));
  }

  void modifyItem(const T& item)
  {
    const QModelIndex idx = indexById(item.id());
    if (idx.isValid()) {
      static_cast<TreeItem<T>*>(idx.internalPointer())->dataRef() = item;
      setDirty();
      emit dataChanged(idx, index(idx.row(), columnCount(idx.parent())-1));
    }
  }

  void removeItem(const T& item)
  {
    const QModelIndex idx = indexById(item.id());
    if (idx.isValid()) {
      removeRow(idx.row(), idx.parent());
      setDirty();
    }
  }

  QModelIndexList indexListByName(const QString& name, const QModelIndex parent = QModelIndex()) const
  {
    return match(index(0, 0, parent), Qt::DisplayRole, name, 1, Qt::MatchFixedString | Qt::MatchCaseSensitive);
  }

  T itemByName(const QString& name, const QModelIndex parent = QModelIndex()) const
  {
    QModelIndexList indexes = indexListByName(name, parent);
    if (!indexes.isEmpty()) {
      return static_cast<TreeItem<T>*>(indexes.first().internalPointer())->data();
    }
    return T();
  }

protected:
  virtual void clearModelItems()
  {
    // get rid of any existing entries should they exist
    if (m_rootItem->childCount()) {
      delete m_rootItem;
      m_rootItem = new TreeItem<T>(T());
    }
  }

  virtual void updateNextObjectId(const QString& id)
  {
    QRegularExpressionMatch m = m_idMatchExp.match(id);
    if (m.hasMatch()) {
      const quint64 itemId = m.captured(1).toUInt();
      if (itemId > m_nextId) {
        m_nextId = itemId;
      }
    }
  }

protected:
  TreeItem<T> *           m_rootItem;
  MyMoneyFile*            m_file;
  quint64                 m_nextId;
  QString                 m_idLeadin;
  quint8                  m_idSize;
  bool                    m_dirty;
  QRegularExpression      m_idMatchExp;
};

#endif // MYMONEYMODEL_H
