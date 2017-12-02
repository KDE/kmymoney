/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
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
#include "onlinejobadministration.h"

/**
 * @brief Convenient template if you know the task type of an onlineJob
 *
 * To prevent using onlineJob.task<T>() repeatingly you can use this class
 * where task() has a defined return type.
 *
 * Any type check is done in the constructors. So an invalid onlineJobTyped
 * cannot exist. This class is very fast as well because task() does not need
 * any checks.
 *
 * onlineJobTyped::isNull() is always false. All constructors will throw
 * onlineJobTyped::badCast or onlineJobTyped::emptyTask if they fail.
 */
template<class T>
class onlineJobTyped : public onlineJob
{
  KMM_MYMONEY_UNIT_TESTABLE
public:
  /**
   * @brief create new task
   *
   * @throws emptyTask if plugin could not be found/loaded (determined at runtime).
   */
  explicit onlineJobTyped();

  /**
    * @brief Create typed onlineJob
    *
    * @throws emptyTask if task == 0
    */
  explicit onlineJobTyped(T* task, const QString& id = QString());

  /** @brief Copy constructor */
  onlineJobTyped(onlineJobTyped<T> const& other);

  /**
    * @brief Copy from onlineJob
    *
    * @throws badTaskCast if task in other does not fit T
    * @throws emptyTask if other has no task
    */
  explicit onlineJobTyped(const onlineJob &other);

  /** @brief Copy constructor with new id */
  explicit onlineJobTyped(const QString &id, const onlineJobTyped<T>& other);

  /** Does not throw */
  inline T* task(); // krazy:exclude=inline

  /** Does not throw */
  inline const T* task() const; // krazy:exclude=inline

  /** Does not throw */
  inline const T* constTask() const { // krazy:exclude=inline
    return task();
  }

  /** Does not throw */
  onlineJobTyped<T> operator =(onlineJobTyped<T> const& other);

private:
  T* m_taskTyped;
};

template<class T>
onlineJobTyped<T>::onlineJobTyped()
    : onlineJob(onlineJobAdministration::instance()->createOnlineTask(T::name()))
{
  m_taskTyped = static_cast<T*>(onlineJob::task());   // this can throw emptyTask

  // Just be safe: an onlineTask developer could have done something wrong
  Q_CHECK_PTR(dynamic_cast<T*>(onlineJob::task()));
}

template<class T>
onlineJobTyped<T>::onlineJobTyped(T* task, const QString& id)
    : onlineJob(task, id),
    m_taskTyped(task)
{
  if (task == 0)
    throw emptyTask(__FILE__, __LINE__);
}

template<class T>
onlineJobTyped<T>::onlineJobTyped(onlineJobTyped<T> const& other)
    : onlineJob(other)
{
  m_taskTyped = dynamic_cast<T*>(onlineJob::task());
  Q_CHECK_PTR(m_taskTyped);
}

template<class T>
onlineJobTyped<T> onlineJobTyped<T>::operator =(onlineJobTyped<T> const & other)
{
  onlineJob::operator =(other);
  m_taskTyped = dynamic_cast<T*>(onlineJob::task());
  Q_CHECK_PTR(m_taskTyped);
  return (*this);
}

template<class T>
onlineJobTyped<T>::onlineJobTyped(const onlineJob &other)
    : onlineJob(other)
{
  m_taskTyped = dynamic_cast<T*>(onlineJob::task()); // can throw emptyTask
  if (m_taskTyped == 0)
    throw badTaskCast(__FILE__, __LINE__);
}

template<class T>
T* onlineJobTyped<T>::task()
{
  Q_CHECK_PTR(m_taskTyped);
  return m_taskTyped;
}

template<class T>
const T* onlineJobTyped<T>::task() const
{
  Q_CHECK_PTR(m_taskTyped);
  return m_taskTyped;
}

#endif // ONLINEJOBTYPED_H
