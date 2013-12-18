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

    onlineJobTyped( T* task, const QString& id = MyMoneyObject::m_emptyId );

    /** @brief Copy constructor */
    onlineJobTyped( onlineJobTyped<T> const& other );

    /** @brief Copy from onlineJob */
    onlineJobTyped(const onlineJob &other);

    /** @brief Copy constructor with new id */
    onlineJobTyped( const QString &id, const onlineJobTyped<T>& other );

    inline T* task();
    inline const T* task() const;
    inline const T* constTask() const { return task(); }

    onlineJobTyped<T> operator =( onlineJobTyped<T> const& other );

private:
    T* m_taskSubType;

    /**
     * @brief Test if m_taskSubType != 0 and throws an exception if needed
     * @throws MyMoneyException if m_taskSubType == 0
     */
    void testForValidTask() const;
};

template<class T>
onlineJobTyped<T>::onlineJobTyped( )
    : onlineJob( new T() ),
      m_taskSubType( 0 )
{
    m_taskSubType = static_cast<T>( onlineJob::task() );
}

template<class T>
onlineJobTyped<T>::onlineJobTyped( T* task, const QString& id )
    : onlineJob(task, id),
      m_taskSubType(task)
{
    testForValidTask();
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
    m_taskSubType = dynamic_cast<T*>(onlineJob::task());
    if (m_taskSubType == 0)
        throw new badTaskCast;
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
        throw new badTaskCast;
}

#endif // ONLINEJOBTYPED_H
