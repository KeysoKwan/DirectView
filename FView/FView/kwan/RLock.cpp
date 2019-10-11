#include "RLock.h"
namespace dxshow {
Rlock::Rlock(std::mutex* mu) : m_Mutex(mu), m_lockSuccessed(false)
{
    m_lockSuccessed = m_Mutex->try_lock();
}

Rlock::~Rlock()
{
    if(m_lockSuccessed)
        m_Mutex->unlock();
}
bool Rlock::LockSuccessed()
{
    return m_lockSuccessed;
}
} // namespace dxshow
