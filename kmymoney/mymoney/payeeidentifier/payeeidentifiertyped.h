/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAYEEIDENTIFIERTYPED_H
#define PAYEEIDENTIFIERTYPED_H

#include "payeeidentifier.h"

template<class T>
class payeeIdentifierTyped : public payeeIdentifier
{
public:
  payeeIdentifierTyped(const payeeIdentifierTyped& other);
  payeeIdentifierTyped(T* pid);
  //explicit payeeIdentifierTyped();
  explicit payeeIdentifierTyped(const payeeIdentifier& other);

  T* operator->();
  const T* operator->() const;

  T* data() {
    return operator->();
  }
  const T* data() const {
    return operator->();
  }

  payeeIdentifierTyped& operator=(const payeeIdentifierTyped& other);
  bool operator==(const payeeIdentifierTyped& other);

private:
  /** this method is not save in this class, so deactivate it */
  void setData(payeeIdentifierData* dataPtr);
  T* m_payeeIdentifierTyped;
};

#if 0
template< class T >
payeeIdentifierTyped<T>::payeeIdentifierTyped()
    : payeeIdentifier()
{
  Q_ASSERT(false && "This method is not implemented yet");
  throw payeeIdentifier::empty();
}
#endif

template< class T >
payeeIdentifierTyped<T>::payeeIdentifierTyped(T* pid)
    : payeeIdentifier(pid),
    m_payeeIdentifierTyped(pid)
{
  if (m_payeeIdentifierTyped == 0)
    throw PAYEEIDENTIFIEREMPTYEXCEPTION;
}

template< class T >
payeeIdentifierTyped<T>::payeeIdentifierTyped(const payeeIdentifierTyped& other)
    : payeeIdentifier(other)
{
  m_payeeIdentifierTyped = dynamic_cast<T*>(payeeIdentifier::data());
  Q_CHECK_PTR(m_payeeIdentifierTyped);
}

template< class T >
payeeIdentifierTyped<T>& payeeIdentifierTyped<T>::operator=(const payeeIdentifierTyped<T>& other)
{
  payeeIdentifierTyped<T>& ret = static_cast<payeeIdentifierTyped<T>&>(payeeIdentifier::operator=(other));
  // This operation is save even if this == &other
  ret.m_payeeIdentifierTyped = dynamic_cast<T*>(ret.payeeIdentifier::data());
  return ret;
}

template< class T >
bool payeeIdentifierTyped<T>::operator==(const payeeIdentifierTyped& other)
{
  return payeeIdentifier::operator==(other);
}

template< class T >
payeeIdentifierTyped<T>::payeeIdentifierTyped(const payeeIdentifier& other)
    : payeeIdentifier(other)
{
  m_payeeIdentifierTyped = dynamic_cast<T*>(payeeIdentifier::data());
  if (m_payeeIdentifierTyped == 0) {
    if (payeeIdentifier::data() == 0)
      throw PAYEEIDENTIFIEREMPTYEXCEPTION;
    throw PAYEEIDENTIFIERBADCASTEXCEPTION;
  }
}

template< class T >
T* payeeIdentifierTyped<T>::operator->()
{
  return m_payeeIdentifierTyped;
}

template< class T >
const T* payeeIdentifierTyped<T>::operator->() const
{
  return m_payeeIdentifierTyped;
}


#endif // PAYEEIDENTIFIERTYPED_H
