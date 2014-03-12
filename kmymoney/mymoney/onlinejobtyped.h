/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ONLINEJOBTYPED_H
#define ONLINEJOBTYPED_H

#include "onlinejob.h"

/**
 * @brief Convient template if you know the task type of an onlineJob
 *
 * To prevent using onlineJob.task<T>() repeatingly you can use this class
 * where task() has a defined return type.
 *
 * onlineJobTyped::isNull() is always false. All constructors will throw
 * onlineJobTyped::badCast if they fail.
 */
template<class T>
class KMM_MYMONEY_EXPORT onlineJobTyped : public onlineJob
{
    KMM_MYMONEY_UNIT_TESTABLE
public:
    explicit onlineJobTyped();

    /**
     * @brief Create typed onlineJob
     * 
     * @throws emptyTask if task == 0
     */
    explicit onlineJobTyped( T* task, const QString& id = MyMoneyObject::m_emptyId );

    /** @brief Copy constructor */
    onlineJobTyped( onlineJobTyped<T> const& other );

    /**
     * @brief Copy from onlineJob
     * 
     * @throws badTaskCast if task in other does not fit T
     * @throws emptyTask if other has no task
     */
    explicit onlineJobTyped(const onlineJob &other);

    /** @brief Copy constructor with new id */
    explicit onlineJobTyped( const QString &id, const onlineJobTyped<T>& other );

    inline T* task();
    inline const T* task() const;
    inline const T* constTask() const { return task(); }

    onlineJobTyped<T> operator =( onlineJobTyped<T> const& other );

private:
    T* m_taskSubType;

    /**
     * @brief Test if m_taskSubType != 0 and throws an exception if needed
     * @throws emptyTask if m_taskSubType == 0
     */
    void testForValidTask() const;
};

template<class T>
onlineJobTyped<T>::onlineJobTyped( )
    : onlineJob( new T() ),
      m_taskSubType( 0 )
{
    m_taskSubType = static_cast<T*>( onlineJob::task() );
}

template<class T>
onlineJobTyped<T>::onlineJobTyped( T* task, const QString& id )
    : onlineJob(task, id),
      m_taskSubType(task)
{
  if (task == 0)
    throw emptyTask(__FILE__, __LINE__);
}

template<class T>
onlineJobTyped<T>::onlineJobTyped( onlineJobTyped<T> const& other )
    : onlineJob(other),
      m_taskSubType( 0 )
{
    m_taskSubType = dynamic_cast<T*>(onlineJob::task());
    testForValidTask();
}

template<class T>
onlineJobTyped<T> onlineJobTyped<T>::operator =( onlineJobTyped<T> const& other )
{
    onlineJob::operator =(other);
    m_taskSubType = dynamic_cast<T*>(onlineJob::task());
    Q_ASSERT( m_taskSubType != 0 );
    return (*this);
}

template<class T>
onlineJobTyped<T>::onlineJobTyped(const onlineJob &other)
    : onlineJob( other )
{
    m_taskSubType = dynamic_cast<T*>(onlineJob::task()); // this can throw emptyTask
    if (m_taskSubType == 0)
        throw badTaskCast(__FILE__, __LINE__);
}

template<class T>
T* onlineJobTyped<T>::task()
{
    Q_ASSERT(m_taskSubType != 0);
    return m_taskSubType;
}

template<class T>
const T* onlineJobTyped<T>::task() const
{
    Q_ASSERT(m_taskSubType != 0);
    return m_taskSubType;
}

template<class T>
void onlineJobTyped<T>::testForValidTask() const
{
    if (m_taskSubType == 0)
        throw emptyTask(__FILE__, __LINE__);
}

#endif // ONLINEJOBTYPED_H
