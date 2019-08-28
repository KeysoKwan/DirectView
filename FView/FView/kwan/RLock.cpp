#include "RLock.h"
namespace dxshow {
Rlock::Rlock(std::mutex* mu) : m_Mutex(mu)
{
    m_Mutex->lock();
}

Rlock::~Rlock()
{
    m_Mutex->unlock();
}
} // namespace dxshow
