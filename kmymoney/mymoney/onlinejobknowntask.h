#ifndef ONLINEJOBKNOWNTASK_H
#define ONLINEJOBKNOWNTASK_H

#include "onlinejob.h"

/**
 * @brief Convient template if you know the task type of an onlineJob
 *
 * To prevent using onlineJob.task<T>() repeatingly you can use this class
 * where task() has a defined return type.
 *
 * It is not possible to create invalid onlineJobs. All constructors will throw
 * onlineJobKnownTask::badCast if they fail.
 */
template<class T>
class KMM_MYMONEY_EXPORT onlineJobKnownTask : public onlineJob
{
    KMM_MYMONEY_UNIT_TESTABLE
public:
    explicit onlineJobKnownTask();

    onlineJobKnownTask( T* task, const QString& id = MyMoneyObject::m_emptyId );

    /** @brief Copy constructor */
    onlineJobKnownTask( onlineJobKnownTask<T> const& other );

    /** @brief Copy from onlineJob */
    onlineJobKnownTask(const onlineJob &other);

    /** @brief Copy constructor with new id */
    onlineJobKnownTask( const QString &id, const onlineJobKnownTask<T>& other );

    inline T* task();
    inline const T* task() const;
    inline const T* constTask() const { return task(); }

    onlineJobKnownTask<T> operator =( onlineJobKnownTask<T> const& other );

private:
    T* m_taskSubType;

    /**
     * @brief Test if m_taskSubType != 0 and throws an exception if needed
     * @throws MyMoneyException if m_taskSubType == 0
     */
    void testForValidTask() const;
};

template<class T>
onlineJobKnownTask<T>::onlineJobKnownTask( )
    : onlineJob( new T() ),
      m_taskSubType( 0 )
{
    m_taskSubType = static_cast<T>( onlineJob::task() );
}

template<class T>
onlineJobKnownTask<T>::onlineJobKnownTask( T* task, const QString& id )
    : onlineJob(task, id),
      m_taskSubType(task)
{
    testForValidTask();
}

template<class T>
onlineJobKnownTask<T>::onlineJobKnownTask( onlineJobKnownTask<T> const& other )
    : onlineJob(other),
      m_taskSubType( 0 )
{
    m_taskSubType = dynamic_cast<T*>(onlineJob::task());
    testForValidTask();
}

template<class T>
onlineJobKnownTask<T> onlineJobKnownTask<T>::operator =( onlineJobKnownTask<T> const& other )
{
    onlineJob::operator =(other);
    m_taskSubType = dynamic_cast<T*>(onlineJob::task());
    Q_ASSERT( m_taskSubType != 0 );
    return (*this);
}

template<class T>
onlineJobKnownTask<T>::onlineJobKnownTask(const onlineJob &other)
    : onlineJob( other )
{
    m_taskSubType = dynamic_cast<T*>(onlineJob::task());
    if (m_taskSubType == 0)
        throw new badTaskCast;
}

template<class T>
T* onlineJobKnownTask<T>::task()
{
    Q_ASSERT(m_taskSubType != 0);
    return m_taskSubType;
}

template<class T>
const T* onlineJobKnownTask<T>::task() const
{
    Q_ASSERT(m_taskSubType != 0);
    return m_taskSubType;
}

template<class T>
void onlineJobKnownTask<T>::testForValidTask() const
{
    if (m_taskSubType == 0)
        throw new badTaskCast;
}

#endif // ONLINEJOBKNOWNTASK_H
