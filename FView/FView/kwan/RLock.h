//简单的自动互斥类
#include <mutex>
namespace dxshow {
class Rlock
{
  public:
    Rlock(std::mutex* mu);   
    ~Rlock();
    bool LockSuccessed();
  private:
    Rlock(){};
    Rlock(const Rlock& cpy) {};
    std::mutex* m_Mutex;
    bool m_lockSuccessed;
};
} // namespace dxshow
