/***************************************************************************
                          mymoneymap.h
                             -------------------
    copyright            : (C) 2007-2008 by Thomas Baumgart
    email                : <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdint.h>
#include <QMap>
#include <QStack>
#include <mymoneyexception.h>

#ifndef MYMONEYMAP_H
#define MYMONEYMAP_H

#define MY_OWN_DEBUG 0

/**
  * @author Thomas Baumgart
  *
  * This template class adds transaction security to the QMap<> class.
  * The interface is very simple. Before you perform any changes,
  * you have to call the startTransaction() method. Then you can use
  * the insert(), modify() and remove() methods to modify the map.
  * Changes are recorded and if you are finished, use the
  * commitTransaction() to finish the transaction. If you want to go
  * back before you have committed the transaction, use
  * rollbackTransaction() to set the container to the state it was
  * in before you called startTransaction().
  *
  * The implementation is based on the command pattern, in case
  * someone is interested.
  */
template <class Key, class T>
class MyMoneyMap : protected QMap<Key, T>
{
private:
  // check if a key required (not already contained in the stack) or not
  bool required(const Key& key) const {
    if (m_stack.count() > 1) {
      for (int i = 0; i < m_stack.count(); ++i) {
        if (m_stack[i]->key() == key) {
          return false;
        }
      }
    }
    return true;
  }

public:
  MyMoneyMap() : QMap<Key, T>() {}
  ~MyMoneyMap() {}

  void startTransaction(unsigned long* id = 0) {
    m_stack.push(new MyMoneyMapStart(this, id));
  }

  void rollbackTransaction(void) {
    if (m_stack.count() == 0)
      throw MYMONEYEXCEPTION("No transaction started to rollback changes");

    // undo all actions
    MyMoneyMapAction* action;
    while (m_stack.count()) {
      action = m_stack.pop();
      action->undo();
      delete action;
    }
  }

  bool commitTransaction(void) {
    if (m_stack.count() == 0)
      throw MYMONEYEXCEPTION("No transaction started to commit changes");

    bool rc = m_stack.count() > 1;
    // remove all actions from the stack
    MyMoneyMapAction* action;
    while (m_stack.count()) {
      action = m_stack.pop();
      delete action;
    }
    return rc;
  }

  void insert(const Key& key, const T& obj) {
    if (m_stack.count() == 0)
      throw MYMONEYEXCEPTION("No transaction started to insert new element into container");

    // check if information about the object identified by 'key'
    // is already present in the stack
    if (!required(key)) {
      QMap<Key, T> *container = this;
      (*container)[key] = obj;
      return;
    }

    // store object in
    m_stack.push(new MyMoneyMapInsert(this, key, obj));
  }

  void modify(const Key& key, const T& obj) {
    if (m_stack.count() == 0)
      throw MYMONEYEXCEPTION("No transaction started to modify element in container");

#if 0
    // had to take this out, because we use QPair in one instance as key
    if (key.isEmpty())
      throw MYMONEYEXCEPTION("No key to update object");
#endif

    // check if information about the object identified by 'key'
    // is already present in the stack
    if (!required(key)) {
      QMap<Key, T> *container = this;
      (*container)[key] = obj;
      return;
    }

    m_stack.push(new MyMoneyMapModify(this, key, obj));
  }

  void remove(const Key& key) {
    if (m_stack.count() == 0)
      throw MYMONEYEXCEPTION("No transaction started to remove element from container");

#if 0
    // had to take this out, because we use QPair in one instance as key
    if (key.isEmpty())
      throw MYMONEYEXCEPTION("No key to remove object");
#endif

    // check if information about the object identified by 'key'
    // is already present in the stack
    if (!required(key)) {
      QMap<Key, T> *container = this;
      container->remove(key);
      return;
    }

    m_stack.push(new MyMoneyMapRemove(this, key));
  }

  MyMoneyMap<Key, T>& operator= (const QMap<Key, T>& m) {
    if (m_stack.count() != 0) {
      throw MYMONEYEXCEPTION("Cannot assign whole container during transaction");
    }
    QMap<Key, T>::operator=(m);
    return *this;
  }


  inline QList<T> values(void) const {
    return QMap<Key, T>::values();
  }

  inline QList<Key> keys(void) const {
    return QMap<Key, T>::keys();
  }

  const T& operator[](const Key& k) const {
    return find(k).value();
#if 0
    /*QT_CHECK_INVALID_MAP_ELEMENT;*/ /*PORT ME KDE4*/ return QMap<Key, T>::operator[](k);
#endif
  }

  inline typename QMap<Key, T>::const_iterator find(const Key& k) const {
    return QMap<Key, T>::find(k);
  }

  inline typename QMap<Key, T>::const_iterator begin(void) const {
    return QMap<Key, T>::constBegin();
  }

  inline typename QMap<Key, T>::const_iterator end(void) const {
    return QMap<Key, T>::constEnd();
  }

