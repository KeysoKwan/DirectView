#include "RLock.h"

Rlock::Rlock(mutex* mu) : m_Mutex(mu)
{
    m_Mutex->lock();
}

Rlock::~Rlock()
{
    m_Mutex->unlock();
}
