/*
    SPDX-FileCopyrightText: 2002-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYKEYVALUECONTAINER_H
#define MYMONEYKEYVALUECONTAINER_H

#include "kmm_mymoney_export.h"

/**
  * @author Thomas Baumgart
  */

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <qglobal.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyunittestable.h"

class QString;

template <class Key, class Value> class QMap;

/**
  * This class implements a container for key/value pairs. This is used
  * to store an arbitrary number of attributes with any of the engine
  * object. The container can also be used to have attributes that are
  * attached to this object only for a limited time (e.g. between
  * start of reconciliation end it's end).
  *
  * To give any class the ability to have a key/value pair container,
  * just derive the class from this one. See MyMoneyAccount as an example.
  */

class MyMoneyKeyValueContainerPrivate;
class KMM_MYMONEY_EXPORT MyMoneyKeyValueContainer
{
    Q_DECLARE_PRIVATE(MyMoneyKeyValueContainer)
    KMM_MYMONEY_UNIT_TESTABLE

protected:
    MyMoneyKeyValueContainerPrivate * d_ptr;

public:
    MyMoneyKeyValueContainer();

    MyMoneyKeyValueContainer(const MyMoneyKeyValueContainer & other);
    MyMoneyKeyValueContainer(MyMoneyKeyValueContainer && other);
    MyMoneyKeyValueContainer & operator=(MyMoneyKeyValueContainer other);
    friend void swap(MyMoneyKeyValueContainer& first, MyMoneyKeyValueContainer& second);
    virtual ~MyMoneyKeyValueContainer();

    /**
     * This method can be used to retrieve the value for a specific @p key.
     * If the key is unknown in this container, an empty string will be returned.
     *
     * @param key const reference to QString with the key to search for
     * @param defaultValue const reference to the default value
     * @return value of this key. If the key does not exist,
     *         the @a defaultValue is returned.
     */
    QString value(const QString& key, const QString& defaultValue) const;

    /**
     * This method can be used to retrieve the value for a specific @p key.
     * If the key is unknown in this container, an empty string will be returned.
     *
     * @param key const reference to QString with the key to search for
     * @return value of this key. If the key does not exist,
     *         an empty string is returned.
     *
     * This is a convenience function for value(key, defaultValue)
     */
    QString value(const QString& key) const;

    /**
     * convenience method for type @c bool
     */
    bool value(const QString& key, bool defaultValue) const;

    /**
     * convenience method for type @c int
     */
    int value(const QString& key, int defaultValue) const;

    /**
     * convenience method for @c enum
     */
    template<typename T>
    T value(const QString& key, T defaultValue) const
    {
        return static_cast<T>(value(key, static_cast<int>(defaultValue)));
    }

    /**
     * This method is used to add a key/value pair to the container or
     * modify an existing pair.
     *
     * @param key const reference to QString with the key to search for
     * @param value const reference to QString with the value for this key
     * @param defaultValue when value has this defaultValue, the entry is removed
     */
    void setValue(const QString& key, const QString& value, const QString& defaultValue = QString());

    /**
     * convenience method for integral types (except bool)
     */
    template<typename T>
    void setValue(const QString& key, T newValue, T defaultValue)
    {
        return setValue_int(key, static_cast<int>(newValue), static_cast<int>(defaultValue));
    }

    /**
     * Convenience method for setValue(const QString& key, const QString& value, const QString& defaultValue)
     */
    void setValue(const QString& key, const char* value);

    /**
     * This method is used to add a key/value pair representing a boolean
     * value. If the @a value is equal to @a defaultValue the entry is
     * removed, otherwise it will be set to "yes" for @c true or "no"
     * for @c false.
     *
     * @param key const reference to QString with the key to search for
     * @param value the value for this key
     * @param defaultValue when value has this defaultValue, the entry is removed
     */
    void setValue(const QString& key, bool value, bool defaultValue);

    /**
      * This method is used to remove an existing key/value pair from the
      * container. If the key does not exist, the container is not changed.
      *
      * @param key const reference to QString with the key to remove
      */
    void deletePair(const QString& key);

    /**
      * This method clears all pairs currently in the container.
      */
    void clear();

    /**
      * This method is used to retrieve the whole set of key/value pairs
      * from the container. It is meant to be used for permanent storage
      * functionality.
      *
      * @return QMap<QString, QString> containing all key/value pairs of
      *         this container.
      */
    QMap<QString, QString> pairs() const;

    /**
      * This method is used to initially store a set of key/value pairs
      * in the container. It is meant to be used for loading functionality
      * from permanent storage.
      *
      * @param list const QMap<QString, QString> containing the set of
      *             key/value pairs to be loaded into the container.
      *
      * @note All existing key/value pairs in the container will be deleted.
      */
    void setPairs(const QMap<QString, QString>& list);

    /**
      * This operator tests for equality of two MyMoneyKeyValueContainer objects
      */
    bool operator == (const MyMoneyKeyValueContainer &) const;

    QString operator[](const QString& k) const;

    QString& operator[](const QString& k);

private:
    void setValue_int(const QString& key, int value, int defaultValue);
};

inline void swap(MyMoneyKeyValueContainer& first, MyMoneyKeyValueContainer& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyKeyValueContainer::MyMoneyKeyValueContainer(MyMoneyKeyValueContainer && other) : MyMoneyKeyValueContainer() // krazy:exclude=inline
{
    swap(*this, other);
}

inline MyMoneyKeyValueContainer & MyMoneyKeyValueContainer::operator=(MyMoneyKeyValueContainer other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

#endif