  typedef typename QMap<Key, T>::const_iterator const_iterator;

  inline bool contains(const Key& k) const {
    return find(k) != end();
  }

  inline void map(QMap<Key, T>& that) const {
    //QMap<Key, T>* ptr = dynamic_cast<QMap<Key, T>* >(this);
    //that = *ptr;
    that = *(dynamic_cast<QMap<Key, T>* >(const_cast<MyMoneyMap<Key, T>* >(this)));
  }

  inline int count(void) const {
    return QMap<Key, T>::count();
  }

#if MY_OWN_DEBUG
  void dump(void) const {
    printf("Container dump\n");
    printf(" items in container = %d\n", count());
    printf(" items on stack     = %d\n", m_stack.count());

    const_iterator it;
    for (it = begin(); it != end(); ++it) {
      printf("  %s \n", it.key().data());
    }
  }
#endif

private:
  class MyMoneyMapAction
  {
  public:
    MyMoneyMapAction(QMap<Key, T>* container) :
        m_container(container) {}

    MyMoneyMapAction(QMap<Key, T>* container, const Key& key, const T& obj) :
        m_container(container),
        m_obj(obj),
        m_key(key) {}

    virtual ~MyMoneyMapAction() {}
    virtual void undo(void) = 0;
    const Key& key(void) const {
      return m_key;
    }

  protected:
    QMap<Key, T>* m_container;
    T m_obj;
    Key m_key;
  };

  class MyMoneyMapStart : public MyMoneyMapAction
  {
  public:
    MyMoneyMapStart(QMap<Key, T>* container, unsigned long* id) :
        MyMoneyMapAction(container),
        m_idPtr(id),
        m_id(0) {
      if (id != 0)
        m_id = *id;
    }
    virtual ~MyMoneyMapStart() {}
    void undo(void) {
      if (m_idPtr != 0)
        *m_idPtr = m_id;
    }

  private:
    unsigned long* m_idPtr;
    unsigned long  m_id;
  };

  class MyMoneyMapInsert : public MyMoneyMapAction
  {
  public:
    MyMoneyMapInsert(QMap<Key, T>* container, const Key& key, const T& obj) :
        MyMoneyMapAction(container, key, obj) {
      (*container)[key] = obj;
    }

    virtual ~MyMoneyMapInsert() {}
    void undo(void) {
      // m_container->remove(m_key) does not work on GCC 4.0.2
      // using this-> to access those member does the trick
      this->m_container->remove(this->m_key);
    }
  };

  class MyMoneyMapRemove : public MyMoneyMapAction
  {
  public:
    MyMoneyMapRemove(QMap<Key, T>* container, const Key& key) :
        MyMoneyMapAction(container, key, (*container)[key]) {
      container->remove(key);
    }

    virtual ~MyMoneyMapRemove() {}
    void undo(void) {
      (*(this->m_container))[this->m_key] = this->m_obj;
    }
  };

  class MyMoneyMapModify : public MyMoneyMapAction
  {
  public:
    MyMoneyMapModify(QMap<Key, T>* container, const Key& key, const T& obj) :
        MyMoneyMapAction(container, key, (*container)[key]) {
      (*container)[key] = obj;
    }

    virtual ~MyMoneyMapModify() {}
    void undo(void) {
      (*(this->m_container))[this->m_key] = this->m_obj;
    }
  };

protected:
  QStack<MyMoneyMapAction *> m_stack;
};

#if MY_OWN_DEBUG
#include <mymoneyaccount.h>
#include <mymoneytransaction.h>
main()
{
  MyMoneyMap<QString, MyMoneyAccount> container;
  MyMoneyMap<QString, MyMoneyTransaction> ct;

  MyMoneyAccount acc;
  acc.setName("Test");
  // this should not be possible
  // container["a"] = acc;

  QList<MyMoneyAccount> list;
  list = container.values();

  MyMoneyAccount b;
  b.setName("Thomas");

  try {
    container.startTransaction();
    container.insert("001", acc);
    container.dump();
    container.commitTransaction();
    acc.setName("123");
    container.startTransaction();
    container.modify("001", acc);
    container.dump();
    container.rollbackTransaction();
    container.dump();

    container.startTransaction();
    container.remove(QString("001"));
    container.dump();
    container.rollbackTransaction();
    container.dump();

    b = container["001"];
    printf("b.name() = %s\n", b.name().data());

    QMap<QString, MyMoneyAccount>::ConstIterator it;
    it = container.find("001");
    it = container.begin();

  } catch (const MyMoneyException &e) {
    printf("Caught exception: %s\n", e.what().data());
  }

  QMap<QString, MyMoneyAccount> map;
  map["005"] = b;
  container = map;

  printf("b.name() = %s\n", container["001"].name().data());
  printf("b.name() = %s\n", container["005"].name().data());
}

#endif

#endif
